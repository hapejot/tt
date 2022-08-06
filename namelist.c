#include "internal.h"
/**
* @defgroup namelist Name List
* list of names are very common. Basically they are a counter for the list length
* and an array of const char pointers. These functions help to build up these structures
* the pointer array and the counter are held in a structure that is not allocated here but
* is a reglar part of another structure.
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
void namelist_add( t_namelist * nl, const t_name name ) {
    nl->count++;
    nl->names = talloc_realloc( NULL, nl->names, t_name, nl->count );
    nl->names[nl->count - 1] = talloc_strdup( nl->names, name );
}

/** make a deep copy of a name list
* @param to the target name list, which doesn't need to be initialized
* @param from the source to be copied. */
void namelist_copy( t_namelist * to, t_namelist * from ) {
    to->count = from->count;
    to->names = talloc_array( NULL, t_name, to->count );
    assert(talloc_get_type(from->names, t_name));
    for( int i = 0; i < to->count; i++ ) {
        to->names[i] = talloc_strdup( to->names, from->names[i] );
    }
    assert(talloc_get_type(to->names, t_name));
}

/** @} */
