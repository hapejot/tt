%include{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "talloc.h"

    #include "global.h"
    #include "lib.h"
    struct gd gd;
    #define YYCOVERAGE

}

%token_type {char*}
%token_destructor{ free($$); }
%token_prefix TK_
/*
%type statements {struct ast*}
%type stmt {struct ast*}
%type expr {struct ast*}
%type ident {struct ast*}
%type atom {struct ast*}
%type unary_call {struct ast*}
%type binary_call {struct ast*}
%type class_defs {struct ast*}
%type idents {struct ast*}
%type var_list {struct ast*}
%type method_defs {t_method}
%type arg_defs {struct ast*}
*/

%type msg_pattern {t_pattern}

all ::= class_defs.
class_defs ::=.
class_defs ::= class_defs class_ident LBRACK var_list method_defs RBRACK.
class_defs ::= class_defs class_ident LBRACK method_defs RBRACK.
class_defs ::= class_defs class_ident LARROW IDENT LBRACK var_list method_defs RBRACK.

/* returns a class definition */
class_ident ::= IDENT(name).
    {   printf("object: %s\n", name); }
class_ident ::= IDENT(name) IDENT(unary).
    {   printf("object %s: %s\n", unary, name); }

/* returns a list of identifiers */
var_list ::= BAR idents BAR.

/* returns list of identifiers */
idents ::= IDENT.  {  }
idents ::= idents IDENT.

/*
optional_directive ::= directive.
optional_directive ::= .
*/

directive ::= LT tokens GT.

tokens ::= .
tokens ::= tokens token.

token ::= LBRACK.
token ::= RBRACK.
token ::= LARROW.
token ::= IDENT.
token ::= BAR.
token ::= DOT.
token ::= VERBATIM.
token ::= BINOP.
token ::= KEYWORD.
token ::= UARROW.
token ::= STRING.
token ::= CHAR.
token ::= SYMBOL.
token ::= NUMBER.
token ::= LBRACE.
token ::= RBRACE.
token ::= COLON.
token ::= SEMICOLON.

method_defs ::=.
method_defs ::= method_defs msg_pattern(P) LBRACK statements(S) RBRACK.
    { method_def(P, NULL, NULL, S); }
method_defs ::= method_defs msg_pattern(P) LBRACK var_list(L) statements(S) RBRACK.
    { method_def(P, L, NULL, S); }
method_defs ::= method_defs msg_pattern(P) var_list(L) LBRACK statements(S) RBRACK.
    { method_def(P, L, NULL, S); }
method_defs ::= method_defs msg_pattern(P) LBRACK  RBRACK.
    { method_def(P, NULL, NULL, NULL); }
method_defs ::= method_defs msg_pattern(P) VERBATIM(C).
    { method_def_verb(P, C);   }

msg_pattern ::= unary_pattern.
msg_pattern ::= binary_pattern.
msg_pattern ::= keyword_pattern.

binop(op) ::= BINOP(op) .
binop ::= LT.
binop ::= GT.



unary_pattern ::= IDENT(name).
    { printf("method: %s\n", name); }
binary_pattern ::= binop(name) IDENT.
    { printf("method: %s\n", name); }
keyword_pattern ::= KEYWORD(name) IDENT.
    { printf("keyword: %s\n", name); }
keyword_pattern ::= keyword_pattern KEYWORD(name) IDENT.
    { printf("keyword: %s\n", name); }

statements ::= return_statement.
statements ::= statement.
statements ::= statement DOT.
statements ::= statement DOT statements.

statement ::= expression.
statement ::= directive.
statement ::= directive expression.

return_statement ::= UARROW expression.
return_statement ::= directive UARROW expression.

/* expression structure */
expression ::= IDENT LARROW expression.
expression ::= basic_expression.

/* expression structure */
basic_expression ::= primary.
basic_expression ::= primary messages cascaded_messages.
basic_expression ::= primary cascaded_messages.
basic_expression ::= primary messages.

/* expression structure */
primary ::= IDENT.
primary ::= STRING.
primary ::= CHAR.
primary ::= SYMBOL.
primary ::= NUMBER.
primary ::= LBRACK block_body RBRACK.
primary ::= LBRACE expression RBRACE.

/* block */
block_body ::= block_arguments BAR var_list statements.
block_body ::= block_arguments BAR statements.
block_body ::= var_list statements.
block_body ::= statements.
block_body ::= var_list.
block_body ::= .

/* returns list of identifiers */
block_arguments ::= COLON IDENT.
block_arguments ::= block_arguments COLON IDENT.

/* expression */
messages ::= unary_messages.
messages ::= unary_messages keyword_message.
messages ::= unary_messages binary_messages.
messages ::= unary_messages binary_messages keyword_message.
messages ::= binary_messages.
messages ::= binary_messages keyword_message.
messages ::= keyword_message.


/* message expression */
unary_messages ::= IDENT.
unary_messages ::= unary_messages IDENT.

/* message expression */
binary_messages ::= binary_message.
binary_messages ::= binary_message binary_messages.
binary_message ::= binop binary_argument.
binary_argument ::= primary unary_messages.
binary_argument ::= primary.

/* message expression */
keyword_message ::= KEYWORD keyword_argument.
keyword_message ::= keyword_message KEYWORD keyword_argument.

keyword_argument ::= primary.
keyword_argument ::= primary unary_messages.
keyword_argument ::= primary unary_messages binary_messages.
keyword_argument ::= primary binary_messages.

/* cascaded message expression */
cascaded_messages ::= SEMICOLON messages.
cascaded_messages ::= cascaded_messages SEMICOLON messages.

%code {
int main(int argc, char **argv){  
    yyParser xp;
    ParseInit(&xp);
    ParseTrace(stdout, "    |");
    while(nextToken()){
        Parse(&xp, gd.token, strdup(gd.buf)); // strdup is okay. will be "freed" using token destructor.
    }
    Parse(&xp, 0, NULL);
    ParseFinalize(&xp);
    // ParseCoverage(stdout);
    return 0;
}
}
