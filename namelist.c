#include "internal.h"
/**
* @defgroup namelist Name List
* @{
*/

/** @brief clear the structure for further usage.
* 
* The *namelist* itself is not allocated but could be part
* of an already allocated structure.
* @param nl reference to an existing structure to be initialized.
*/
void namelist_init( t_namelist * nl ) {
    nl->count = 0;
    nl->names = NULL;
}

/** adding a name to the name list.
* Memory will be allocated by the name list and also the name 
* will be copied. The paramter can safely being freed after this call.
* @param nl the modified list
* @param name the string to be added */
void namelist_add( t_namelist * nl, const char *name ) {
    nl->count++;
    nl->names = talloc_realloc( NULL, nl->names, char *, nl->count );
    nl->names[nl->count - 1] = talloc_strdup( nl->names, name );
}

/** make a deep copy of a name list
* @param to the target name list, which doesn't need to be initialized
* @param from the source to be copied. */
void namelist_copy( t_namelist * to, t_namelist * from ) {
    to->count = from->count;
    to->names = talloc_array( NULL, char *, to->count );
    assert(talloc_get_type(from->names, char *));
    for( int i = 0; i < to->count; i++ ) {
        to->names[i] = talloc_strdup( to->names, from->names[i] );
    }
    assert(talloc_get_type(to->names, char *));
}

/** @} */
