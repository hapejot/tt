#include <assert.h>
#include "talloc.h"

struct test{
    int num;
    char * str;
};

void main(){
    struct test * t = talloc(NULL, struct test);
    assert(t);
    t->str = talloc_strdup(t, "Peter");
}
