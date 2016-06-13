#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "auxlib.h"
#include "stringset.h"

string CPP = "/usr/bin/cpp";
const size_t MAXLINESIZE = 1024;
bool lflag = false;
bool yflag = false;

// Scan the options, -D -y -l -@ and check for operands.
void check_flags (int argc, char** argv) {
   for (;;) {
      int flag = getopt (argc, argv, "D:yl@:");
      if (flag == EOF) break;
      switch (flag) {
         case 'l':
            lflag = true;
            DEBUGF('o', "Flag l set");
            break;
         case 'y': 
            yflag= true;
            DEBUGF('o', "Flag y set");
            break;
         case '@':
            set_debugflags(optarg);
            break;
         case 'D':
            CPP += " -D";
            CPP += optarg;
            CPP += " ";
            DEBUGF('o', "Flag D set with option: %c", optarg);
            break;
         default:
            errprintf("%c: %s", (char) optopt, "option not recognized.\n");
            break;
      }
   }
}

// rem_new_line if is last character in line
void rem_new_line (char* string, char new_line) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == new_line) *nlpos = '\0';
}

// pre process the lines in the file
void preproclines (FILE* pipe, char* filename) {
   int linenr = 1;
   char inputname[MAXLINESIZE];
   
   strcpy (inputname, filename);
   for (;;) {
      char buffer[MAXLINESIZE];
      char* tmp_fgets = fgets (buffer, MAXLINESIZE, pipe);
      if (tmp_fgets == NULL) break;
      rem_new_line (buffer, '\n');
      int sscanftok = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, filename);
      if (sscanftok == 2) {
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         intern_stringset(token);
      }
      ++linenr;
   }
}

int main (int argc, char** argv) {
   set_execname (argv[0]);
   check_flags(argc, argv);

   DEBUGF('c', "argc: %d optind: %d\n", argc, optind);
   if (optind >= argc) {

      errprintf("Usage: %s", "Usage: ", "[-ly] [-@ flag ...]",
         " [-D string] filename.oc\n", get_execname());
   } else {

      char *filename = argv[optind];
      char *ext = strrchr(filename, '.');
      if (ext == NULL || strcmp(ext, ".oc") != 0) {
         errprintf("Error: not a .oc file.\n");
         return (get_exitstatus());
      }

      string command = CPP + " " + filename;
      FILE* pipe = popen (command.c_str(), "r");
      if (pipe == NULL) {
         syserrprintf (command.c_str());
      } else {
         preproclines (pipe, filename);
         int pclose_rc = pclose (pipe);
         eprint_status (command.c_str(), pclose_rc);
         if (pclose_rc != 0) {
            set_exitstatus(EXIT_FAILURE);
         }
      }
      // Remove file suffix and replace with .str for output file
      string outfile = basename(filename);
      size_t i = outfile.find_last_of('.');
      outfile.erase(i+1, 2);
      outfile.append("str");
      FILE *out = fopen(outfile.c_str(), "w");
      dump_stringset(out);
      fclose(out);
   }
   return get_exitstatus();
}
