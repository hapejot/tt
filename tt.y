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
%token_destructor{ /* fprintf(stderr, "free: %s\n", $$); */  free($$); }
%token_prefix TK_

%type statements                {t_statements*}
%type return_statement          {t_statements*}
%type statement                 {t_statements*}
%type expression                {t_expression*}
%type primary                   {t_expression*}
%type binary_argument           {t_expression*}
%type block_body                {t_expression*}
%type keyword_argument          {t_expression*}
%type basic_expression          {t_expression*}
%type assignment_expression     {t_expression*}
%type inline_array              {t_expression*}

%type block_arguments           {t_namelist}

%type expression_sequence       {t_expression_list}

%type msg_pattern               {t_message_pattern*}
%type keyword_pattern           {t_message_pattern*}
%type unary_pattern             {t_message_pattern*}
%type binary_pattern            {t_message_pattern*}


%type messages                  {t_messages*}
%type unary_messages            {t_messages*}
%type binary_message            {t_messages*}
%type binary_messages           {t_messages*}
%type keyword_message           {t_messages*}
%type cascaded_messages         {t_messages*}

all ::= class_defs.
class_defs ::=.
class_defs ::= class_defs class_ident LBRACK var_list method_defs RBRACK.
class_defs ::= class_defs class_ident LBRACK method_defs RBRACK.
class_defs ::= class_defs class_ident LARROW IDENT LBRACK var_list method_defs RBRACK.

/* returns a class definition */
class_ident ::= IDENT(name).
    {   class_enter(name); }
class_ident ::= IDENT(name) IDENT(unary).
    {   class_enter(name); 
        if(strcmp(unary, "class") == 0) { 
            printf("class\n");
        } 
    }
class_ident ::= IDENT(SUPER) KEYWORD(KW) IDENT(ARG).
    {   printf("object %s: %s %s\n", SUPER, KW, ARG); }

/* returns a list of identifiers */
var_list ::= BAR idents BAR.

/* returns list of identifiers */
idents ::= IDENT.  {  }
idents ::= idents IDENT.

optional_directive ::= directive.
optional_directive ::= .

directive ::= LT tokens GT.


tokens ::= .
tokens ::= tokens token.

token ::= LBRACK.
token ::= RBRACK.
token ::= LARROW.
/* idents result in a parsing conflict
token ::= IDENT.
*/
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
token ::= ASSIGN.

method_defs ::=.
method_defs ::= method_defs msg_pattern LBRACK statements(S) RBRACK.
    { method_stmts(S); }
method_defs ::= method_defs msg_pattern LBRACK var_list(L) statements(S) RBRACK.
    { (void)L;
        (void)S; }
method_defs ::= method_defs msg_pattern var_list(L) LBRACK statements(S) RBRACK.
    {   (void)L;
        (void)S;}
method_defs ::= method_defs msg_pattern LBRACK  RBRACK.
    {   }
method_defs ::= method_defs msg_pattern VERBATIM(C).
    { (void)C;  }
method_defs ::= method_defs directive.
method_defs ::= method_defs IDENT IDENT BINOP msg_pattern LBRACK optional_directive var_list statements RBRACK.

msg_pattern ::= unary_pattern(p).
    { method_enter(p); }
msg_pattern ::= binary_pattern(p).
    { method_enter(p); }
msg_pattern ::= keyword_pattern(p).
    { method_enter(p); }

binop(op) ::= BINOP(op) .
binop ::= LT.
binop ::= GT.

unary_pattern(mp) ::= IDENT(message).
    {   mp = talloc_zero(NULL, t_message_pattern);
        namelist_init(&mp->parts);
        namelist_init(&mp->names);
        namelist_add(&mp->parts, message);
    }
binary_pattern(mp) ::= binop(message) IDENT(name).
    {   mp = talloc_zero(NULL, t_message_pattern);
        namelist_init(&mp->parts);
        namelist_init(&mp->names);
        namelist_add(&mp->parts, message);
        namelist_add(&mp->names, name);
    }
keyword_pattern(mp) ::= KEYWORD(message) IDENT(name).
    {   mp = talloc_zero(NULL, t_message_pattern);
        namelist_init(&mp->parts);
        namelist_init(&mp->names);
        namelist_add(&mp->parts, message);
        namelist_add(&mp->names, name);
    }
keyword_pattern(mp) ::= keyword_pattern(mp) KEYWORD(message) IDENT(name).
    {   
        namelist_add(&mp->parts, message);
        namelist_add(&mp->names, name);
    }

statements(S) ::= return_statement(S).
statements(S) ::= statement(S).
statements(S) ::= statement(S) DOT.
statements(S) ::= statement(S0) DOT statements(STMS).
    {   S = S0;
        S->next = STMS; }

statement(S) ::= expression(E).
    {   S = talloc_zero(NULL, t_statements);
        S->next = NULL;
        S->type = stmt_message;
        S->expr = E;
    }
statement ::= directive.
statement ::= directive expression.

return_statement(S) ::= UARROW expression(E).
    {   S = talloc_zero(NULL, t_statements);
        S->next = NULL;
        S->type = stmt_return;
        S->expr = E;
    }

return_statement(R) ::= directive return_statement(RS).
    {   R = RS; }

/* expression structure */
assignment_expression(E) ::= IDENT(S) LARROW expression(E0).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_assignment;
        E->u.assignment.target = talloc_strdup(E, S);
        E->u.assignment.value = E0; }
assignment_expression(E) ::= IDENT(S) ASSIGN expression(E0).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_assignment;
        E->u.assignment.target = talloc_strdup(E, S);
        E->u.assignment.value = E0; }

expression(E) ::= basic_expression(E).
    {   talloc_get_type(E, t_expression);  }
expression(E) ::= assignment_expression(E).
    {   talloc_get_type(E, t_expression);  }

/* expression structure */
basic_expression(E) ::= primary(E).
    {   talloc_get_type(E, t_expression);  }
basic_expression(E) ::= primary(P) messages(MS) cascaded_messages(CMS).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_message;
        E->u.msg.target = P;
        E->u.msg.m = MS;
        MS->next = CMS;
        for(t_messages *m = MS; m ; m = m->next )
            printf("*** MSG: %s\n", m->sel);
    }
/* basic_expression ::= primary cascaded_messages. */

basic_expression(e) ::= primary(p) messages(msgs).
    {   e = talloc_zero(NULL, t_expression);
        e->tag = tag_message;
        e->u.msg.target = p;
        e->u.msg.m = msgs;
    }
/* expression structure */
primary(E) ::= IDENT(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_ident;
        E->u.ident = talloc_strdup(E, S);
    }
primary(e) ::= STRING(s).
    {   e = talloc_zero(NULL, t_expression);
        e->tag = tag_string;
        e->u.strvalue = talloc_strdup(e, s);
    }
primary(E) ::= CHAR(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_char;
        E->u.intvalue = S[0]; }
primary ::= SYMBOL.
    { assert(false); }
primary(E) ::= NUMBER(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_number;
        E->u.intvalue = atoi(S);
        printf("#### %s -> %d\n", S, E->u.intvalue);
        }
primary(E) ::= LBRACK block_body(B) RBRACK.
    {   E = talloc_get_type(B, t_expression); }
primary(E) ::= LPAREN expression(EX) RPAREN.
    {   E = talloc_get_type(EX, t_expression); }
primary(E) ::= inline_array(E).
    {   talloc_get_type(E, t_expression); }

inline_array(E) ::= LBRACE expression_sequence(XS) RBRACE.
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_array;
        E->u.exprs = XS;
    }
expression_sequence(E) ::= expression(X).
    {   // E = talloc_zero(NULL, t_expression_list);
        E.count = 1;
        E.list = talloc_zero(NULL, t_expression*);
        E.list[0] = X;
    }
expression_sequence(E) ::= expression_sequence(XS) DOT expression(X).
    {   E = XS;
        E.count++;
        E.list = talloc_realloc(E.list, E.list, t_expression*, E.count);
        E.list[E.count-1] = X;
    }

/* block */
block_body(E) ::= block_arguments(ARGS) BAR var_list statements(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_block;
        E->u.block.statements = S;
        E->u.block.params = ARGS; }
block_body(E) ::= block_arguments(ARGS) BAR statements(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_block;
        E->u.block.statements = S;
        E->u.block.params = ARGS; }
block_body(E) ::= var_list statements(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_block; 
        E->u.block.statements = S;}
block_body(E) ::= statements(S).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_block; 
        E->u.block.statements = S;}
block_body(E) ::= var_list.
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_block; }
block_body(E) ::= .
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_block; }

/* returns list of identifiers */
block_arguments(A) ::= COLON IDENT(S).
    {   namelist_init(&A);
        namelist_add(&A, S); }
block_arguments(A) ::= block_arguments(A) COLON IDENT(S).
    {   namelist_add(&A, S); }

/* expression */
messages(M) ::= unary_messages(M).
messages(M) ::= unary_messages(M0) keyword_message(M1).
    {   M = M0;
        message_add_msg(M, M1); }
messages(M) ::= unary_messages(M0) binary_messages(M1).
    {   M = M0;
        message_add_msg(M, M1); }
messages(M) ::= unary_messages(M0) binary_messages(M1) keyword_message(M2).
    {   M = M0;
        message_add_msg(M, M1); 
        message_add_msg(M, M2); }
messages(M) ::= binary_messages(M).
messages(M) ::= binary_messages(M0) keyword_message(M1).
    {   M = M0;
        message_add_msg(M, M1); }
messages(M) ::= keyword_message(M).


/* message expression */
unary_messages(M) ::= IDENT(S).
    {   M = talloc_zero(NULL, t_messages);
        M->sel = talloc_strdup(M, S);
    }
unary_messages(M) ::= unary_messages(M0) IDENT(S).
    {   M = M0;
        t_messages * M1 = M->next = talloc_zero(M0, t_messages);
        M1->sel = talloc_strdup(M1, S);
    }

/* message expression */
binary_messages(M) ::= binary_message(M).
binary_messages(M) ::= binary_message(M0) binary_messages(M1).
    {   M = M0;
        M->next = M1; }

binary_message(M) ::= binop(S) binary_argument(E).
    {   M = talloc_zero(NULL, t_messages);
        M->sel = talloc_strdup(M, S);
        M->args = talloc_zero(M, t_expression*);
        M->args[0] = E;
        M->argc = 1;
    }

binary_argument(E) ::= primary(E0) unary_messages(M0).
    {   E = talloc_zero(NULL, t_expression);
        E->tag = tag_message;
        E->u.msg.target = E0;
        E->u.msg.m = M0; 
        printf("TALLOC NAME: %s\n", talloc_get_name(E)); 
    }
binary_argument(E) ::= primary(E).
    {   printf("TALLOC NAME: %s\n", talloc_get_name(E)); } 

/* message expression */
keyword_message(M) ::= KEYWORD(S) keyword_argument(E).
    {   M = talloc_zero(NULL, t_messages);
        M->sel = talloc_strdup(M, S);
        M->argc = 1;
        M->args = talloc_zero_array(M, t_expression*, 1);
        M->args[0] = E;
    }

keyword_message(M) ::= keyword_message(M) KEYWORD(S) keyword_argument(E).
    {   
        printf("TALLOC NAME: %s\n", talloc_get_name(E));
        M->sel = talloc_strdup_append(M->sel, S);
        M->argc++;
        M->args = talloc_realloc(M, M->args, t_expression*, M->argc);
        M->args[M->argc-1] = E;
    }
// keyword_argument(E) ::= primary(E).
//     {   printf("TALLOC NAME: %s\n", talloc_get_name(E)); }

keyword_argument(E) ::= binary_argument(E).
    {   printf("TALLOC NAME: %s\n", talloc_get_name(E)); }
    
keyword_argument ::= primary unary_messages binary_messages.
    {   assert(false); }
keyword_argument ::= primary binary_messages.
    {   assert(false); }

/* cascaded message expression */
cascaded_messages(CMS) ::= SEMICOLON messages(MS).
    { CMS = MS;
        printf("cascade %d: %s\n",__LINE__, MS->sel);
    }
cascaded_messages(CMS) ::= SEMICOLON messages(MS) cascaded_messages(MSX).
    {
        CMS = MS;
        CMS->next = MSX;
        printf("cascade %d: %s\n",__LINE__, MS->sel);
    }

%syntax_error {
    printf("*** Syntax Error\n");
    printf("*** Line %d: %s\n", gd.line_count, gd.line);
    printf("*** Pos %d\n", gd.pos);
}

%code {
void parse(){
    yyParser xp;
    ParseInit(&xp);
    ParseTrace(stdout, "    |");
    while(nextToken()){
        Parse(&xp, gd.token, strdup(gd.buf)); // strdup is okay. will be "freed" using token destructor.
    }
    Parse(&xp, 0, NULL);
    ParseFinalize(&xp);
}


/*
int main(int argc, char **argv){  
    assert(argc == 2);
    readFile(argv[1]);
    exit(-1);
    parse();
    // ParseCoverage(stdout);
    return 0;
}
*/
}
