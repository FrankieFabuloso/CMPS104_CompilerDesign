#ifndef __TYPECHECKER_H__
#define __TYPECHECKER_H__

#include <string>
#include <iostream>
#include <unordered_map>
#include <bitset>
#include <vector>

using namespace std;

// enum defining attribute values
enum {  ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
        ATTR_string, ATTR_struct, ATTR_array, ATTR_function, ATTR_proto,
        ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval,
        ATTR_const, ATTR_vreg, ATTR_vaddr, ATTR_bitset_size
};

// attributes, symbol tables, and symbol table entries
struct symbol;
using attr_bitset = bitset<ATTR_bitset_size>;
using symbol_table = unordered_map<string*, symbol*>;
using symbol_entry = pair<string*,symbol*>;
using symbol_stack = vector<symbol_table*>;

#include "astree.h"
#include "auxlib.h"

struct symbol {
   attr_bitset attributes;
   symbol_table* fields;
   string *type_id;
   size_t filenum, linenum, offset, block_num;
   vector<symbol*>* parameters;
};

// Initialize the global stack
void typecheck_init ();

// Perform a DFS traversal of the astree pointed at by root
void fn_names_traversal (astree* root);

// Set the attributes
void set_attributes_recur (astree *root);

// Return a string which is the concatenation of all of the attributes.
// This used for the printing necessary for the .ast file
const char *get_attributes (attr_bitset attributes);

void table_dump ();

void free_typechecker();

#endif

