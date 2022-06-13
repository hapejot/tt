#include "internal.h"
extern t_methoddef *current_method;
void method_stmts( t_statements * stmts ) {
    current_method->statements = stmts;
    talloc_steal(current_method, stmts);
}
