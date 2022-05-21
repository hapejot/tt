#include "global.h"
#include "tt.h"
#include <assert.h>
#include <talloc.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>

#include "lib.h"

#define METACLASS "MetaClass"
#define STRING "String"

/** Assignment */
typedef struct _Assign *Assign;

/** Assignments */
typedef struct _Assigns *Assigns;

/** all classes */
struct itab *classes = NULL;

/** all methods */
struct itab *methods = NULL;

/** all method names */
struct itab *method_names = NULL;

/** all global variables */
struct itab *variables = NULL;

/** all strings */
struct itab *strings = NULL;

/** number of strins */
int string_count = 0;

/** number of string classes */
int string_class_num = 0;

/** details of a class */
struct classinfo {
    bool meta;
    char *name;
    char *super;
    int num;
};

/** details of a method */
struct methodinfo {
    char *classname;
    char *name;
};

/** details of a global variable */
struct varinfo {
    char *classname;
    char *name;
};

/** details of a string */
struct stringinfo {
    int num;
};

/** single assignment of a value to string */
struct _Assign {
    char *name;
    char *value;
};

/** list of assignment of a value to string */
struct _Assigns {
    Assign assign;
    Assigns next;
};

/** global assignments */
Assigns global_assigns = NULL;

/** create new assignment pair */
Assign Assign_new( char *name, char *value ) {
    Assign result = malloc( sizeof( *result ) );
    result->name = strdup( name );
    result->value = strdup( value );
    return result;
}

/** return the value of the assignment pair */
char *Assign_value( Assign a ) {
    return a->value;
}
/** return the name of the assignment pair */
char *Assign_name( Assign a ) {
    return a->name;
}

/** create a new assignment list */
Assigns Assigns_new( Assign assign, Assigns next ) {
    Assigns result = malloc( sizeof( *result ) );
    result->assign = assign;
    result->next = next;
    return result;
}

/** add an assignment pair to the global list. No additional checks are done. */
void Assign_add( char *name, char *value ) {
    Assign a = Assign_new( name, value );
    global_assigns = Assigns_new( a, global_assigns );
}

/** find a pair in the global assignment list with the given name */
Assign Assign_find( char *name ) {
    for( Assigns as = global_assigns; as; as = as->next ) {
        if( strcmp( name, as->assign->name ) == 0 ) {
            return as->assign;
        }
    }
    return NULL;
}

/** dump the contents of the global assignment list */
void Assigns_dump( Assigns as ) {
    for( ; as; as = as->next ) {
        printf( "%s <- %s\n", as->assign->name, as->assign->value );
    }
}

/**
* @defgroup itab ITab
* sorted list of structures -> tables with primary index
* @{
*******************************************************************/
/**
 @brief structure of an entry in the itab.
 */
struct itab_entry {
    const char *key;
    void *value;
};
/**
 @brief structure of itab
 */
struct itab {
    int total;
    int used;
    struct itab_entry *rows;
};

/**
 @brief iterator over elements of an itab.
 */

struct itab_iter {
    struct itab *tab;
    int pos;
};

/** 
 @brief create a new itab with default parameters.
 @return reference to an itab structure.

 Detailed description follows here.
 */
struct itab *itab_new(  ) {
    struct itab *r = talloc( NULL, struct itab );
    r->total = 10;
    r->used = 0;
    r->rows = talloc_array( r, struct itab_entry, r->total );
    return r;
}

/**
@brief compares the keys of two entries
@return \arg < 0, when first key is lower
        \arg == 0, when both keys are equal
        \arg > 0, when second key is lower
*/
int itab_entry_cmp( const void *aptr, const void *bptr ) {
    const struct itab_entry *a = aptr;
    const struct itab_entry *b = bptr;
    return strcmp( a->key, b->key );
}

void itab_append( struct itab *itab, const char *key, void *value ) {
    assert( itab );
    if( itab->total == itab->used ) {
        itab->total *= 2;
        itab->rows =
                talloc_realloc( itab, itab->rows, struct itab_entry,
                                itab->total );
    }
    struct itab_entry *row = &itab->rows[itab->used];
    row->key = talloc_strdup( itab, key );
    row->value = value;
    itab->used++;


    qsort( itab->rows,                 // base
           itab->used,                 // nmemb
           sizeof( struct itab_entry ), // size
           itab_entry_cmp );

}

void *itab_read( struct itab *itab, const char *key ) {
    assert( itab );
    assert( key );
    struct itab_entry dummy = { key, NULL };
    struct itab_entry *r = bsearch( &dummy,
                                    itab->rows,
                                    itab->used,
                                    sizeof( struct itab_entry ),
                                    itab_entry_cmp );
    if( r )
        return r->value;
    else
        return NULL;
}

void itab_dump( struct itab *itab ) {
    assert( itab );
    for( int i = 0; i < itab->used; i++ ) {
        fprintf( stderr, "%s: %p\n", itab->rows[i].key, itab->rows[i].value );
    }
}

struct itab_iter *itab_foreach( struct itab *tab ) {
    if( tab->used > 0 ) {
        struct itab_iter *r = talloc_zero( NULL, struct itab_iter );
        r->tab = tab;
        r->pos = 0;
        return r;
    }
    else
        return NULL;
}

struct itab_iter *itab_next( struct itab_iter *iter ) {
    iter->pos++;
    if( iter->tab->used > iter->pos ) {
        return iter;
    }
    else {
        talloc_free( iter );
        return NULL;
    }
}

void *itab_value( struct itab_iter *iter ) {
    return iter->tab->rows[iter->pos].value;
}

const char *itab_key( struct itab_iter *iter ) {
    return iter->tab->rows[iter->pos].key;
}

/*! @} */

/**
 @defgroup runtime-context Runtime Context

 @{
*/

struct context *context_new( struct context *super ) {
    struct context *r = talloc_zero( NULL, struct context );
    r->super = super;
    return r;
}

struct contextdef context_global_def = {.global = true};
struct contextdef context_class_def = {.instance = true };

struct contextdef *context_lookup( struct context *ctx, const char *name ) {
    fprintf( stderr, "lookup %s %p %s\n", name, ctx, ctx->name );

    if( ctx->name ) {
        char *lookup = talloc_strdup( NULL, ctx->name );
        lookup = talloc_strdup_append( lookup, "$" );
        lookup = talloc_strdup_append( lookup, name );
        struct varinfo *vref = itab_read( variables, lookup );
        fprintf( stderr, "lookup var: %s %p\n", lookup, vref );
        if( vref )
            return &context_class_def;
    }

    if( ctx->super )
        return context_lookup( ctx->super, name );
    else {
        fprintf( stderr, "-----\n" );
        return &context_global_def;
    }
}

/** @} */

/**
@defgroup tokenizer Tokenizer

convert stdin into tokens.
each token is returned by the call to @see nextToken.

@{
*/

bool is_ident_char( int c ) {
    return isalpha( c ) || isdigit( c ) || c == '_';
}


bool is_binary_char( int c ) {
    switch ( c ) {
        case '!':
        case '%':
        case '&':
        case '*':
        case '+':
        case ',':
        case '/':
        case '<':
        case '=':
        case '>':
        case '?':
        case '@':
        case '\\':
        case '~':
        case '|':                     // sollte laut Vorschlag ein Binary Operator sein. Kollidiert aber mit der temporary declaration.
// das muss dann wohl auf der Syntaxebene geklärt werden.
        case '-':
            return true;
        default:
            return false;
    }
}


/**
@brief read one line from stdin
stores the result into @c{gd.line}.

trailing blanks are removed.
*/
bool readLine(  ) {
    static int line_count = 0;

    char *line = fgets( gd.line, sizeof( gd.line ), stdin );
    line_count++;
    printf( "%2d:%s", line_count, line );
    if( line ) {
        size_t len = strlen( line );
        while( len >= 0 && line[len] <= 32 )
            line[len--] = 0;
        gd.pos = 0;
        gd.state = 1;
        return true;
    }
    else {
        gd.line[0] = 0;
        gd.state = 2;
        return false;
    }
}


bool readChar( char *t ) {
    bool result = true;
    if( gd.state == 0 ) {
        result = readLine(  );
    }
    if( result ) {
        *t = gd.line[gd.pos++];
        while( *t == 0 ) {
            if( readLine(  ) ) {
                *t = gd.line[gd.pos++];
            }
            else {
                result = false;
                break;
            }
        }
    }
    return result;
}

bool readStringToken(  ) {
    int idx = 0;
    char c;
    while( readChar( &c ) && '\'' != c ) {
        if( c == '\\' )
            readChar( &c );
        gd.buf[idx++] = c;
    }
    gd.buf[idx] = 0;
    gd.token = TK_STRING;
    return true;
}

/**
This is a more detailed description.
*/
bool nextToken(  ) {
    char c;
    bool result = false;
    while( true ) {
        while( readChar( &c ) && isspace( c ) );
        if( c == '"' ) {
            while( readChar( &c ) && c != '"' );
        }
        else
            break;
    }
    if( gd.state == 1 ) {
        if( isalpha( c ) ) {
            int idx = 0;
            for( ;; ) {
                gd.buf[idx++] = c;
                readChar( &c );
                if( !is_ident_char( c ) )
                    break;
            }
            if( c == ':' ) {
                gd.buf[idx++] = c;
                gd.token = TK_KEYWORD;
            }
            else {
                gd.pos--;
                gd.token = TK_IDENT;
            }
            gd.buf[idx] = 0;
            result = true;
        }
        else if( is_binary_char( c ) ) {
            for( int idx = 0; is_binary_char( c ); idx++ ) {
                gd.buf[idx] = c;
                gd.buf[idx + 1] = 0;
                readChar( &c );
            }
            gd.pos--;
            gd.token = 0;
            gd.token = TK_BINOP;
            result = true;
            if( strcmp( "<-", gd.buf ) == 0 ) {
                gd.token = TK_LARROW;
                result = true;
            }
            else if( strcmp( "|", gd.buf ) == 0 ) {
                gd.token = TK_BAR;
                result = true;
            }
            else if( 0 == strcmp( "<", gd.buf ) ) {
                gd.token = TK_LT;
                result = true;
            }
            else if( 0 == strcmp( ">", gd.buf ) ) {
                gd.token = TK_GT;
                result = true;
            }
        }
        else if( isdigit( c ) ) {
            int idx = 1;
            while( isdigit( c ) ) {
                gd.buf[idx - 1] = c;
                gd.buf[idx] = 0;
                readChar( &c );
            }
            gd.pos--;
            gd.token = TK_NUMBER;
            result = true;
        }
        else {
            switch ( c ) {
                case '\'':
                    result = readStringToken(  );
                    break;
                case '.':
                    result = true;
                    gd.token = TK_DOT;
                    break;
                case ';':
                    result = true;
                    gd.token = TK_SEMICOLON;
                    break;
                case '(':
                    result = true;
                    gd.token = TK_LBRACE;
                    break;
                case ')':
                    result = true;
                    gd.token = TK_RBRACE;
                    break;
                case '[':
                    result = true;
                    gd.token = TK_LBRACK;
                    break;
                case ']':
                    result = true;
                    gd.token = TK_RBRACK;
                    break;
                case '{':
                    {
                        int i = 0;
                        gd.buf[i] = 0;
                        readChar( &c );
                        while( c != '}' ) {
                            gd.buf[i++] = c;
                            gd.buf[i] = 0;
                            readChar( &c );
                        }
                        gd.token = TK_VERBATIM;
                    }
                    result = true;
                    break;
                case '#':
                    readChar( &c );
                    for( int idx = 0; is_ident_char( c ) || c == ':'; idx++ ) {
                        gd.buf[idx] = c;
                        gd.buf[idx + 1] = 0;
                        readChar( &c );
                    }
                    gd.pos--;
                    gd.token = TK_SYMBOL;
                    result = true;
                    break;
                case '^':
                    result = true;
                    gd.token = TK_UARROW;
                    break;
                case ':':
                    result = true;
                    gd.token = TK_COLON;
                    break;
                case '$':
                    result = true;
                    gd.token = TK_CHAR;
                    readChar( &c );
                    gd.buf[0] = c;
                    gd.buf[1] = 0;
                    break;
                default:
                    gd.pos--;
                    break;
            }
        }
    }
    return result;
}

/** @} */

void ast_vars_to_c( FILE * out, struct ast *cls, char *super,
                    struct ast *vars ) {
    for( struct ast * c = cls; c; c = c->u.cls.next ) {
        if( super && strcmp( super, c->u.cls.name ) == 0 ) {
            ast_vars_to_c( out, cls, c->u.cls.super, c->u.cls.vars );
            break;
        }
    }
    for( struct ast * v = vars; v; v = v->u.names.next ) {
        fprintf( out, "  void * %s;\n", v->u.names.v );
    }
}

struct meth {
    char *name;
    struct meth *next;
};


struct ast *super_class_of( struct ast *main, struct ast *class ) {
    const char *super_name = class->u.cls.super;
    for( struct ast * cls = main; cls; cls = cls->u.cls.next ) {
        if( strcmp( super_name, cls->u.cls.name ) == 0 ) {
            return cls;
        }
    }
    return NULL;
}


void ast_methods_to_c( FILE * out, struct ast *ast ) {
}





void ast_to_c( FILE * out, struct ast *ast, struct context *context ) {
    assert( ast );
    switch ( ast->tag ) {
        case AST_UNARY:
            fprintf( out, "%s(-1, (struct Object*)", ast->u.unary.sel );
            ast_to_c( out, ast->u.unary.o, context );
            fprintf( out, ")" );
            break;
        case AST_ASSIGN:
            fprintf( out, "self->%s = ", ast->u.asgn.var );
            ast_to_c( out, ast->u.asgn.expr, context );
            break;
        case AST_IDENT:
            {
                struct contextdef *d = context_lookup( context, ast->u.id.v );
                if( d->instance )
                    fprintf( out, "self->%s", ast->u.id.v );
                else if( d->local )
                    fprintf( out, "%s", ast->u.id.v );
                else if( d->global )
                    fprintf( out, "((struct Object*)&v_%s$Meta)",
                             ast->u.id.v );
            }
            break;

        case AST_STRING:
            {
                struct stringinfo *si = itab_read( strings, ast->u.str.v );
                assert( si );
                fprintf( out, "&v_string%d", si->num );
            }
            break;
        case AST_STMT:
            ast_to_c( out, ast->u.stmt.v, context );
            fprintf( out, ";\n" );
            if( ast->u.stmt.next )
                ast_to_c( out, ast->u.stmt.next, context );
            break;
        case AST_CLASS:
        case AST_META:
            assert( false );
            break;
        default:
            fprintf( out, "/* AST: %d */\n", ast->tag );
            break;
    }
}

void ast_dump( int level, struct ast *ast ) {
    assert( ast );
    printf( "%d ", level );
    for( int i = 0; i < level; i++ ) {
        printf( "    " );
    }
    switch ( ast->tag ) {
        case AST_UNARY:
            printf( "unary call: %s\n", ast->u.unary.sel );
            ast_dump( level + 1, ast->u.unary.o );
            break;
        case AST_ASSIGN:
            printf( "assign: %s\n", ast->u.asgn.var );
            ast_dump( level + 1, ast->u.asgn.expr );
            break;
        case AST_IDENT:
            printf( "ident: %s\n", ast->u.id.v );
            break;
        case AST_STRING:
            printf( "string: %s\n", ast->u.str.v );
            break;
        case AST_STMT:
            printf( ">\n" );
            ast_dump( level + 1, ast->u.stmt.v );
            if( ast->u.stmt.next )
                ast_dump( level, ast->u.stmt.next );
            break;
        case AST_CLASS:
        case AST_META:
            if( ast->tag == AST_CLASS )
                printf( "class: %s > %s\n", ast->u.cls.name,
                        ast->u.cls.super );
            else
                printf( "meta %s\n", ast->u.cls.name );
            if( ast->u.cls.vars )
                ast_dump( level + 1, ast->u.cls.vars );
            if( ast->u.cls.next )
                ast_dump( level, ast->u.cls.next );
            break;
        case AST_METHOD:
            printf( "method: %s\n", ast->u.methods.name );
            if( ast->u.methods.body )
                ast_dump( level + 1, ast->u.methods.body );
            if( ast->u.methods.next )
                ast_dump( level, ast->u.methods.next );
            break;
        case AST_NAMES:
            printf( "::%s\n", ast->u.names.v );
            if( ast->u.names.next )
                ast_dump( level, ast->u.names.next );
            break;
        default:
            printf( "AST: %d\n", ast->tag );
            break;
    }
}

struct ast *ast_new( int tag ) {
    struct ast *r;
    r = talloc( NULL, struct ast );
    r->tag = tag;
    return r;
}


void require_classes(  ) {
    if( !classes ) {
        classes = itab_new(  );
        methods = itab_new(  );
        method_names = itab_new(  );
        variables = itab_new(  );
        strings = itab_new(  );

        gd.classnum = 1;

// class_add_def( METACLASS, "Object", NULL, NULL );
        class_add_def( "Class", "Object", NULL, NULL );
        class_add_def( "Object", NULL, NULL, NULL );
        class_add_def( "String", "Object", NULL, NULL );
        string_class_num = gd.classnum - 1;

        gd.classnum = 100;
    }
}

void clean_method_name( char *mname ) {
    for( int i = strlen( mname ) - 1; i >= 0; i-- ) {
        if( mname[i] == ':' )
            mname[i] = '$';
    }
}


/**
@brief fills the method table from all methods of a class.
*/
void ast_fill_methods( const char *p_clsname, struct ast *p_methods ) {
    char mname[500];

    for( struct ast * m = p_methods; m; m = m->u.methods.next ) {
        strcpy( mname, p_clsname );
        strcat( mname, "$" );
        if( m->u.methods.name )
            strcat( mname, m->u.methods.name );
        else {
        }
        clean_method_name( mname );
        itab_append( methods, mname, m );
        m->u.methods.classname = talloc_strdup( methods, p_clsname );
        printf( "new method: %s\n", mname );

        char *selector = talloc_strdup( method_names, m->meth_name );
        clean_method_name( selector );
        const char *n = itab_read( method_names, selector );
        if( n == NULL )
            itab_append( method_names, m->meth_name, selector );
    }
}

void ast_dump_classes(  ) {
    for( struct itab_iter * cls_iter = itab_foreach( classes );
         cls_iter; cls_iter = itab_next( cls_iter ) ) {
        struct ast *cls = itab_value( cls_iter );
        printf( "%d %s\n", cls->u.cls.num, cls->u.cls.name );
    }
}

void class_add_def( const char *name, const char *super,
                    struct ast *vars, struct ast *methods ) {
    require_classes(  );
    printf( "add def for %s : %s\n", name, super );
    struct ast *odef = itab_read( classes, name );
// Class might exist because its a predefined class.
    if( odef == NULL ) {
        printf( "new class\n" );
        class_add_meta( name, super, NULL, NULL );
        struct classinfo *def = talloc_zero( classes, struct classinfo );
        def->num = gd.classnum++;
        def->name = talloc_strdup( def, name );
        itab_append( classes, name, def );
    }
    ast_fill_methods( name, methods );
    for( struct ast * v = vars; v; v = v->u.names.next ) {
        struct varinfo *vi = talloc_zero( variables, struct varinfo );
        vi->name = talloc_strdup( vi, v->u.names.v );
        vi->classname = talloc_strdup( vi, name );
        char name[1000];
        sprintf( name, "%s$%s", vi->classname, vi->name );
        itab_append( variables, name, vi );
    }
}

void class_add_meta( const char *name, const char *super,
                     struct ast *vars, struct ast *methods ) {
    require_classes(  );
    char metaname[1000];
    sprintf( metaname, "%s$Meta", name );
    char supermeta[1000];
    sprintf( supermeta, "%s$Meta", super );
    printf( "add meta for %s : %s\n", name, super );

    struct classinfo *def;
    def = itab_read( classes, metaname );
    if( !def ) {
        def = talloc_zero( classes, struct classinfo );
        def->meta = true;
        def->num = gd.classnum++;
        def->name = talloc_strdup( def, metaname );
        itab_append( classes, metaname, def );
    }

    ast_fill_methods( metaname, methods );

    for( struct ast * v = vars; v; v = v->u.names.next ) {
        struct varinfo *vi = talloc_zero( variables, struct varinfo );
        vi->name = talloc_strdup( vi, v->u.names.v );
        vi->classname = talloc_strdup( vi, metaname );
        char name[1000];
        sprintf( name, "%s$%s", vi->classname, vi->name );
        itab_append( variables, name, vi );
    }
}

void c_generate_structs( FILE * out ) {
    fprintf( out, "#include \"tt-base.h\"\n" );
    for( struct itab_iter * cls_iter = itab_foreach( classes );
         cls_iter; cls_iter = itab_next( cls_iter ) ) {
        struct classinfo *cls = itab_value( cls_iter );
        fprintf( out, "struct /* %d */ %s ", cls->num, cls->name );
        fprintf( out, "{\n  int tag;\n" "     char *data;\n" );
// ast_vars_to_c( out, cls, cls->u.cls.super, cls->u.cls.vars );
        for( struct itab_iter * var_iter = itab_foreach( variables );
             var_iter; var_iter = itab_next( var_iter ) ) {
            struct varinfo *v = itab_value( var_iter );
            if( strcmp( v->classname, cls->name ) == 0 ) {
                fprintf( out, "  struct Object * %s;\n", v->name );
            }
        }
        fprintf( out, "};\n" );
        if( cls->meta ) {
            fprintf( out, "struct %s v_%s = { .tag = %d }; \n", cls->name,
                     cls->name, cls->num );
        }
    }
}

/**
count all occurences of the given char in the string.

@return number of chars with value c.
*/
int count_char( const char *str, char c ) {
    int result = 0;
    for( int i = 0; str[i] != 0; i++ ) {
        if( str[i] == c )
            result++;
    }
    return result;
}

void c_generate_protos( FILE * out ) {
    for( struct itab_iter * iter = itab_foreach( method_names );
         iter; iter = itab_next( iter ) ) {
        char *name = itab_value( iter );
        const char *key = itab_key( iter );
        int len = count_char( key, ':' );
        fprintf( out, "void %s(int, struct Object*", name );
        for( int i = 0; i < len; i++ )
            fprintf( out, ", struct Object* arg%d", i );
        fprintf( out, "); /* %s */\n", key );
    }
}

void string_register( const char *str ) {
    struct stringinfo *si = itab_read( strings, str );
    if( !si ) {
        si = talloc_zero( strings, struct stringinfo );
        si->num = ++string_count;
        itab_append( strings, str, si );
    }
    assert( si );
}
void c_generate_strings( FILE * out ) {
    for( struct itab_iter * iter = itab_foreach( strings );
         iter; iter = itab_next( iter ) ) {
        struct stringinfo *info = itab_value( iter );
        const char *str = itab_key( iter );

        fprintf( out,
                 "struct String v_string%d = {.tag = %d, .data = \"%s\"};\n",
                 info->num, string_class_num, str );
    }
}

/**
generate all blocks that are defined with method implementations.

these blocks are later on called by the dispatcher.
*/
void c_generate_blocks( FILE * out ) {
    struct context *root = context_new( NULL );
    for( struct itab_iter * iter = itab_foreach( methods );
         iter; iter = itab_next( iter ) ) {
        struct ast *m = itab_value( iter );
        assert( m->tag == AST_METHOD );
        fprintf( out, "void %s(struct Object* obj", itab_key( iter ) );
        if( m->u.methods.args ) {
            for( struct ast * arg = m->u.methods.args;
                 arg; arg = arg->u.argdef.next )
                fprintf( out, ", struct Object* %s", arg->u.argdef.name );
        }
        fprintf( out, "){\n" );
        fprintf( out, "struct %s * self = (struct %s*)obj;",
                 m->u.methods.classname, m->u.methods.classname );
        if( m->u.methods.src ) {
            fprintf( out, "   %s\n", m->u.methods.src );
        }
        struct context *meta_context = context_new( root );
        meta_context->ctx_class = true;
        char *ctx_name =
                talloc_strdup( meta_context, m->u.methods.classname );
        ctx_name = talloc_strdup_append( ctx_name, "$Meta" );
        meta_context->name = ctx_name;
        struct context *class_context = context_new( meta_context );
        class_context->ctx_class = true;
        class_context->name = m->u.methods.classname;
        if( m->u.methods.body )
            ast_to_c( out, m->u.methods.body, class_context );
        fprintf( out, "}\n" );
    }
}

void c_generate_dispatchers( FILE * out ) {
    for( struct itab_iter * iter = itab_foreach( method_names );
         iter; iter = itab_next( iter ) ) {
        const char *current_method = itab_value( iter );
        const char *orig_method = itab_key( iter );
        int len = count_char( orig_method, ':' );
// start function
        fprintf( out, "void %s(int tag, struct Object* self",
                 current_method );
        for( int i = 0; i < len; i++ )
            fprintf( out, ", struct Object* arg%d", i );
        fprintf( out, "){\n"
                 "   if(tag == -1) tag = self->tag;\n" "   switch(tag) {\n" );
// loop over all classes
        for( struct itab_iter * cls_iter = itab_foreach( classes );
             cls_iter; cls_iter = itab_next( cls_iter ) ) {
            struct ast *cls = itab_value( cls_iter );
            char nm[1000];
            sprintf( nm, "%s$%s", cls->u.cls.name, current_method );
            struct ast *impl = itab_read( methods, nm );
            fprintf( out, "      case %d:\n", cls->u.cls.num );
            if( impl ) {
                fprintf( out, "        %s(self", nm );
                for( int i = 0; i < len; i++ )
                    fprintf( out, ", arg%d", i );
                fprintf( out, ");\n" );
            }
            else {
                if( cls->u.cls.super ) {
                    struct ast *super =
                            itab_read( classes, cls->u.cls.super );
                    if( !super )
                        fprintf( stderr, "class %s not defined.\n",
                                 cls->u.cls.super );
                    assert( super );
                    fprintf( out, "        %s(%d, self);\n", current_method,
                             super->u.cls.num );
                }
                else {
                    fprintf( out,
                             "        error( 1, 1, \"Method unkown\" );\n" );

                }
            }
            fprintf( out, "        break;\n" );
        }
        fprintf( out, "  }\n" "}\n" );
    }
}


/**
@defgroup internal_structures Internal Representation
@{
*/
void method_def(t_pattern pattern, void* locals, void* directive, void* statements){
}

void method_def_verb(t_pattern pattern, void* coding){
}

/**
|}
*/

/**
@defgroup c_code_generator C Code Generator
@{
*/

/**
@brief generate the C-Code into the file stream.
@param[in] FILE* out
*/
void c_generate( FILE * out ) {

    itab_dump( variables );
    c_generate_structs( out );
    c_generate_protos( out );
    c_generate_strings( out );
    c_generate_blocks( out );
    c_generate_dispatchers( out );

}

/** @} */
