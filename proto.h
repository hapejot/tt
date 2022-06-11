/* class_enter.c */
void class_enter(const char *name);
/* lib.c */
void namelist_init(t_namelist *nl);
void namelist_add(t_namelist *nl, const char *name);
void namelist_copy(t_namelist *to, t_namelist *from);
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
_Bool is_ident_char(int c);
_Bool is_binary_char(int c);
_Bool src_clear(void);
_Bool src_add(const char *line);
_Bool src_read(const char *name);
_Bool src_dump(void);
_Bool readLine(void);
_Bool readChar(char *t);
_Bool readStringToken(void);
void parse_verbatim(char c);
_Bool nextToken(void);
/* method_name.c */
char *method_name(const char *class, const char *sel);
/* require_classes.c */
void require_classes(void);
/* require_current_class.c */
void require_current_class(void);
/* method_enter.c */
void method_enter(t_message_pattern *mp);
/* method_stmts.c */
void method_stmts(t_statements *stmts);
