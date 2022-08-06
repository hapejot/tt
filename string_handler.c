#include "internal.h"


/** returns the CString reference of a String object
* Errors: Object is no String object
* @param self the string object
*/
const char *string_cstr( t_object * self ) {
    assert( self->handler == string_handler );
    return self->u.data;
}


t_object *string_meta_handler( t_object * self, const char *sel,
                               t_object ** args ) {
    t_object *result = self;
    if( 0 == strcmp( "new:streamContents:", sel ) ) {
        int size = args[0]->u.intval;
        msg_add( "new string with size: %d", size );
        t_object *par[1];
        t_object *stream = object_new( stream_handler );

        stream = object_new( stream_handler );
        stream->u.vals.i[0] = 0;
        stream->u.vals.i[1] = size;
        stream->u.vals.p[0] = talloc_array( stream, char, size + 1 );
        talloc_reference( self, stream );
        par[0] = stream;
        msg_add( "stream for streamContent: %p", stream );
        t_object *o = object_send( args[1], "value:", par );
// talloc_unlink(NULL, par[0] );
        result = object_new( string_handler );
        result->u.data = talloc_strdup( result, stream->u.vals.p[0] );
    }
    else
        result = method_exec( self, "StringMeta", sel, args );

    return result;
}

t_object *string_handler( t_object * self, const char *sel, t_object ** args ) {
    t_object *result = self;
    const char *self_data = string_cstr( self );
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
            object_send_void( args[0], "value:", &r );
        }
    }
    else if( 0 == strcmp( sel, "readStream" ) ) {
        t_object *result = object_new( stream_handler );
        result->u.vals.i[0] = 0;
        result->u.vals.i[1] = strlen( self->u.data );
        result->u.vals.p[0] = self->u.data;
        msg_add( "stream for readStream: %p", result );
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
