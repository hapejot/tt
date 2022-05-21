#ifndef _LIB_H
#define _LIB_H

/**
@addtogroup internal_structures
@{ */
typedef struct s_names * t_names;
struct s_names {
    char *name;
    t_names next;
};

struct s_pattern {
    char* selector;
    t_names params;
};
typedef struct s_pattern * t_pattern;

/** @} */

typedef struct _Assign *Assign;
typedef struct _Assigns *Assigns;
extern Assigns global_assigns;
struct context {
    struct context * super;
    bool ctx_class;
    const char* name;
};

struct contextdef {
    bool global;
    bool instance;
    bool local;
};
/**
@defgroup assign Assign
@{
*/
Assign Assign_new( char * name, char * value );
Assigns Assigns_new( Assign assign, Assigns next );
void Assign_add( char * name, char * value );
Assign Assign_find( char * name );
char *Assign_value( Assign a );
char *Assign_name( Assign a );

void Assigns_dump( Assigns as );
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

/* @brief read a line from input and store it somewhere.
@return true if successful
*/
bool readLine( void );

/** @brief read one character from input
    and store it somewhere.
@param[in] t c-string of some sort.
@return true if successful
*/
bool readChar( char * t );

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
void ast_to_c( FILE *out, struct ast *ast, struct context * );
void ast_fill_classes( struct ast *ast );
void ast_fill_methods();
void ast_generate_methods();

struct ast* ast_new(int tag);

void class_add_def(const char* name, const char* super, struct ast * vars, struct ast* methods);
void class_add_meta(const char* name, const char* super, struct ast * vars, struct ast* methods);

void string_register(const char* str);

void c_generate(FILE*);

void method_def(t_pattern pattern, void *locals, void *directive, void *statements);
void method_def_verb(t_pattern pattern, void *coding);

int parser_main(int, char**);
#endif


