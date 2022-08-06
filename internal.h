#include "global.h"
#include "tt.h"
#include <assert.h>
#include <talloc.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include "lib.h"
#include "env.h"

/** @defgroup internal Internal Functions
@{
*/
/** beginning a new class */
void class_enter( const char *name );

bool is_ident_char( int c );
/** binary chars are special ones for binary message names */
bool is_binary_char( int c );
/** clear the source table */
bool src_clear( void );
/** add a line to the source table */
bool src_add( const char *line );
/** read a line of the source table */
bool src_read( const char *name );
/** dump the source table */
bool src_dump( void );
/** read the next line from the source */
bool readLine( void );
/** read the next char from the source */
bool readChar( char *t );
/** read a string token from the source */
bool readStringToken( void );
/** parse the verbatim chars (not used)  */
void parse_verbatim( char c );
/** parse the next token */
bool nextToken( void );
/** construct the method name from combination of class and selector */
char *method_name( const char *class, const char *sel );
/** require definition of classes */
void require_classes( void );
/** require that there is a current class */
void require_current_class( void );
/** enter a method */
void method_enter( t_message_pattern * mp );

/** message selector for dumping an object */
#define MSG_DUMP "dump"

/** global definitions structure, references objects to be used somehwere in the code.
these objects or classes are predefined ones
*/
struct s_globals {
    t_object *String;                            ///< reference to the string meta object
    t_object *Integer;                           ///< reference to the integer meta object
    t_object *True;                              ///< reference to the True object
    t_object *False;                             ///< reference to the False object
};

/** global */
extern struct s_globals global;

/** compare to cstrings and return if they are equal */
bool cstr_equals( const char *, const char * );
/** new object */
t_object *object_new( t_message_handler hdl );
/** send a message to an object */
t_object *object_send( t_object * self, const char *sel, t_object ** args );
/** send a message to an object ignoring the result */
void object_send_void( t_object * self, const char *sel, t_object ** args );

char *method_name( const char *, const char * );

/** simulate the execution of a list of statemtents */
t_object *simulate( t_env * env, t_statements * stmts );
/** evaluate an expression */
t_object *eval( t_env * env, t_expression * expr );

/** handle string messages */
t_object *string_handler( t_object * self, const char *sel,
                          t_object ** args );
/** handle string meta messages */
t_object *string_meta_handler( t_object * self, const char *sel,
                               t_object ** args );
/** cstring */
const char* string_cstr(t_object* self);

/** handle integer meta messages */
t_object *integer_meta_handler( t_object * self, const char *sel, t_object ** args );
/** handle integer messges */
t_object *int_handler( t_object * self, const char *sel, t_object ** args );
/** handle character messages */
t_object *char_handler( t_object * self, const char *sel, t_object ** args );
/** handle block messages */
t_object *block_handler( t_object * self, const char *sel, t_object ** args );
/** handle stream messages */
t_object *stream_handler( t_object * self, const char *sel,
                          t_object ** args );
/** handle transcript messages */
t_object *transcript_handler( t_object * self, const char *sel,
                              t_object ** args );

/** execute a method */
t_object *method_exec( t_object * self, const char *clsname, const char *sel,
                       t_object ** args );

/** @} */


/** @addtogroup itab
@{ 
*/
/** how many lines? */
int itab_lines( struct itab *itab );
/** new */
struct itab *itab_new( void );
/** compare entries */
int itab_entry_cmp( const void *aptr, const void *bptr );
/** append new line */
void itab_append( struct itab *itab, const char *key, void *value );
/** find a line by key */
void *itab_read( struct itab *itab, const char *key );
/** dump content to output */
void itab_dump( struct itab *itab );
/** start iteration */
struct itab_iter *itab_foreach( struct itab *tab );
/** cycle through iterator */
struct itab_iter *itab_next( struct itab_iter *iter );
/** value of current iterator */
void *itab_value( struct itab_iter *iter );
/** key of current iterator */
const char *itab_key( struct itab_iter *iter );
/** @} */
