#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <deque>
#include <map>

using namespace std;

struct Link {
    map<string, int> suffixes;
    unsigned total; // sum(words[].count)
};

typedef map<deque<string>, Link> Chain;

void generate(Chain c, deque<string> prefix, size_t maxlen)
{
    for (;;) {
        Chain::const_iterator chainIt = c.find(prefix);
        if (chainIt == c.end())
            break;
        const Link &l = chainIt->second;
        long r = random() % l.total;
        map<string, int>::const_iterator it;
        for (it = l.suffixes.begin(); r > 0; ++it) {
            r -= it->second;
            if (r <= 0)
                break;
        }
        cout << it->first << " ";
        if (*it->first.rbegin() == '.') {
            cout << "\n\n";
            break;
        }
        prefix.push_back(it->first);
        if (prefix.size() > maxlen)
            prefix.pop_front();
    }
}

static string
nextword(std::istream &in)
{

    std::string r;
    in >> r;
    return r;

    char c;
    std::ostringstream os;
    while  (in.good() && isspace(in.peek()))
        in.read(&c, 1);


    if (in.good() && isalnum(in.peek())) {
        while (in.good() && isalnum(in.peek())) {
            in.read(&c, 1);
            os << c;
        }
    } else {
        in.read(&c, 1);
        os << c;
    }
    if (!in.good())
        return "";
    return os.str();
}

int
main(int argc, char *argv[])
{
    deque<string> list;
    Chain c;
    vector<deque<string> >starts;
    size_t prefixLength = argc > 1 ? atoi(argv[1]) : 3;

    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srandom(time(0) + ts.tv_nsec);

    starts.push_back(list);
    for (;;) {
        string word = nextword(cin);
        if (word.length() == 0)
            break;

        Link &l = c[list];
        l.total++;
        l.suffixes[word]++;
        list.push_back(word);
        if (list.size() > prefixLength)
            list.pop_front();
        if (!isalnum(*word.rbegin()))
            starts.push_back(list);
    }

    for (int i = 0; i < 1000; ++i) 
        generate(c, starts[random() % starts.size()], prefixLength);
    return 0;
}
