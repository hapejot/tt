#include "global.h"
#include "tt.h"
#include <assert.h>
#include <talloc.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "lib.h"

/* class_enter.c */
void class_enter(const char *name);
/* lib.c */
int itab_lines(struct itab *itab);
struct itab *itab_new(void);
int itab_entry_cmp(const void *aptr, const void *bptr);
void itab_append(struct itab *itab, const char *key, void *value);
void *itab_read(struct itab *itab, const char *key);
void itab_dump(struct itab *itab);
struct itab_iter *itab_foreach(struct itab *tab);
struct itab_iter *itab_next(struct itab_iter *iter);
void *itab_value(struct itab_iter *iter);
const char *itab_key(struct itab_iter *iter);
bool is_ident_char(int c);
bool is_binary_char(int c);
bool src_clear(void);
bool src_add(const char *line);
bool src_read(const char *name);
bool src_dump(void);
bool readLine(void);
bool readChar(char *t);
bool readStringToken(void);
void parse_verbatim(char c);
bool nextToken(void);
/* method_name.c */
char *method_name(const char *class, const char *sel);
/* method_stmts.c */
/* require_classes.c */
void require_classes(void);
/* require_current_class.c */
void require_current_class(void);
/* method_enter.c */
void method_enter(t_message_pattern *mp);
