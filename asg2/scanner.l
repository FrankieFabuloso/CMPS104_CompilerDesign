%{

#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { lexer_useraction (); }
#define IGNORE(THING)   { }


%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER      [A-Za-z_]
DIGIT       [0-9]
NUMBER      {DIGIT}+

PREPROC  (#{NUMBER} {STRING})
IDENT    ({LETTER}({LETTER}|{DIGIT})*)
BADIDENT ({NUMBER}+{IDENT})
CHAR 	 ('([^\\'\n]|\\[\\'"0nt])')
STRING 	 (\"([^\\"\n]|\\[\\'"0nt])*\")
BADSTRING 	(\"[^\n\"\\]*(.|\n))
ABADCHAR 	('(.*)|\n')
BADCHAR 	('[\n\'\\]'|'.[^\'])

%%

"#".*           { lexer_include(); }
[ \t]+          { IGNORE (white space) }
\n              { lexer_newline(); }

"void"         { return yylval_token (TOK_VOID);   }
"bool"         { return yylval_token (TOK_BOOL);   }
"char"         { return yylval_token (TOK_CHAR);   }
"int"          { return yylval_token (TOK_INT);    }
"string"       { return yylval_token (TOK_STRING); }
"if"           { return yylval_token (TOK_IF);     }
"else"         { return yylval_token (TOK_ELSE);   }
"while"        { return yylval_token (TOK_WHILE);  }
"return"       { return yylval_token (TOK_RETURN); }
"struct"       { return yylval_token (TOK_STRUCT); }
"false"        { return yylval_token (TOK_FALSE);  }
"true"         { return yylval_token (TOK_TRUE);   }
"null"         { return yylval_token (TOK_NULL);   }
"ord"          { return yylval_token (TOK_ORD);    }
"chr"          { return yylval_token (TOK_CHR);    }
"new"          { return yylval_token (TOK_NEW);    }
"=="           { return yylval_token (TOK_EQ);     }
"!="           { return yylval_token (TOK_NE);     }
"<"            { return yylval_token (TOK_LT);     }
"<="           { return yylval_token (TOK_LE);     }
">"            { return yylval_token (TOK_GT);     }
">="           { return yylval_token (TOK_GE);     }
"[]"           { return yylval_token (TOK_ARRAY);  }

{NUMBER}       { return yylval_token (TOK_INTCON);       }
{IDENT}        { return yylval_token (TOK_IDENT);        }
{BADIDENT}     { lexer_badtoken(yytext);               }
{CHAR}         { return yylval_token (TOK_CHARCON);      }
{STRING}       { return yylval_token (TOK_STRINGCON);    }
{BADCHAR}      { lexer_badtoken(yytext);               } 
{BADSTRING}    { lexer_badtoken(yytext);               }

"="            { return yylval_token ('=');  }
"+"            { return yylval_token ('+');  }
"-"            { return yylval_token ('-');  }
"*"            { return yylval_token ('*');  }
"/"            { return yylval_token ('/');  }
"("            { return yylval_token ('(');  }
")"            { return yylval_token (')');  }
";"            { return yylval_token (';');  }
"["            { return yylval_token ('[');  }
"]"            { return yylval_token (']');  }
"{"            { return yylval_token ('{');  }
"}"            { return yylval_token ('}');  }
","            { return yylval_token (',');  }
"."            { return yylval_token ('.');  }
"%"            { return yylval_token ('%');  }
"!"            { return yylval_token ('!');  }

.              { lexer_badchar (*yytext); }

%%
