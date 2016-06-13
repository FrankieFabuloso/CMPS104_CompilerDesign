#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>

using namespace std;
struct astree;
#include "typechecker.h"
#include "auxlib.h"

struct astree {
   int symbol;                   // token code
   size_t filenum;                // index into filename
   size_t linenum;                // line number from source code
   size_t offset;                // offset of token in current line
   const string* clexinfo;        // pointer to lex info
   attr_bitset attributes;       // attributes for the node
   size_t block_num;              // block number
   symbol_entry* struct_entry;   // struct table entry
   vector<astree*> children;     // children of given n-way node
   bool checked = false;         // visitation
};

astree* new_astree (int symbol, int filenum, int linenum, int offset,
                    const char* clexinfo);
astree* change_sym (astree* root, int symbol);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt3 (astree* root, astree* left, astree* mid, astree* right);
astree* adopt2sym(astree* root, astree*left, astree* right, int symbol);
astree* adopt1sym (astree* root, astree* child, int symbol);
void dump_astree (FILE* outfile, astree* root);
void dump_tok (FILE* outfile, astree* node);
void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);
void free_ast3 (astree* tree1, astree* tree2, astree* tree3);

void assign_attrs(astree* ast);

#endif
