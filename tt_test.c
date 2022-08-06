#include "internal.h"

extern struct itab *methods;
extern struct itab *strings;
extern struct itab *variables;
extern struct itab *method_names;
extern struct itab *classes;


extern struct gd gd;
struct s_globals  global;


t_object *object_new( t_message_handler hdl ) {
    t_object *result = talloc_zero( NULL, t_object );
    result->handler = hdl;
    return result;
}
t_object *object_send( t_object * o, const char *sel, t_object ** args ) {
    return o->handler( o, sel, args );
}
void object_send_void( t_object * o, const char *sel, t_object ** args ) {
    t_object *x = object_send( o, sel, args );
    if( x != o ) {
// fprintf( stderr, "send void %s leaves these parents:\n", sel );
// talloc_show_parents( x, stderr );
    }
    talloc_unlink( NULL, x );
}

char *string_chars( t_object * str ) {
    object_send_void( str, "dump", NULL );
    tt_assert( str->handler == string_handler );
    return ( char * )str->u.data;
}

bool cstr_equals( const char *a, const char *b ) {
    return ( 0 == strcmp( a, b ) );
}


t_object *true_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "ifTrue:", sel ) ) {
        result = object_send( args[0], "value", NULL );
    }
    else if( cstr_equals( "ifFalse:", sel ) ) {
// all set
    }
    else if( 0 == strcmp( "ifTrue:ifFalse:", sel ) ) {
        result = object_send( args[0], "value", NULL );
    }
    else if( 0 == strcmp( "ifFalse:ifTrue:", sel ) ) {
        result = object_send( args[1], "value", NULL );
    }
    else
        result = method_exec( self, "True", sel, args );
    return result;
}

t_object *false_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "ifFalse:", sel ) ) {
        result = object_send( args[0], "value", NULL );
    }
    else if( 0 == strcmp( "ifTrue:ifFalse:", sel ) ) {
        result = object_send( args[1], "value", NULL );
    }
    else if( 0 == strcmp( "ifFalse:ifTrue:", sel ) ) {
        result = object_send( args[0], "value", NULL );
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
        int idx = self->u.vals.i[0];
        int max = self->u.vals.i[1];
        char *chars = ( char * )self->u.vals.p[0];
        char c = args[0]->u.intval;
        tt_assert(chars);
        if( max <= idx ) {
            max *= 2;
            chars = talloc_realloc( self, chars, char, max + 1 );
            tt_assert(chars);
            self->u.vals.i[1] = max;
            self->u.vals.p[0] = chars;
        }
        chars[idx] = args[0]->u.intval;
        self->u.vals.i[0] = idx + 1;
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


t_object *method_exec( t_object * self, const char *clsname, const char *sel,
                       t_object ** args ) {
    t_object *result = self;
    t_methoddef *m = method_read( clsname, sel );
    if( m ) {
        t_env *env = m->env;
        tt_assert( env );
        msg_add( "selector %s is defined on %s (env:%p, self:%p, handler:%p)",
                 sel, clsname, env, self, self->handler );
        for( int i = 0; i < m->args.count; i++ ) {
            env_set_local( env, m->args.names[i], args[i] );
        }
        env_set_local( env, "self", self );
        // env_dump( env, "after self" );
        result = simulate( env, m->statements );
        msg_add( "done simulation of method %s", sel );
    }
    else {
        msg_add( "%s %s not found.", clsname, sel );
        // msg_print_last(  );
        abort(  );
    }
    return result;
}

t_object *block_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    t_block *b = self->u.data;
    t_env *env = b->env;
    msg_add( "block handler" );
    if( cstr_equals( "dump", sel ) ) {
        msg_add( "dumping block.." );
    }
    else if( 0 == strcmp( "value", sel ) ) {

        result = simulate( env, b->statements );
    }
    else if( 0 == strcmp( "value:", sel ) ) {
        assert( args );
        assert( args[0] );
        assert( args[0]->handler );
        for( int i = 0; i < b->params.count; i++ ) {
            tt_assert( env != NULL );
            env_set_local( env, b->params.names[i], args[i] );
        }
        object_send_void( args[0], MSG_DUMP, NULL );
        result = simulate( env, b->statements );
    }
    else if( 0 == strcmp( "whileFalse:", sel ) ) {
        for( ;; ) {
            t_object *r = simulate( env, b->statements );
            object_send_void( r, "ifFalse:", args );
            if( r->handler == true_handler )
                break;
        }
    }
    else
        result = method_exec( self, "Block", sel, args );
    return result;
}

t_object *eval_messages( t_env * env, t_expression * expr ) {
    t_object *result = NULL;
    msg_add( "eval message env:%p", env );
// t <- eval target
    // env_dump( env, "DEBUG" );
    t_object *target = result = eval( env, expr->u.msg.target );

    tt_assert( target );
// foreach msg in cascade:
//      send msg to t
    assert( expr );
    int n = 1;
    for( t_messages * m = expr->u.msg.m; m != NULL; m = m->next ) {
        msg_add( "%d ... %p %s %s", n++, target, m->sel,
                 m->cascaded ? "cascaded" : "nested" );
        if( m->argc ) {
            t_object **args = talloc_zero_array( NULL, t_object *,
                                                 m->argc );

            for( int i = 0; i < m->argc; i++ ) {
                args[i] = eval( env, m->args[i] );
                assert( args[i] );
                msg_add( "..... arg %d evaluated.", i + 1 );
            }
            result = object_send( target, m->sel, args );
            talloc_free( args );
        }
        else
            result = object_send( target, m->sel, NULL );
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
            expr->u.block.env = env_new( env );
            result->u.data = &expr->u.block;
            for( int i = 0; i < expr->u.block.params.count; i++ ) {
                env_add( expr->u.block.env, expr->u.block.params.names[i] );
            }
            for( int i = 0; i < expr->u.block.locals.count; i++ ) {
                env_add( expr->u.block.env, expr->u.block.locals.names[i] );
            }
            break;

        case tag_ident:
            result = NULL;
            t_slot *slot = env_get_all( env, expr->u.ident, NULL );
            result = slot->val;
            if( result )
                msg_add( "eval ident %s -> %p(%p)", expr->u.ident, result,
                         result->handler );
            else {
                // env_dump( env, "IDENT no RESULT" );
                tt_assert( result );
            }
            break;

        case tag_assignment:
            msg_add( "eval assignment" );
            result = eval( env, expr->u.assignment.value );
            msg_add( "assign result[%p] to %s", result, expr->u.assignment.target );
            tt_assert( env );
            env_set( env, ( t_name ) expr->u.assignment.target, result );
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
            // msg_print_last(  );
            abort(  );
            break;
    }
    return result;
}

t_object *simulate( t_env * env, t_statements * stmts ) {
    t_object *result = NULL;
    assert( stmts );

    msg_add( "simulate" );

    while( stmts ) {
        switch ( stmts->type ) {
            case stmt_message:
                msg_add( "message stmt" );
                result = eval( env, stmts->expr );
                break;
            case stmt_return:
                msg_add( "return stmt" );
                result = eval( env, stmts->expr );
                msg_add( "returning value and leaving method...\n" );
                //msg_print_last(  );
                stmts = NULL;
                break;

            default:
                msg_add( "error: unkonwn stmt type: %d\n", stmts->type );
                // msg_print_last(  );
                abort(  );
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
    if( result == NULL )
        msg_add( "method %s not found in class %s", selector, class );
    tt_assert( result );
    talloc_free( nm );
    return result;
}

void test1(  ) {
    src_clear(  );
    src_add( "OrderedCollection class [" );
    src_add( "    main0: args [ Transcript show: ('- {1} -- {2} -' format: {10. 'Peter ist doof!'} ) ]" );
    src_add( "    main: args [ " );
    src_add( "        1 to: 9 do: [ :i | " );
    src_add( "            1 to: i do: [ :j | " );
    src_add( "                Transcript " );
    src_add( "                    show: ('{1} * {2} = {3}' format: {j. i. j * i}); " );
    src_add( "                    show: ' ' " );
    src_add( "            ]. " );
    src_add( "            Transcript show: '-'; cr. " );
    src_add( "        ]" );
    src_add( "    ]" );
    src_add( "]" );
    src_add( "String [" );
    src_add( "format: collection [" );
    src_add( "	\"Format the receiver by interpolating elements from collection, as in the following examples: " );
    src_add( "	('Five is {1}.' format: { 1 + 4}) >>> 'Five is 5.'" );
    src_add( "	('Five is {five}.' format: (Dictionary with: #five -> 5)) >>>  'Five is 5.'" );
    src_add( "	('In {1} you can escape \\{ by prefixing it with \\\\' format: {'strings'}) >>> 'In strings you can escape { by prefixing it with \\'" );
    src_add( "	('In \\{1\\} you can escape \\{ by prefixing it with \\\\' format: {'strings'}) >>> 'In {1} you can escape { by prefixing it with \\' \"" );
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

    t_env *e = env_new( NULL );
    gd.env = e;

    parse(  );
    fflush( stdout );



    env_add( e, "Transcript" );
    env_set_local( e, "Transcript", object_new( transcript_handler ) );

    global.String = object_new( string_meta_handler );
    global.True = object_new( true_handler );
    global.False = object_new( false_handler );
    global.Integer = object_new( integer_meta_handler );

    env_add( e, "String" );
    env_add( e, "True" );
    env_add( e, "False" );
    env_add( e, "Integer" );

    env_set_local( e, "String", global.String );
    env_set_local( e, "True", global.True );
    env_set_local( e, "False", global.False );
    env_set_local( e, "Integer", global.Integer );


    t_methoddef *m = method_read( "OrderedCollection", "main:" );
    assert( m );
    assert( m->args.count == 1 );
    assert( talloc_get_type( m->args.names, t_name ) );
    assert( 0 == strcmp( m->args.names[0], "args" ) );
    assert( m->statements != NULL );
    simulate( e, m->statements );
    // msg_print_last(  );
    talloc_free( e );
    talloc_free( classes );

// talloc_report_depth_file( methods, 0, 99, stderr );

    talloc_free( methods );
    talloc_free( method_names );
    talloc_free( variables );
    talloc_free( strings );
    src_clear(  );
}

void tt_abort( const char *reason ) {
    fprintf( stderr, "talloc abort: %s\n", reason );
// exit( -1 );
    msg_print_last(  );
    abort(  );
}

int main( int argc, char **argv ) {
    talloc_set_abort_fn( tt_abort );
    talloc_set_log_stderr(  );
    // talloc_enable_leak_report(  );
    test1(  );
    return 0;
}
