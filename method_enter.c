#include "internal.h"
extern struct itab *methods;
extern t_classdef *current_class;
extern t_methoddef *current_method;

/** set this method to be the current one.
* all further fiddeling with this current method
* goes into this one then.
* this state is usefull to load complexity off from the parser
*/
void method_enter( t_message_pattern * mp ) {
    require_current_class(  );
//  make selector from parts by concatenating them 
    char *sel = talloc_strdup( NULL, mp->parts.names[0] );
    for( int i = 1; i < mp->parts.count; i++ ) {
        sel = talloc_strdup_append( sel, mp->parts.names[i] );
    }
//  create a consistent method name from class name and selector
    char *nm = method_name( current_class->name, sel );
//  lookup this method name in our table 
    t_methoddef *odef = itab_read( methods, nm );
    if( odef == NULL ) {
//  create a new method entry 
        odef = talloc_zero( methods, t_methoddef );
        odef->sel = talloc_strdup( odef, sel );
//      prepare an environment TODO: how is this environment handled for multiple entries?
        odef->env = env_new( current_class->env );
        env_add( odef->env, "self" );
        namelist_copy( &odef->args, &mp->names );
        for( int i = 0; i < odef->args.count; i++ ) {
            env_add( odef->env, odef->args.names[i] );
        }

        talloc_steal( odef, odef->args.names );
        assert( talloc_get_type( odef->args.names, t_name ) );
        itab_append( methods, nm, odef );
    }

/*  else we should think of some reaction
 *  option a) replace the definition
 *  option b) throw an error
 */
    current_method = odef;
    talloc_steal( odef, mp );
    talloc_free( nm );
    talloc_free( sel );
}
