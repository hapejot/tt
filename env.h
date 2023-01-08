/**
 * @file env.h
 * @author Peter Jaeckel (jaeckel@acm.org)
 * @brief  Environment Header 
 * @version 0.1
 * @date 2022-08-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */


/** @defgroup env Environment 
@{ 
*/
/** adding a name defintion the an environment */
void env_add( t_env * env, const t_name name );
/** clearing the values from the environment */
void env_clear( t_env * env );
/** setting a value to a name defintion in the environment */
void env_set( t_env * env, const t_name name, t_object * value );
/** getting the slot defining the current name value pair fitting the name */
t_slot *env_get( t_env * env, const t_name name );
/** output of the environment to stderr */
void env_dump(t_env*env, const char*);
/** sets only the local definition if any. 
* does not walk up the env chain. *
*/
void env_set_local( t_env * env, const t_name name, t_object * value );
/** new environment, inheriting definitions from another environment  */
t_env *env_new( t_env * parent );
/**
    searching the name in alle environments starting at the top walking down.
    @param name is the name of the defintion to be searched
    @param env refers to the top level environment
    @param env_found if not null receives the reference to the environment where the name has been found.
*/
t_slot *env_get_all( t_env * env, const t_name name, t_env ** env_found );

/** @} */
