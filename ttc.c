#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "talloc.h"

#include "global.h"
#include "lib.h"

extern struct gd gd;

int main(int argc, char** argv){
    assert(argc == 2);
    src_read(argv[1]);
    exit(-1);
    parse();
    // ParseCoverage(stdout);
    return 0;
}
