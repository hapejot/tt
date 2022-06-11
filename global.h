#include <stdbool.h>
bool nextToken(  );

#define AST_STRING          1
#define AST_IDENT           2
#define AST_UNARY_CALL      3
#define AST_STMT            4
#define AST_ASSIGN          5
#define AST_UNARY           6
#define AST_NAMES           7
#define AST_CLASS           8
#define AST_METHOD          9
#define AST_META            10
#define AST_ARG             11
#define AST_ARGDEF          12

struct ast {
    int tag;                    //!< descriminator for the union, tags start with AST_
    union {
        struct {
            char *v;            //!< string value owned by the syntax tree
        } str;                  //!< string node
        struct {
            char *v;            //!< id value owned by the syntax tree
        } id;                   //!< id node
        struct {
            struct ast *o;      //!< method target
            char *sel;          //!< selector
            struct ast *arg;    //!< list of arguments
        } unary;                //!< unary method call node
        struct {
            struct ast *v;      //!< argument value node
            struct ast *next;   //!< next argument
        } arg;                  //! argument list.
        struct argdef {
            const char *key;    //!< Keyword including the colon at the end
                                //!< if it is no keyword then the plain unary or binary name is here.
            const char *name;   //!< parameter name
            struct ast *next;   //!< next keyword in the list
        } argdef;
        struct {
            struct ast *v;
            struct ast *next;
        } stmt;
        struct {
            char *var;
            struct ast *expr;
        } asgn;
        struct {
            char *name;
            char *super;
            int num;
            struct ast *vars;
            struct ast *next;
        } cls;
        struct {
            char *v;
            struct ast *next;
        } names;
        struct {
            const char *name;
            struct ast *args;
            char *classname;
            char *src;
            struct ast *body;
            struct ast *next;
        } methods;
    } u;
};

#define meth_name   u.methods.name
#define meth_class  u.methods.classname
#define meth_body   u.methods.body
#define meth_next   u.methods.next

struct gd {
    int state;                  // 0 - init, 1 - running, 2 - end
    int paridx;
    int token;
    int pos;
    char buf[50];
    char *line;
    int line_count;
    struct ast *ast;
    int classnum;
    struct itab *src;
    struct itab_iter *src_iter;
};

extern struct gd gd;

extern void parse();
