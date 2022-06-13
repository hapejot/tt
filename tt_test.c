#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "talloc.h"

#include "global.h"
#include "lib.h"

extern struct itab *methods;
extern struct itab *strings;
extern struct itab *variables;
extern struct itab *method_names;
extern struct itab *classes;

#define MSG_DUMP "dump"

#define tt_assert(x)                                                 \
    if (!(x))                                                        \
    {                                                                \
        printf("assert failed: %s %s %d\n", #x, __FILE__, __LINE__); \
        msg_print_last();                                            \
        exit(-1);                                                    \
    }

extern struct gd gd;
///////////////   Prototypes   //////////////
extern char *method_name( const char *, const char * );

t_object *simulate( t_env * env, t_statements * stmts );
t_object *eval( t_env * env, t_expression * expr );
t_env *env_add( t_env * env, const char *name, t_object * val );
t_env *env_new( t_env * parent );
t_object *string_handler( t_object * self, const char *sel,
                          t_object ** args );
t_object *int_handler( t_object * self, const char *sel, t_object ** args );
t_object *method_exec( t_object * self, const char *clsname, const char *sel,
                       t_object ** args );
/////////////////////////////////////////////
struct s_globals {
    t_object *String;
    t_object *Integer;
    t_object *True;
    t_object *False;
} global;


t_object *object_new( t_message_handler hdl ) {
    t_object *result = talloc_zero( NULL, t_object );
    result->handler = hdl;
}

char *string_chars( t_object * str ) {
    str->handler( str, "dump", NULL );
    tt_assert( str->handler == string_handler );
    return ( char * )str->u.data;
}

bool cstr_equals( const char *a, const char *b ) {
    return ( 0 == strcmp( a, b ) );
}

t_object *integer_meta_handler( t_object * self, const char *sel,
                                t_object ** args ) {
    t_object *result = self;
    if( cstr_equals( "readFrom:ifFail:", sel ) ) {
        long num = strtol( string_chars( args[0] ), NULL, 10 );
        if( errno )
            result = args[1]->handler( args[1], "value", NULL );
        else {
            result = object_new( int_handler );
            result->u.intval = num;
        }
    }
    else
        result = method_exec( self, "IntegerMeta", sel, args );
    return result;
}


t_object *true_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "ifTrue:", sel ) ) {
        result = args[0]->handler( args[0], "value", NULL );
    }
    else if( cstr_equals( "ifFalse:", sel ) ) {
// all set
    }
    else if( 0 == strcmp( "ifTrue:ifFalse:", sel ) ) {
        result = args[0]->handler( args[0], "value", NULL );
    }
    else if( 0 == strcmp( "ifFalse:ifTrue:", sel ) ) {
        result = args[1]->handler( args[1], "value", NULL );
    }
    else
        result = method_exec( self, "True", sel, args );
    return result;
}

t_object *false_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "ifFalse:", sel ) ) {
        result = args[0]->handler( args[0], "value", NULL );
    }
    else if( 0 == strcmp( "ifTrue:ifFalse:", sel ) ) {
        result = args[1]->handler( args[1], "value", NULL );
    }
    else if( 0 == strcmp( "ifFalse:ifTrue:", sel ) ) {
        result = args[0]->handler( args[0], "value", NULL );
    }
    else
        result = method_exec( self, "False", sel, args );
    return result;
}


t_object *char_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "==", sel ) ) {
        if( args[0]->handler == char_handler ) {
            if( self->u.intval == args[0]->u.intval )
                result = global.True;
            else
                result = global.False;
        }
        else
            result = global.False;
    }
    else if( cstr_equals( "dump", sel ) ) {
        msg_add( "char: %c", self->u.intval );
    }
    else
        result = method_exec( self, "Char", sel, args );
    return result;
}

t_object *stream_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( cstr_equals( "upTo:", sel ) ) {
        tt_assert( args[0]->handler = char_handler );
        char sep = args[0]->u.intval;
        int idx = self->u.vals.i[0];
        int start = idx;
        int max = self->u.vals.i[1];
        char *chars = ( char * )self->u.vals.p[0];
        while( idx < max ) {
            if( chars[idx] == sep )
                break;
            idx++;
        }
        result = object_new( string_handler );
        int len = idx - start;
        result->u.data = talloc_zero_array( result, char, len + 1 );
        memcpy( result->u.data, chars + start, len );
        self->u.vals.i[0] = idx + 1;
    }
    else if( 0 == strcmp( "atEnd", sel ) ) {
        if( self->u.vals.i[0] < self->u.vals.i[1] )
            result = global.False;
        else
            result = global.True;
    }
    else if( 0 == strcmp( "next", sel ) ) {
        result = object_new( char_handler );
        result->u.intval = ( ( char * )self->u.vals.p[0] )[self->u.vals.i[0]];
        self->u.vals.i[0]++;
    }
    else if( cstr_equals( "nextPut:", sel ) ) {
        tt_assert( args[0]->handler == char_handler );
        fprintf( stderr, "<%c>\n", args[0]->u.intval );
    }
    else if( cstr_equals( "dump", sel ) ) {
        msg_add( "Stream len:%d pos:%d", self->u.vals.i[1],
                 self->u.vals.i[0] );
    }
    else
        result = method_exec( self, "Stream", sel, args );
    return result;
}

t_object *array_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( cstr_equals( "at:", sel ) ) {
        tt_assert( args[0]->handler == int_handler );
        int idx = args[0]->u.intval;
        tt_assert( 1 <= idx && idx <= self->u.vars.cnt );
        result = self->u.vars.vs[idx - 1];
    }
    else
        result = method_exec( self, "Array", sel, args );
    return result;
}

t_object *transcript_handler( t_object * self, const char *sel,
                              t_object ** args ) {
    t_object *result = method_exec( self, "Array", sel, args );
    return result;
}
t_object *int_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    assert( sel );
    msg_add( "int handler: (%d) %s", self->u.intval, sel );
    if( 0 == strcmp( sel, "to:do:" ) ) {
        assert( args[0]->handler == int_handler );
        int start = self->u.intval;
        int finish = args[0]->u.intval;

        for( int i = start; i <= finish; i++ ) {
            t_object *par[1];
            par[0] = object_new( int_handler );
            par[0]->u.intval = i;

            args[1]->handler( args[1], "value:", par );
            talloc_free( par[0] );
        }
    }
    else if( 0 == strcmp( sel, MSG_DUMP ) ) {
        msg_add( "int: %d", self->u.intval );
    }
    else if( cstr_equals( "asString", sel ) ) {
        result = object_new( string_handler );
        result->u.data = talloc_asprintf( result, "%d", self->u.intval );
    }
    else
        result = method_exec( self, "Integer", sel, args );
    return result;
}

t_object *string_meta_handler( t_object * self, const char *sel,
                               t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "new:streamContents:", sel ) ) {
        int size = args[0]->u.intval;
        msg_add( "new string with size: %d", size );
        t_object *par[1];
        par[0] = object_new( stream_handler );
        par[0]->u.vals.i[0] = 0;
        par[0]->u.vals.i[1] = size;
        par[0]->u.vals.p[0] = talloc_array( par[0], char, size + 1 );

        args[1]->handler( args[1], "value:", par );
        msg_add( "move stream data to string data now." );
        talloc_free( par[0] );
        result = object_new( string_handler );
        result->u.data = talloc_strdup( result, "<string from stream>" );

    }
    else
        result = method_exec( self, "StringMeta", sel, args );

    return result;
}

t_object *method_exec( t_object * self, const char *clsname, const char *sel,
                       t_object ** args ) {
    t_object *result = self;
    t_methoddef *m = method_read( clsname, sel );
    if( m ) {
        t_env *env = env_new( self->env );
        tt_assert( env );
        env_add( env, "self", self );
        msg_add( "selector %s is defined on %s", sel, clsname );
        for( int i = 0; i < m->args.count; i++ ) {
            env_add( env, m->args.names[i], args[i] );
        }
        result = simulate( env, m->statements );
        msg_add( "done simulation of method %s", sel );
        talloc_free( env );
    }
    else {
        msg_add( "%s %s not found.", clsname, sel );
        msg_print_last(  );
        exit( -1 );
    }
    return result;
}

t_object *string_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    const char *self_data = ( const char * )self->u.data;
    if( 0 == strcmp( sel, MSG_DUMP ) ) {
        msg_add( "str: '%s'", self_data );
    }
    else if( cstr_equals( "asString", sel ) ) {
// all set...
    }
    else if( cstr_equals( "do:", sel ) ) {
        int n = strlen( self_data );
        for( int i = 0; i < n; i++ ) {
            t_object *r = object_new( char_handler );
            r->u.intval = self_data[i];
            args[0]->handler( args[0], "value:", &r );
            talloc_free( r );
        }
    }
    else if( 0 == strcmp( sel, "readStream" ) ) {
        t_object *result = object_new( stream_handler );
        result->u.vals.i[0] = 0;
        result->u.vals.i[1] = strlen( self->u.data );
        result->u.vals.p[0] = self->u.data;
        return result;
    }
    else if( 0 == strcmp( sel, "species" ) ) {
        return global.String;
    }
    else if( 0 == strcmp( sel, "size" ) ) {
        result = object_new( int_handler );
        result->u.intval = strlen( self->u.data );
    }
    else {
        result = method_exec( self, "String", sel, args );
    }
    return result;
}

t_env *env_new( t_env * parent ) {
    t_env *env = talloc_zero( parent, t_env );
    env->next = parent;
    return env;
}
t_env *env_add( t_env * env, const char *name, t_object * val ) {
    tt_assert( env != NULL );
    t_slot *n = talloc_zero( env, t_slot );
    n->next = env->slots;
    n->name = name;
    n->val = val;
    env->slots = n;

    talloc_reference( env, val );

    return env;
}

t_object *block_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    t_block *b = self->u.data;
    t_env *env = env_new( self->env );
    msg_add( "block handler" );
    if( 0 == strcmp( "value", sel ) ) {

        result = simulate( env, b->statements );
    }
    else if( 0 == strcmp( "value:", sel ) ) {
        assert( args );
        assert( args[0] );
        assert( args[0]->handler );
        for( int i = 0; i < b->params.count; i++ ) {
            tt_assert( env != NULL );
            env = env_add( env, b->params.names[i], args[i] );
        }
        args[0]->handler( args[0], MSG_DUMP, NULL );
        result = simulate( env, b->statements );
    }
    else if( 0 == strcmp( "whileFalse:", sel ) ) {
        for( ;; ) {
            t_object *r = simulate( env, b->statements );
            r->handler( r, "ifFalse:", args );
            if( r->handler == true_handler )
                break;
        }
    }
    else
        result = method_exec( self, "Block", sel, args );
    talloc_free( env );
    return result;
}

t_object *eval_messages( t_env * env, t_expression * expr ) {
    t_object *result = NULL;
    msg_add( "eval message" );
// t <- eval target
    t_object *target = result = eval( env, expr->u.msg.target );

    tt_assert( target );
// foreach msg in cascade:
//      send msg to t
    assert( expr );
    int n = 1;
    for( t_messages * m = expr->u.msg.m; m != NULL; m = m->next ) {
        msg_add( "%d ... %p %s %s", n++, target, m->sel,
                 m->cascaded ? "cascaded" : "nested" );
// target->handler(target, MSG_DUMP, NULL);
        if( m->argc ) {
            t_object **args = talloc_zero_array( NULL, t_object *,
                                                 m->argc );

            for( int i = 0; i < m->argc; i++ ) {
                args[i] = eval( env, m->args[i] );
                assert( args[i] );
// talloc_steal( args, args[i] );
                msg_add( "..... arg %d evaluated.", i + 1 );
// args[i]->handler( args[i], MSG_DUMP, NULL );
            }
            result = target->handler( target, m->sel, args );
            talloc_free( args );
        }
        else
            result = target->handler( target, m->sel, NULL );
        if( !m->cascaded )
            target = result;
    }
    return result;
}
t_object *eval( t_env * env, t_expression * expr ) {
    t_object *result = NULL;
    assert( expr );

    switch ( expr->tag ) {
        case tag_string:
            msg_add( "eval str %s", expr->u.strvalue );
            result = object_new( string_handler );
            result->u.data = talloc_strdup( result, expr->u.strvalue );
            result->env = env;
            break;

        case tag_number:
            result = object_new( int_handler );
            result->u.intval = expr->u.intvalue;
            msg_add( "eval number %d", result->u.intval );
            break;

        case tag_char:
            msg_add( "eval char" );
            result = object_new( char_handler );
            result->u.intval = expr->u.intvalue;
            break;

        case tag_message:
            result = eval_messages( env, expr );
            break;

        case tag_block:
            msg_add( "eval block" );
            result = object_new( block_handler );
            result->u.data = &expr->u.block;
            result->env = env;
            break;

        case tag_ident:
            msg_add( "eval ident %s", expr->u.ident );
            result = NULL;
            while( !result ) {
                for( t_slot * s = env->slots; s; s = s->next ) {
                    if( 0 == strcmp( expr->u.ident, s->name ) ) {
                        result = s->val;
                        break;
                    }
                }
                if( !result && env->next )
                    env = env->next;
            }
            if( !result ) {
                msg_add( "ident %s undefined", expr->u.ident );
            }
            break;

        case tag_assignment:
            msg_add( "eval assignment" );
            result = eval( env, expr->u.assignment.value );
            msg_add( "assign result to %s", expr->u.assignment.target );
            tt_assert( env );
            env = env_add( env, expr->u.assignment.target, result );
            break;

        case tag_array:
            {
                int n = expr->u.exprs.count;
                msg_add( "eval array %d elements", n );
                result = object_new( array_handler );
                result->u.vars.cnt = n;
                result->u.vars.vs = talloc_array( result, t_object *, n );
                for( int i = 0; i < expr->u.exprs.count; i++ ) {
                    result->u.vars.vs[i] = eval( env, expr->u.exprs.list[i] );
                }
            }
            break;

        default:
            msg_add( "error: unknown eval tag: %d", expr->tag );
            msg_print_last(  );
            exit( -1 );
            break;
    }
    return result;
}
t_object *simulate( t_env * env, t_statements * stmts ) {
    t_object *result = NULL;
    assert( stmts );

    msg_add( "simulate" );
    t_env *tenv = env;
    int depth = 0;
    while( tenv ) {
        for( t_slot * s = tenv->slots; s; s = s->next ) {
            msg_add( "..%d.. %s", depth, s->name );
        }
        tenv = tenv->next;
        depth++;
    }

    while( stmts ) {
        switch ( stmts->type ) {
            case stmt_message:
                msg_add( "message stmt" );
                if( result )
                    talloc_free( result );
                result = eval( env, stmts->expr );
                break;
            case stmt_return:
                msg_add( "return stmt" );
                result = eval( env, stmts->expr );
                msg_add( "returning value and leaving method...\n" );
                msg_print_last(  );
                stmts = NULL;
                break;

            default:
                msg_add( "error: unkonwn stmt type: %d\n", stmts->type );
                msg_print_last(  );
                exit( -1 );
                break;
        }
        if( stmts )
            stmts = stmts->next;
    }
    return result;
}


t_methoddef *method_read( const char *class, const char *selector ) {
    char *nm = method_name( class, selector );
    t_methoddef *result = itab_read( methods, nm );
    talloc_free( nm );
    return result;
}

void test1(  ) {
    src_clear(  );
    src_add( "OrderedCollection class [" );
    src_add( "    main: args [ '{1}/{2}' format: {10. 'Peter ist doof!'} ]" );
    src_add( "    main2: args [ " );
    src_add( "        1 to: 9 do: [ :i | " );
    src_add( "            1 to: i do: [ :j | " );
    src_add( "                Transcript " );
    src_add( "                    show: ('{1} * {2} = {3}' format: {j. i. j * i}); " );
    src_add( "                    show: ' ' " );
    src_add( "            ]. " );
    src_add( "            Transcript show: ' '; cr. " );
    src_add( "        ]" );
    src_add( "    ]" );
    src_add( "]" );
    src_add( "String [" );
    src_add( "format: collection [" );
    src_add( "	\"Format the receiver by interpolating elements from collection, as in the following examples:\" " );
    src_add( "	\"('Five is {1}.' format: { 1 + 4}) >>> 'Five is 5.'\"" );
    src_add( "	\"('Five is {five}.' format: (Dictionary with: #five -> 5)) >>>  'Five is 5.'\"" );
    src_add( "	\"('In {1} you can escape \\{ by prefixing it with \\\\' format: {'strings'}) >>> 'In strings you can escape { by prefixing it with \\' \"" );
    src_add( "	\"('In \\{1\\} you can escape \\{ by prefixing it with \\\\' format: {'strings'}) >>> 'In {1} you can escape { by prefixing it with \\' \"" );
    src_add( "" );
    src_add( "	^ self species" );
    src_add( "		new: self size" );
    src_add( "		streamContents: [ :result | " );
    src_add( "			| stream |" );
    src_add( "			stream := self readStream." );
    src_add( "			[ stream atEnd ]" );
    src_add( "				whileFalse: [ | currentChar |" );
    src_add( "					(currentChar := stream next) == ${" );
    src_add( "						ifTrue: [ | expression index |" );
    src_add( "							expression := stream upTo: $}." );
    src_add( "							index := Integer readFrom: expression ifFail: [ expression ]." );
    src_add( "							result nextPutAll: (collection at: index) asString ]" );
    src_add( "						ifFalse: [ " );
    src_add( "                            currentChar == $\\" );
    src_add( "								ifTrue: [ stream atEnd" );
    src_add( "										ifFalse: [ result nextPut: stream next ] ]" );
    src_add( "								ifFalse: [ result nextPut: currentChar ] ] ] ] ]" );
    src_add( "]" );
    src_add( "Stream [" );
    src_add( "  nextPutAll: aCollection [^ aCollection do: [:each | self nextPut: each]]" );
    src_add( "]" );
    src_add( "ByteArray class [" );
    src_add( "      newWithSize: n []" );
    src_add( "]" );
    src_dump(  );
    parse(  );
    fflush( stdout );


    t_env *e = env_new( NULL );
    env_add( e, "Transcript", object_new( transcript_handler ) );

    global.String = object_new( string_meta_handler );
    global.True = object_new( true_handler );
    global.False = object_new( false_handler );
    global.Integer = object_new( integer_meta_handler );
    env_add( e, "String", global.String );
    env_add( e, "True", global.True );
    env_add( e, "False", global.False );
    env_add( e, "Integer", global.Integer );

    t_methoddef *m = method_read( "OrderedCollection", "main:" );
    assert( m );
    assert( m->args.count == 1 );
    assert( talloc_get_type( m->args.names, char * ) );
    assert( 0 == strcmp( m->args.names[0], "args" ) );
    assert( m->statements != NULL );
    simulate( e, m->statements );
    msg_print_last(  );
    talloc_free( e );
    talloc_free( classes );

    talloc_report_depth_file( methods, 0, 99, stderr );

    talloc_free( methods );
    talloc_free( method_names );
    talloc_free( variables );
    talloc_free( strings );
    src_clear(  );
}

int main( int argc, char **argv ) {
    talloc_enable_leak_report(  );
    test1(  );
    return 0;
}
