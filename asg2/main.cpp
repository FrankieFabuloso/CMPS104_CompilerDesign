#include <string>
using namespace std;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <vector>
#include "lyutils.h"
#include "stringset.h"
#include "auxlib.h"
#include "astree.h"

#define BUFSIZE 4096


char* add_extension(char* ocfile, string new_ext){
  char output_file[BUFSIZE];
  strcpy(output_file, basename(ocfile));
  auto dot_index = strrchr(output_file, '.');
  *dot_index = '\0';
  strcat(output_file, new_ext.c_str());
  return strdup(output_file);
}

void fill_string_table(){
  //Reads yyin, fills stringtable
  unsigned token_type;
  while((token_type = yylex())){
    if (token_type == YYEOF)
      break;
  }
}

FILE* preprocess(string ocfile, string options){
  string cmd = "cpp ";
  cmd += ocfile + " ";
  if (!options.empty())
    cmd += "-D" + options;

  return(popen(cmd.c_str(), "r"));
}

int main (int argc, char **argv) {
  int c;
  bool yflag = false;
  bool lflag = false;
  string dflag;
  string atflag;
  set_execname(argv[0]);

  while((c = getopt(argc-1, argv, "lyD:@:")) != -1)
    switch(c){
      case 'l':
        lflag = true;
        break;
      case 'y':
        yflag = true;
        break;
      case 'D':
        dflag = optarg;
        break;
      case '@':
        set_debugflags(optarg);
        break;
      default:
        fprintf(stderr, "No such option: `%s'\n", optarg);
    }
  yy_flex_debug = lflag;
  yydebug = yflag;
  char* ocfile = argv[argc-1];
  if (!strstr(ocfile, ".oc")){
    fprintf(stderr, "File %s is not a .oc file\n", ocfile);
    set_exitstatus(1);
  }
  FILE* tmp = fopen(ocfile, "r");
  if (tmp == NULL){
    fprintf(stderr, "File not found\n");
    exit(1);
  }
  fclose(tmp);

  //File parsing
  char* str_fname = add_extension(ocfile, ".str");
  char* tok_fname = add_extension(ocfile, ".tok");

  //Dump stringset to file
  FILE* strfile = fopen(str_fname, "w");

  //Data is dumped to tokfile via scanner and lyutils
  tokfile = fopen(tok_fname, "w");

  yyin = preprocess(ocfile, dflag);   
  fill_string_table();
  fclose(yyin);
  dump_stringset(strfile);
  fclose(strfile);
  fclose(tokfile);
  return get_exitstatus();
}
