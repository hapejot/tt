#include "internal.h"
extern struct itab * classes;
extern struct itab * methods;
extern struct itab * method_names;
extern struct itab * variables;
extern struct itab * strings;
extern int string_class_num;

void require_classes( ) {
    if( !classes ) {
        classes = itab_new( );
        methods = itab_new( );
        method_names = itab_new( );
        variables = itab_new( );
        strings = itab_new( );

        gd.classnum = 1;

        class_enter( "Behavior" );
        class_enter( "Object" );

        string_class_num = gd.classnum - 1;


        gd.classnum = 100;
    }
}
