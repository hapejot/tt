#include "internal.h"
extern struct itab *methods;
extern t_classdef *current_class;
extern t_methoddef *current_method;
void method_enter( t_message_pattern * mp ) {
    require_current_class(  );
    char *sel = talloc_strdup( NULL, mp->parts.names[0] );
    for( int i = 1; i < mp->parts.count; i++ ) {
        sel = talloc_strdup_append( sel, mp->parts.names[i] );
    }
    printf( "+++ enter method %s\n", sel );

    char *nm = method_name( current_class->name, sel );
    t_methoddef *odef = itab_read( methods, nm );
    if( odef == NULL ) {
        odef = talloc_zero( methods, t_methoddef );
        odef->sel = talloc_strdup( odef, sel );
        namelist_copy( &odef->args, &mp->names );
        talloc_steal( odef, odef->args.names );
        assert( talloc_get_type( odef->args.names, char * ) );
        itab_append( methods, nm, odef );
    }
    current_method = odef;
    talloc_free( nm );
    talloc_free( sel );
}
