#include "internal.h"
extern t_classdef *current_class;
void require_current_class(  ) {
    assert( current_class != NULL );
}
