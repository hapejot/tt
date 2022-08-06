#include "internal.h"

t_object *integer_meta_handler( t_object * self, const char *sel,
                                t_object ** args ) {
    t_object *result = self;
    if( cstr_equals( "readFrom:ifFail:", sel ) ) {
        long num = strtol( string_cstr( args[0] ), NULL, 10 );
        if( errno )
            result = object_send( args[1], "value", NULL );
        else {
            result = object_new( int_handler );
            result->u.intval = num;
        }
    }
    else
        result = method_exec( self, "IntegerMeta", sel, args );
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
            object_send_void( args[1], "value:", par );
        }
    }
    else if( 0 == strcmp( sel, MSG_DUMP ) ) {
        msg_add( "int: %d", self->u.intval );
    }
    else if( cstr_equals( "asString", sel ) ) {
        result = object_new( string_handler );
        result->u.data = talloc_asprintf( result, "%d", self->u.intval );
    }
    else if( cstr_equals("*", sel)){
        tt_assert(args[0]->handler == int_handler);
        result = object_new( int_handler );
        result->u.intval = self->u.intval * args[0]->u.intval;
    }
    else
        result = method_exec( self, "Integer", sel, args );
    return result;
}

