#include "internal.h"
extern struct itab *classes;
extern t_classdef *current_class;
void class_enter( const char *name ) {
    require_classes( );
    t_classdef *odef = itab_read( classes, name );


    if( odef == NULL) {
        printf( "new class %s\n", name );
        odef = (t_classdef *)_talloc_zero(classes, sizeof(t_classdef), "t_classdef");
        odef->id = gd.classnum++;
        odef->name = talloc_strdup( odef, name );
        odef->env = env_new(gd.env);
        env_add(odef->env, "CLASSVAR");
        itab_append( classes, name, odef );
    }
    current_class = odef;
}
