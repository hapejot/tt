#include "internal.h"
char *method_name( const char *class, const char *sel ) {
    char *result = talloc_strdup( NULL , class );
    result = talloc_strdup_append( result, "/" );
    result = talloc_strdup_append( result, sel );
    return result;
}
