// Copyright (c) 2016 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <list>

typedef std::list<std::string> stringlist;

int checkFile(const char *name, stringlist &list)
{
   struct stat sb;
   if (stat(name, &sb) == 0) {
      if (sb.st_blocks / 8 != (sb.st_size + 4095) / 4096)
         return -1;
   }
   return 0;
}

int scanDir(int parent, const char *name, stringlist &list)
{
   int fd = open(name, O_RDONLY);
   if (fd == -1)
      return 0;

   if (fchdir(fd) != -1) {
      DIR *d = fdopendir(fd);
      if (d != 0) {
         for (;;) {
            dirent ent, *res;
            int rc = readdir_r(d, &ent, &res);
            if (rc == -1 || res == 0) {
               break;
            }
            switch (ent.d_type) {
             case DT_DIR:
               if (ent.d_name[0] != '.')
                  if (scanDir(fd, ent.d_name, list) == -1) {
                     list.push_front(ent.d_name);
                     return -1;
                  }
               break;
             case DT_REG:
               if (checkFile(ent.d_name, list) == -1) {
                  list.push_front(ent.d_name);
                  return -1;
               }
               break;
             default:
               break;
            }
         }
         closedir(d);
      }
   }
   if (parent != -1) {
      int rc = fchdir(parent);
      if (rc == -1)
         abort();
   }
   return 0;
}

void
findFile(const char *name)
{
   stringlist list;
   if (scanDir(-1, name, list) == -1) {
      std::cout << name;
      for (auto i : list)
         std::cout << "/" << i;
      std::cout << "\n";
   }
}

int
testMap(const char *name)
{
   int fd = open(name, O_RDONLY);

   if (fd == -1)
      abort();

   struct stat sb;
   int rc = fstat(fd, &sb);
   if (rc == -1)
      abort();

   size_t size = sb.st_blocks * 512;
   void *p = mmap(0, sb.st_blocks * 512, PROT_READ, MAP_PRIVATE, fd, 0);
   if (p == MAP_FAILED)
      abort();
   return ((char *)p)[size - 1];

}

struct Usage{};
std::ostream &operator << (std::ostream &os, const Usage &) {
   return os << "usage: maptest <-m file-to-map | -f directory-to-search>\n";
}

int
main(int argc, char *argv[])
{

   int c;
   while ((c = getopt(argc, argv, "f:m:")) != -1) {
      switch (c) {
         case 'f':
            findFile(optarg);
            break;
         case 'm':
            testMap(optarg);
            break;
         default:
            std::clog << Usage();
            return -1;
      }
   }
   if (optind != argc || argc == 1)
      std::clog << Usage();
}
