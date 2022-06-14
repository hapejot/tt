#ifndef _LIB_H
#define _LIB_H

/**
@addtogroup internal_structures
@{ */

typedef char * t_name;
typedef struct s_namelist {
    int count;
    char **names;
} t_namelist;
typedef struct s_names *t_names;

typedef struct s_expression_list {
    int count;
    struct s_expression **list;
} t_expression_list;

struct s_names {
    char *name;
    t_names next;
};

struct s_pattern {
    char *selector;
    t_names params;
};
typedef struct s_pattern *t_pattern;


typedef struct s_classdef {
    int id;
    char *name;
    char *meta;
    char *super;
} t_classdef;

typedef enum e_statement_type {
    stmt_return = 100,
    stmt_assign,
    stmt_message
} t_statement_type;

typedef struct s_statements {
    t_statement_type type;
    struct s_expression *expr;
    struct s_statements *next;
} t_statements;

typedef struct s_methoddef {
    char *sel;
    t_namelist args;
    t_namelist locals;
    t_statements *statements;
} t_methoddef;

typedef struct s_message_pattern {
    t_namelist parts;
    t_namelist names;
} t_message_pattern;

typedef enum e_expression_tag {
    tag_string,
    tag_char,
    tag_message,
    tag_number,
    tag_ident,
    tag_block,
    tag_array,
    tag_assignment
} t_expression_tag;

typedef struct s_assignment {
    const char *target;
    struct s_expression *value;
} t_assignment;

typedef struct s_block {
    t_namelist params;
    t_namelist locals;
    t_statements *statements;
} t_block;

typedef struct s_expression {
    t_expression_tag tag;
    union {
        int intvalue;
        const char *strvalue;
        const char *ident;
        t_expression_list exprs;
        struct msg {
            struct s_expression *target;
            struct s_messages *m;
        } msg;
        t_assignment assignment;
        t_block block;
    } u;
} t_expression;

typedef struct s_messages {
    bool cascaded;              // either it's cascaded or nested 
// nested means the result of the previous method invokation is used as the target for the next invokation
// cascaded means that all methods are invoked on the same first target.
    char *sel;
    int argc;
    t_expression **args;
    struct s_messages *next;
} t_messages;

typedef struct s_message_cascade {
    t_messages *msgs;
    struct s_message_cascade *next;
} t_message_cascade;


typedef struct s_object *( *t_message_handler ) ( struct s_object *,
                                                  const char *sel,
                                                  struct s_object ** args );
typedef struct s_object {
    t_message_handler handler;
    union {
        void *data;
        int intval;
        struct {
            int i[10];
            void *p[10];
        } vals;
        struct {
        struct s_object **vs;
        int cnt;
        } vars;
    } u;
    struct s_env *env;
} t_object;

typedef struct s_slot {
    const char *name;
    t_object *val;
    struct s_slot *next;
} t_slot;
typedef struct s_env {
    t_slot * slots;
    struct s_env * next;
} t_env;

/** @} */

/**
@addtogroup tokenizer
@{
*/
/**
@brief check if character is part of an identifier.
@param[in] c  character to classify.
@returns true if if c is an identifier character.
*/
bool is_ident_char( int c );

/**
clear and initialize the source that will alter be parsed.

needs to be called before using *src_add*.  *src_read* will do
it automatically.
*/
bool src_clear(  );

/**
dumps all the lines of the current source.
*/
bool src_dump(  );

/** adding one line to the source that will be parsed. 
*/
bool src_add( const char * );
/** read a file into src itab.
*/
bool src_read( const char * );

/* @brief read a line from input and store it somewhere.
   @return true if successful
 */
bool readLine( void );

/** @brief read one character from input
    and store it somewhere.
@param[in] t c-string of some sort.
@return true if successful
*/
bool readChar( char *t );

/** @brief read string token.
@return true if successful
*/
bool readStringToken( void );

/** @brief read next token. 
@return true if successful
*/
bool nextToken( void );

/** @} */


void ast_dump( int level, struct ast *ast );
void ast_fill_classes( struct ast *ast );
void ast_fill_methods(  );
void ast_generate_methods(  );

struct ast *ast_new( int tag );

//void class_add_def(const char* name, const char* super, struct ast * vars, struct ast* methods);
//void class_add_meta(const char* name, const char* super, struct ast * vars, struct ast* methods);
void class_enter( const char *name );
void class_dump_all(  );

t_methoddef *method_read( const char *class, const char *selector );
void method_enter( t_message_pattern * );
void method_stmts( t_statements * );

void string_register( const char *str );

void c_generate( FILE * );

void method_def( t_pattern pattern, void *locals, void *directive,
                 void *statements );
void method_def_verb( t_pattern pattern, void *coding );

int parser_main( int, char ** );

void namelist_init( t_namelist * );
void namelist_add( t_namelist *, const t_name );
void namelist_copy( t_namelist * to, t_namelist * from );


void *itab_read( struct itab *, const char * );

void msg_add( const char *msg, ... );
void msg_print_last(  );

void message_add_msg( t_messages * ms, t_messages * m );
void message_add_arg( t_messages * m, t_expression * x );

#endif
