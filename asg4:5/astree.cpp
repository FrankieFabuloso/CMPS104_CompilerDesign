#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
#include "typechecker.h"

astree* new_astree (int symbol, int filenum, int linenum, int offset,
                    const char* clexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenum = filenum;
   tree->linenum = linenum;
   tree->offset = offset;
   tree->clexinfo = intern_stringset (clexinfo);
   tree->attributes = 0;
   tree->block_num = 0;
   tree->struct_entry = NULL;
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenum, tree->linenum, tree->offset,
           get_yytname (tree->symbol), tree->clexinfo->c_str());
   return tree;
}

astree* change_sym(astree *root, int symbol) {
   root->symbol = symbol;
   return root;
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->clexinfo->c_str(),
           child, child->clexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt3 (astree* root, astree* left,
       astree* middle, astree* right) {
   adopt1 (root, left);
   adopt1 (root, middle);
   adopt1 (root, right);
   return root;
}

astree* adopt2sym (astree* root, astree* left,
        astree* right, int symbol) {
   root = adopt2(root, left, right);
   root->symbol = symbol;
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

// used for printing to the .ast file
// not (normally) called externally -- only by dump_astree_rec()
static void dump_node (FILE* outfile, astree* node) {
   char* tname = (char*) get_yytname(node->symbol);
   if (strstr(tname, "TOK_") == tname) tname += 4;
   fprintf(outfile, "%s \"%s\" (%zu.%zu.%zu) {%lu} %s ",
      tname,
      (node->clexinfo)->c_str(), node->filenum,
      node->linenum, node->offset, node->block_num,
      get_attributes(node->attributes));
   if (node->symbol == TOK_STRUCT) {
      fprintf(outfile, "\"%s\"", node->struct_entry->first->c_str());
   }
   if (node->symbol == TOK_IDENT) {
      fprintf(outfile, "(%u.%u.%u)", 0, 0, 0);
   }
   fprintf(outfile, "\n");
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
      if (root == NULL) return;
      for (int i = 0; i < depth; i++) fprintf(outfile, "|\t");
      dump_node (outfile, root);
      for (size_t child = 0; child < root->children.size(); ++child) {
         dump_astree_rec (outfile, root->children[child], depth + 1);
      }
}

// Used for printing to the .tok file (called externally)
void dump_tok (FILE* outfile, astree* node) {
   fprintf(outfile, "%3ld %ld.%03ld %-3d %-15s (%s)\n",
            node->filenum, node->linenum, node->offset,
            node->symbol, get_yytname(node->symbol),
            node->clexinfo->c_str());
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep) {
   DEBUGF ('f', "toknum = %d, yyvaluep = %p\n", toknum, yyvaluep);
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n", get_yytname (toknum), toknum);
   }
   fflush (NULL);
}

void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: \"%s\")\n",
           (uintptr_t) root, root->filenum, root->linenum, root->offset,
           get_yytname (root->symbol), root->clexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

void free_ast3 (astree* tree1, astree* tree2, astree* tree3) {
   free_ast(tree1);
   free_ast(tree2);
   free_ast(tree3);
}

