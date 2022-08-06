#ifndef _LIB_H
#define _LIB_H

#define tt_assert(x)                                                 \
    if (!(x))                                                        \
    {                                                                \
        printf("assert failed: %s %s %d\n", #x, __FILE__, __LINE__); \
        msg_print_last();                                            \
        abort();                                                    \
    }

/** @addtogroup namelist 
@{
*/
/** name */
typedef const char *t_name;
/** 
structure containing the counter and array of names.
*/
typedef struct s_namelist {
    int count; ///< number of names in the list
    t_name *names; ///< array of names
} t_namelist;
/** array of names */
typedef struct s_names *t_names;
/**
a linked list of names
*/
struct s_names {
    char *name; ///< name
    t_names next; ///< next
};
/** @} */
/**
@addtogroup internal_structures
@{ */


/** a list of expressions */
typedef struct s_expression_list {
    int count; ///< number of expression defined
    struct s_expression **list; ///< list
} t_expression_list;


/**
method pattern with the selector parts concatenated and the
list of names in order.
*/
struct s_pattern {
    char *selector; ///< selector 
    t_names params; ///< parameter names
};
/** patterns */
typedef struct s_pattern *t_pattern;


/**
a class has an id (not pouplated right now)
also consists of a name, the name of the metaclass and the name of the
super class.
the environment should not be used anymore.
*/
typedef struct s_classdef {
    int id;                                      ///< number for identification (not yet used)
    char *name;                                  ///< name
    char *meta;                                  ///< name of meta class
    char *super;                                 ///< name of super class
    struct s_env *env;                           ///< unused
} t_classdef;

/** statement type */
typedef enum e_statement_type {
    stmt_return = 100,
    stmt_assign,
    stmt_message
} t_statement_type;

/**
linked list of statements. 
The type, expression and the next statments.
*/
typedef struct s_statements {
    t_statement_type type;                       ///< type
    struct s_expression *expr;                   ///< expression
    struct s_statements *next;                   ///< next statement
} t_statements;

/**
method definition
*/
typedef struct s_methoddef {
    char *sel;                                   ///< selector
    t_namelist args;                             ///< arguments
    t_namelist locals;                           ///< locals
    t_statements *statements;                    ///< statements
    struct s_env *env;                           ///< unused
} t_methoddef;

/**
message pattern as it is parsed.
*/
typedef struct s_message_pattern {
    t_namelist parts;                            ///< parts of the pattern
    t_namelist names;                            ///< names of the pattern
} t_message_pattern;

/** expression type */
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

/**
 an assignment consists of a target name and an expression 
 */
typedef struct s_assignment {
    const char *target;                          ///< name that should be assigned
    struct s_expression *value;                  ///< value that is evaluated and assigned to the name
} t_assignment;

/** a block has parameters, locals and statments
the environment should not be used anymore.
*/
typedef struct s_block {
    t_namelist params;                           ///< list of defined parameters
    t_namelist locals;                           ///< list of local variables
    t_statements *statements;                    ///< statements
    struct s_env *env;                           ///< unused
} t_block;

/** an expression
is either an integer, a string, a symbol, an assignment, a block, or a message call 
*/
typedef struct s_expression {
    t_expression_tag tag;                        ///< tag
    union {
        int intvalue;                            ///< integer value
        const char *strvalue;                    ///< string value
        const char *ident;                       ///< symbol
        t_expression_list exprs;                 ///< expression list
        struct msg {
            struct s_expression *target;         ///< target
            struct s_messages *m;                ///< messages
        } msg;
        t_assignment assignment;                 ///< assignment
        t_block block;                           ///< block
    } u;                                         ///< union
} t_expression;

/** message calling structure 
*/
typedef struct s_messages {
    bool cascaded;                               ///< either it's cascaded or nested nested means the result of the previous method invokation is used as the target for the next invokation cascaded means that all methods are invoked on the same first target.
    char *sel;                                   ///< selector
    int argc;                                    ///< argument count
    t_expression **args;                         ///< expressions for the arguments
    struct s_messages *next;                     ///< next message, if any
} t_messages;

/** a cascade of messages to the same target. 
is it still used somewhere?
*/
typedef struct s_message_cascade {
    t_messages *msgs;                            ///< messages
    struct s_message_cascade *next;              ///< next messages
} t_message_cascade;


/** message handler */
typedef struct s_object *( *t_message_handler ) ( struct s_object *,
                                                  const char *sel,
                                                  struct s_object ** args );
/**
an object contains mainly of the handler function and some data area.
broken down into some specific alternatives for convenience.
(but are not so convenient)
*/
typedef struct s_object {
    t_message_handler handler;                   ///< handler for the messages
    union {
        void *data;                              ///< opacue data pointer
        int intval;                              ///< integer value
        struct {
            int i[10];                           ///< list of integer values
            void *p[10];                         ///< list of pointer values
        } vals;                                  ///< values
        struct {
            struct s_object **vs;                ///< list of object values
            int cnt;                             ///< count of objects in the list
        } vars;                                  ///< general vars
    } u;                                         ///< union
} t_object;

/**
one slot of the environment.
A linked list of name and value pairs. Values are expressed as objects.
Names have to be cstrings.
*/
typedef struct s_slot {
    const char *name; ///< name
    t_object *val; ///< value
    struct s_slot *next; ///< next slot
} t_slot;

/**
an environment is a list of slots.
and also points to a lower environment.
The names in this environment supersede the ones in the lower
environments.
*/
typedef struct s_env {
    t_slot *slots;                               ///< slots
    struct s_env *next;                          ///< next environment
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
