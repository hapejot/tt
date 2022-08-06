#include "internal.h"

t_object *transcript_handler( t_object * self, const char *sel,
                              t_object ** args ) {
    t_object *result = self;
    if(cstr_equals("show:", sel)){
        printf("--->|%s|\n", string_cstr(args[0]));
    }
    return result;
}
