#include "global.h"
#include "tt.h"
#include <assert.h>
#include <talloc.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "lib.h"

#define METACLASS "MetaClass"
#define STRING "String"

/** Assignment */
typedef struct _Assign *Assign;

/** Assignments */
typedef struct _Assigns *Assigns;

/** all classes */
struct itab *classes = NULL;

/** current class
defined with *class_enter*. the next call of *class_enter* will
set a new class.
*/
t_classdef *current_class;
t_methoddef *current_method;

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

/** returns the number of lines in the table 
*/
int itab_lines( struct itab *itab ) {
    assert( itab );
    return itab->used;
}

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
    struct itab *r = talloc_zero( NULL, struct itab );
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

bool src_clear(  ) {
    if( gd.src ) {
        talloc_free( gd.src );
    }
    gd.src = itab_new(  );
    if( gd.src_iter ) {
        talloc_free( gd.src_iter );
    }
    gd.src_iter = NULL;
}

bool src_add( const char *line ) {
    int n = itab_lines( gd.src );
    char buf[10];
    sprintf( buf, "%09d", n + 1 );
    itab_append( gd.src, buf, talloc_strdup( gd.src, line ) );
}

/**
read file into itab.
*/
bool src_read( const char *name ) {
    FILE *f = fopen( name, "r" );
    char buf[1000];
    char *line;
    int line_no = 1;
    src_clear(  );
    for( ;; ) {
        line = fgets( buf, sizeof( buf ), f );
        if( line == NULL )
            break;
        int n = strlen( line );
        while( n > 0 && isspace( line[--n] ) )
            line[n] = 0;
        char line_number[10];
        sprintf( line_number, "%09d", line_no );
        itab_append( gd.src, line_number, talloc_strdup( gd.src, line ) );
        line_no++;
    }
    fclose( f );
}

bool src_dump(  ) {
    for( struct itab_iter * x = itab_foreach( gd.src );
         x; x = itab_next( x ) ) {
        printf( "%s:%s\n", itab_key( x ), itab_value( x ) );
    }
}

/**
@brief read one line from stdin
stores the result into @c{gd.line}.

trailing blanks are removed.
*/
bool readLine(  ) {
    if( gd.src_iter == NULL ) {
        gd.src_iter = itab_foreach( gd.src );
    }
    else {
        gd.src_iter = itab_next( gd.src_iter );
    }
    if( gd.src_iter ) {
        gd.line = itab_value( gd.src_iter );
        gd.line_count++;
        printf( "%2d:%s\n", gd.line_count, gd.line );
        gd.pos = 0;
        gd.state = 1;
        return true;
    }
    else {
        gd.line = "";
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
void parse_verbatim( char c ) {
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
            if( strcmp( ":=", gd.buf ) == 0 ) {
                gd.token = TK_ASSIGN;
                result = true;
            }
            else if( strcmp( "<-", gd.buf ) == 0 ) {
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
            int idx = 0;
            while( isdigit( c ) ) {
                printf("### digit %c\n", c);
                gd.buf[idx++] = c;
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
                    gd.token = TK_LPAREN;
                    break;
                case ')':
                    result = true;
                    gd.token = TK_RPAREN;
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
                    result = true;
                    gd.token = TK_LBRACE;
                    break;
                case '}':
                    result = true;
                    gd.token = TK_RBRACE;
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
                    readChar( &c );
                    if( c == '=' ) {
                        gd.token = TK_ASSIGN;
                    }
                    else
                        gd.pos--;
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



/** @defgroup msg Messages 
@{
*/
typedef char t_msg[200];
#define MSG_LOG_LEN 200
static struct s_msgs {
    int size;
    int pos;
    t_msg msgs[MSG_LOG_LEN];
} msgs;

void msg_init(  ) {
    if( msgs.size != MSG_LOG_LEN ) {
        msgs.size = MSG_LOG_LEN;
        msgs.pos = 0;
    }
}
void msg_add( const char *msg, ... ) {
    va_list ap;
    msg_init(  );
    va_start( ap, msg );

    vsnprintf( msgs.msgs[msgs.pos], 199, msg, ap );
    msgs.pos = ( msgs.pos + 1 ) % msgs.size;
    va_end( ap );
}

void msg_print_last(  ) {
    printf("-------------------------------------\n");
    const char*fmt = "%03d --- %s\n";
    int n = 1;
    for( int i = msgs.pos; i < msgs.size; i++ ) {
        if( msgs.msgs[i][0] )
            printf( fmt, n++, msgs.msgs[i] );
        msgs.msgs[i][0] = 0;
    }
    for( int i = 0; i < msgs.pos; i++ ) {
        if( msgs.msgs[i][0] )
            printf( fmt,n++, msgs.msgs[i] );
        msgs.msgs[i][0] = 0;
    }
}

/** @} */



/** @defgroup messages Syntax Messages 
@{
*/
void message_add_msg(t_messages *ms, t_messages *m){
    while(ms->next) ms = ms->next;
    ms->next = m;
}

/** |} */
