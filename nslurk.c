/*
 * attach to another pids namespace and launch a shell (or something else)
 */
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <fcntl.h>
#include <stdio.h>
#include <err.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


static const char *
nsname(int type)
{
   struct typetuple {
      int type;
      const char *name;
   };
   static struct typetuple types[] = {
      { CLONE_NEWNS, "mnt" },
      { CLONE_NEWNET, "net" },
      { CLONE_NEWPID, "pid" },
      { 0, 0 }
   };

   for (struct typetuple *t = types; t->name; t++) {
      if (t->type == type)
         return t->name;
   }
   abort();
}

static void
doNamespace(const char *pidstr, int type)
{
   char path[64]; // plenty for "/proc/XXXXX/ns/xxx"

   int rc = snprintf(path, sizeof path, "/proc/%s/ns/%s", pidstr, nsname(type));
   if (rc >= sizeof path)
      errx(1, "invalid pid/namespace");

   int fd = open(path, O_RDONLY);
   if (fd == -1) {
      err(1, "can't open file %s", path);
   }
   if (setns(fd, type) == -1) {
      err(1, "can't set ns");
   }
}

int
main(int argc, char *argv[])
{

   bool verbose = false;
   int c;
   while ((c = getopt(argc, argv, "n:m:p:v")) != -1) {
      switch (c) {
         case 'm':
            doNamespace(optarg, CLONE_NEWNS);
            break;
         case 'n':
            doNamespace(optarg, CLONE_NEWNET);
            break;
         case 'p':
            doNamespace(optarg, CLONE_NEWPID);
            break;
         case 'v':
            verbose = true;
      }
   }

   const char *cmd;

   if (argc != optind) {
      size_t cmdlen = 0;
      int i;

      for (i = optind; i < argc; ++i)
         cmdlen += strlen(argv[i]) + 1;
      char *cmdline = malloc(cmdlen - 1), *p = cmdline;

      for (p = cmdline, i = optind; i < argc; ++i)
         p += snprintf(p, cmdline + cmdlen - p, "%s%s", argv[i], i == argc - 1 ? "" : " ");
      assert(p - cmdline == cmdlen - 1);
      cmd = cmdline;
   } else {
      cmd = getenv("SHELL");
   }

   if (verbose) {
      fprintf(stderr, "running '%s'\n", cmd);
   }
   if (system(cmd) != 0) {
      err(1, "'%s' failed", cmd);
   }
}
