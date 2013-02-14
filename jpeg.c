#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <jpeglib.h>

#define BUFFERSIZE (65536 * 100)

struct imgsrc {
    struct jpeg_source_mgr pub;
    JOCTET buf[BUFFERSIZE];
    const char *fn;
    int fd;
};

boolean
fill_input_buffer(struct jpeg_decompress_struct *dec)
{
    struct imgsrc *src = (struct imgsrc *)dec->src;
    src->pub.next_input_byte = src->buf;
    src->pub.bytes_in_buffer = read(src->fd, src->buf, sizeof src->buf);
    return 1;
}

void
skip_input_data(struct jpeg_decompress_struct *dec, long skip)
{
    struct imgsrc *src = (struct imgsrc *)dec->src;
    int skipBlocks, take;

    take = MIN(src->pub.bytes_in_buffer, skip);

    skip -= take;
    src->pub.bytes_in_buffer -= take;
    src->pub.next_input_byte += take;

    skipBlocks = skip / BUFFERSIZE;
    if (skipBlocks) {
        lseek(src->fd, SEEK_CUR, BUFFERSIZE * skipBlocks);
        skip %= BUFFERSIZE;
    }

    if (skip) {
        fill_input_buffer(dec);
        src->pub.next_input_byte += skip;
        src->pub.bytes_in_buffer -= skip;
    }
}

void
init_source(struct jpeg_decompress_struct *dec)
{
    struct imgsrc *src = (struct imgsrc *)dec->src;
    src->fd = open(src->fn, O_RDONLY);
}

void
term_source(struct jpeg_decompress_struct *dec)
{
    struct imgsrc *src = (struct imgsrc *)dec->src;
    close(src->fd);
}

void
loadsrc(struct imgsrc *src, const char *fn)
{
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = term_source;
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = src->buf;
    src->fn = strdup(fn);
}

int
main(int argc, char *argv[])
{
    char bright[] = " .:o+OQ";
    struct jpeg_decompress_struct decomp;
    struct jpeg_error_mgr error;
    struct imgsrc src;

    int stride, x, y, samplex, sampley;
    int lines = 24, columns = 80;
    const char *p;
    JSAMPARRAY buffer;

    p = getenv("LINES");
    if (p)
        lines = atoi(p);
    p = getenv("COLUMNS");
    if (p)
        columns = atoi(p);

    decomp.err = jpeg_std_error(&error);
    jpeg_create_decompress(&decomp);

    loadsrc(&src, argv[1]);
    decomp.src = &src.pub;
    jpeg_read_header(&decomp, 1);
    jpeg_start_decompress(&decomp);

    samplex = decomp.output_width / columns; // number of JPEG pixels across for a char.
    sampley = decomp.output_height / lines; // number of JPEG pixels down for a char.

    // We stride one row of the terminal at a time
    stride = decomp.output_width * decomp.output_components;
    buffer = decomp.mem->alloc_sarray((j_common_ptr)&decomp, JPOOL_IMAGE, stride, 1);

    int *intensity = calloc(columns, sizeof (int));
    int *samples = calloc(columns, sizeof (int));

    int linesDone = 0;

    while (decomp.output_scanline < decomp.output_height) {
        int havelines = jpeg_read_scanlines(&decomp, buffer, 1);
        if (havelines == 0)
            break;

        int to = 0;
        for (int col = 0; col < decomp.output_width; ++col) {
            for (int component = 0; component < decomp.output_components; ++component) {
                // XXX: Balance RGB
                intensity[to] += buffer[0][col * decomp.output_components + component];
                samples[to]++;
            }
            if ((col + 1) % samplex == 0) {
                if (++to >= columns)
                    break;
            }
        }

        if (++linesDone % sampley == 0) {
            for (int i = 0; i < columns; ++i) {
                intensity[i] *= sizeof bright - 1;
                intensity[i] /= 256 * samples[i];
                putc(bright[intensity[i]], stdout);
                intensity[i] = 0;
                samples[i] = 0;
            }
            putc('\n', stdout);
        }
    }

    jpeg_finish_decompress(&decomp);
    jpeg_destroy_decompress(&decomp);
    return 0;
}
