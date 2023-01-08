/**
 * @file env.c
 * @author your name (you@domain.com)
 * @brief Environment 
 * @version 0.1
 * @date 2022-08-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "internal.h"

void env_dump( t_env * env, const char *reason ) {
    bool empty = true;
    fprintf( stderr, "[[[ ENV %s\n", reason );
    for( t_env * e = env; e; e = e->next ) {
        empty = false;
        fprintf( stderr, "---- Frame [%p] ----\n", e );
        for( t_slot * s = e->slots; s; s = s->next ) {
            fprintf( stderr, "ENV: %s: %p\n", s->name, s->val );
        }
    }
    if( empty )
        fprintf( stderr, "---- Empty Frames ----\n" );
    fprintf( stderr, "ENV %s ]]]\n", reason );
}


t_env *env_new( t_env * parent ) {
    t_env *env = talloc_zero( parent, t_env );
    env->next = parent;
    return env;
}


t_slot *env_get( t_env * env, const t_name name ) {
    for( t_slot * n = env->slots; n; n = n->next ) {
        if( strcmp( name, n->name ) == 0 )
            return n;
    }
    return NULL;
}

t_slot *env_get_all( t_env * env, const t_name name, t_env ** env_found ) {
    for( t_env * e = env; e; e = e->next ) {
        t_slot *s = env_get( e, name );
        if( s ) {
            if( env_found )
                *env_found = e;
            return s;
        }
    }
    // env_dump( env, "GET_ALL" );
    abort(  );
}

void env_add( t_env * env, const t_name name ) {
    tt_assert( env != NULL );
    t_slot *n = env_get( env, name );
    if( !n ) {
        n = talloc_zero( env, t_slot );
        n->next = env->slots;
        n->name = name;
        n->val = NULL;
        env->slots = n;
    }
}

void env_set_local( t_env * env, const t_name name, t_object * val ) {
    t_slot *n = env_get( env, name );
    if( n ) {
        tt_assert( val );
        if( n->val && n->val != val ) {
            talloc_unlink( env, n->val );
            n->val = val;
        }
        n->val = val;
        talloc_reference( env, n->val );
    }
    else {
        env_dump( env, "ERROR" );
        abort(  );
    }
}

void env_set( t_env * env, const t_name name, t_object * val ) {
    t_env *env_found = NULL;
    t_slot *n = env_get_all( env, name, &env_found );
    if( n ) {
        n->val = val;
        talloc_reference( env_found, val );
    }
    else {
        env_dump( env, "ERROR" );
        abort(  );
    }
}
