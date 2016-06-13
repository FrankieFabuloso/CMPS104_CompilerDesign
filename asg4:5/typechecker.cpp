#include <string>
#include <unordered_map>
#include <vector>

#include "typechecker.h"
#include "astree.h"
#include "lyutils.h"

/* 
// Data Structures and symbol tables:
// We need symbol tables for:
// - function and variable names
// - type names
// - fields
*/
symbol_stack idents; // function and variable names
symbol_table types;  // type idents -- structs + primitives
symbol_stack fields; // possible struct fields
size_t block_count = 0;
size_t next_block = 1;
vector<size_t> blockstack;

// Symbol table output file (from main.cpp)
extern FILE *sym_file;
extern FILE *oil_file;

// Initialize typechecking
void typecheck_init() {
   symbol_table *global = new symbol_table();
   blockstack.push_back(block_count);
   idents.push_back(global);
}

// Enter a block
// Returns a pointer to a symbol table used for the new scope
symbol_table *enter_block() {
   block_count = next_block;
   next_block++;
   blockstack.push_back(block_count);
   symbol_table *new_block_table = new symbol_table();
   return new_block_table;
}

// Leave a block downwards
void exit_block() {
   blockstack.pop_back();
}

void symbol_dump (symbol *sym, string *name) {
   if (sym == NULL || name == NULL) return;   
   size_t depth = sym->block_num;
   for (size_t i = 0; i < depth; i++) fprintf(sym_file, "\t");
   fprintf(sym_file, "%s (%zu.%zu.%zu) {%zu} %s\n",
           name->c_str(),
           sym->filenum,
           sym->linenum,
           sym->offset, 
           sym->block_num,
           get_attributes(sym->attributes));
}

// Create a new symbol
// We use the filenum, linenum, offset from the ast node
// blocknr comes from the blockptr
symbol *new_symbol (astree *symbol_node) {
   if (symbol_node == NULL) return NULL;
   symbol *newsym = new symbol();
   newsym->fields = NULL;
   newsym->filenum = symbol_node->filenum;
   newsym->linenum = symbol_node->linenum;
   newsym->offset = symbol_node->offset;
   newsym->block_num = blockstack.back();
   newsym->attributes = symbol_node->attributes;
   newsym->parameters = NULL;
   newsym->type_id = NULL;
   return newsym;
}

// Insert a sym + name into a symbol table
bool sym_insertion (symbol_table *table, symbol *sym, string *name) {
   if (table == NULL || sym == NULL || name == NULL) return false;
   if (table->find(name) != table->end()) return false;
   table->insert(make_pair(name, sym));
   symbol_dump(sym, name);
   return true;
}

// Create a new struct for sym table
void new_type_table (astree *struct_node) {
   if (struct_node == NULL || struct_node->children.empty()) return;
   astree *struct_name = struct_node->children[0];
   if (struct_name == NULL) return;
   symbol *symbol_type = new_symbol(struct_name);
   sym_insertion(&types, symbol_type, (string*)struct_name->clexinfo);
   fprintf(oil_file, "struct s_%s {\n",struct_name->clexinfo->c_str()); // ic
   
   symbol_table *field_table = new symbol_table();
   // skip the first child in the loop 
   bool skip = true;
   for (auto &i : struct_node->children) {
      if (i->symbol == TOK_TYPEID && skip) {
         skip = false;
         continue;
      }
      if (i->children.empty()) return;
      astree *field_name = i->children[0];
      if (field_name == NULL) return;
      symbol *field = new_symbol(i);
      if (field == NULL) return;
      field->attributes.set(ATTR_field);
      sym_insertion(field_table, field, (string*)field_name->clexinfo);
      fprintf(oil_file, "\t%s f_%s_%s;\n", 
                  i->clexinfo->c_str(),
                  struct_name->clexinfo->c_str(),
                  field_name->clexinfo->c_str());
   }
   fprintf(oil_file, "};\n");
   fields.push_back(field_table);
   symbol_type->fields = field_table;
   struct_node->checked = true;
   struct_node->struct_entry = new symbol_entry(
            (string*)struct_name->clexinfo, symbol_type);
}

// Create a new variable sym table
void new_var_table (astree *var_node) {
   if (var_node == NULL || var_node->children.empty()) return;
   astree *var_type = var_node->children[0];
   if (var_type == NULL) return;
   astree *var_name = var_type->children[0];
   if (var_name == NULL) return;
   astree *var_val = var_node->children[1];
   if (var_val == NULL) return;
   symbol *var_sym = new_symbol(var_node);
   if (var_sym == NULL) return;
   var_sym->attributes.set(ATTR_variable);
   sym_insertion(idents[blockstack.back()],
                 var_sym, (string*)var_name->clexinfo);
   for (uint8_t g = 0; g <= blockstack.back(); g++ ) {
      fprintf(oil_file, "\t");
   }
   fprintf(oil_file, "%s __%s;\n", var_type->clexinfo->c_str(),
                                  var_name->clexinfo->c_str());
   var_node->checked = true;

}

void new_block_table (astree *block_node) {
   for (auto &i : block_node->children) {
      switch(i->symbol) {
      case (TOK_VARDECL):
         new_var_table(i);
         break;
      case (TOK_BLOCK): {
         symbol_table *iblock = enter_block();
         idents.push_back(iblock);
         new_block_table(i);
         exit_block();
         break;
      }
      default:
         break;
      }
   }
   block_node->checked = true;
}

// Create a new function parameter
void new_fn_parameter (astree *param, symbol_table *fn_table,
      symbol *fn_symbol) {
   if (param == NULL) return;
   if (param->children.empty()) return;
   astree *param_name = param->children[0];
   if (param_name == NULL) return;
   symbol *param_sym = new_symbol(param_name);
   if (param_sym == NULL) return;
   param_sym->attributes.set(ATTR_variable);
   param_sym->attributes.set(ATTR_param);
   fn_symbol->parameters->push_back(param_sym);
   sym_insertion(fn_table, param_sym, (string*)param_name->clexinfo);
   param->checked = true;
   fprintf(oil_file, "\t%s _%d_%s", param->clexinfo->c_str(),
                                    (int)blockstack.back(),
                                    param_name->clexinfo->c_str());

}

// Table for newly found function
void new_fn_table (astree *fn_node) {
   if (fn_node == NULL || fn_node->children.empty()) return;
   astree *type_node = fn_node->children[0];
   if (type_node == NULL || type_node->children.empty()) return;
   astree *name_node = type_node->children[0];
   if (name_node == NULL) return;
   astree *params_node = fn_node->children[1];
   if (params_node == NULL) return;
   astree *fnblock_node = fn_node->children[2];
   if (fnblock_node == NULL) return;

   symbol *fn_symbol = new_symbol(fn_node);
   if (fn_symbol == NULL) return;
   fn_symbol->attributes.set(ATTR_function);
   fn_symbol->parameters = new vector<symbol*>();

   symbol_dump(fn_symbol, (string *)name_node->clexinfo);

   // new block
   symbol_table *fn_table = enter_block();
   idents.push_back(fn_table);

   fprintf(oil_file, "%s __%s (\n", type_node->clexinfo->c_str(),
                                    name_node->clexinfo->c_str());
   // parameters
   for (auto &param_type : params_node->children) {
      new_fn_parameter(param_type, fn_table, fn_symbol);
      if (param_type == params_node->children.back()) {
         fprintf(oil_file, ")\n");
      } else {
         fprintf(oil_file, ",\n");
      }
   }
   fprintf(oil_file, "{\n");
   // block
   new_block_table(fnblock_node);
   exit_block();
   sym_insertion(idents[blockstack.back()],
                 fn_symbol, (string*)name_node->clexinfo);
   fn_node->checked = true;
   fprintf(oil_file, "{\n");
}

// print out the items stored in the two tables
void table_dump () {
   printf("Identifiers:\n");
   int ident_scp = 0;
   for (auto v: idents) {
      printf("%d\n", ident_scp);
      for (auto i = v->cbegin(); i != v->cend(); i++) {
         printf("\t %s %s\n", i->first->c_str(),
         get_attributes(i->second->attributes));
      }
      ident_scp++;
   }
   printf("Types:\n");
   for (auto i = types.cbegin(); i != types.cend(); i++) {
      printf("%s\n", i->first->c_str());
      for (auto j = i->second->fields->cbegin();
            j != i->second->fields->cend(); j++) {
         printf("\t%s %s\n", j->first->c_str(),
         get_attributes(j->second->attributes));
      }
   }
}

// Perform a DFS traversal of passed in astree.
void fn_names_traversal (astree *root) {
   if (root == NULL) return;
   if (root->children.empty()) {
      return;
   } else {
      for (auto &child : root->children) {
         if (child->checked) continue;
         switch(child->symbol) {
            case TOK_STRUCT: new_type_table(child); break;
            case TOK_FUNCTION: new_fn_table(child); break;
            case TOK_VARDECL: new_var_table(child); break;
            case TOK_BLOCK: {
               symbol_table *iblock = enter_block();
               idents.push_back(iblock);
               new_block_table(child);
               exit_block();
               break;
            }
         }
         fn_names_traversal(child);
      }
   }
}

// Main switch statement used for setting any given attribute 
// determined with scanned token.
void set_attributes(astree* n) {
   if (n == NULL) return;
   DEBUGF('z', "%s\n", get_yytname(n->symbol));
   switch (n->symbol) {
      case TOK_INTCON:                    
         n->attributes.set(ATTR_const);   
      case TOK_INT:
         n->attributes.set(ATTR_int);      break;
      case TOK_CHARCON:
         n->attributes.set(ATTR_const);
      case TOK_CHAR:
         n->attributes.set(ATTR_char);     break;
      case TOK_STRINGCON:
         n->attributes.set(ATTR_const);
      case TOK_STRING:
         n->attributes.set(ATTR_string);   break;
      case TOK_VOID:
         n->attributes.set(ATTR_void);     break;
      case TOK_BOOL:
         n->attributes.set(ATTR_bool);     break;
      case TOK_STRUCT:
         n->attributes.set(ATTR_struct);   break;
      case TOK_NULL:
         n->attributes.set(ATTR_const);    break;
      case TOK_FIELD:
         n->attributes.set(ATTR_field);    break;
      case TOK_FUNCTION:
         n->attributes.set(ATTR_function); break;
      case TOK_ARRAY:
         n->attributes.set(ATTR_array);    break;
      case TOK_VARDECL:
         n->attributes.set(ATTR_variable); break;
      case TOK_PROTOTYPE:
         n->attributes.set(ATTR_proto);    break;
      default: break;
   }
}

// Recursively set the attributes that will be printed starting at a tree root
void set_attributes_recur (astree *root) {
   if (root == NULL) return;
   if (root->children.empty()) {
      return;
   } else {
      for (auto &child : root->children) {
         set_attributes(child);
         set_attributes_recur(child);
      }
   }
}

// Return a string which is the concatenation of all of the attributes
// Helps with modified printing for the .ast file with given
// specifications of assignment.
const char *get_attributes (attr_bitset attributes) {
   string attrs_concat = "";
   if (attributes.test(ATTR_void))     attrs_concat += "void ";
   if (attributes.test(ATTR_bool))     attrs_concat += "bool ";
   if (attributes.test(ATTR_char))     attrs_concat += "char ";
   if (attributes.test(ATTR_int))      attrs_concat += "int ";
   if (attributes.test(ATTR_null))     attrs_concat += "null ";
   if (attributes.test(ATTR_string))   attrs_concat += "string ";
   if (attributes.test(ATTR_struct))   attrs_concat += "struct ";
   if (attributes.test(ATTR_array))    attrs_concat += "array ";
   if (attributes.test(ATTR_function)) attrs_concat += "function ";
   if (attributes.test(ATTR_proto))    attrs_concat += "prototype ";
   if (attributes.test(ATTR_variable)) attrs_concat += "variable ";
   if (attributes.test(ATTR_field))    attrs_concat += "field ";
   if (attributes.test(ATTR_typeid))   attrs_concat += "typeid ";
   if (attributes.test(ATTR_param))    attrs_concat += "param ";
   if (attributes.test(ATTR_lval))     attrs_concat += "lval ";
   if (attributes.test(ATTR_const))    attrs_concat += "const ";
   if (attributes.test(ATTR_vreg))     attrs_concat += "vreg ";
   if (attributes.test(ATTR_vaddr))    attrs_concat += "vaddr ";
   return attrs_concat.c_str();
}

void free_typechecker () {
   int size = idents.size();
   for (int i = 0; i < size; i++) {
      free (idents[i]);
   }
}
