// Behavior trees: decision planning and decision making.
// Supersedes finite state-machines (FSM) and hierarchical finite state-machines (HFSM).
// - rlyeh, public domain.

typedef int (*bt_func)();

typedef struct bt_t {
    uint64_t type;
    int (*action)();
    union {
        int argi;
        float argf;
    };
    array(struct bt_t) children;
} bt_t;

API bt_t    bt(const char *ini_file, unsigned flags);
API int     bt_run(bt_t *b);
API void    bt_addfun(const char *name, int(*func)());
API bt_func bt_findfun(const char *name);
API char   *bt_funcname(bt_func fn);

API int ui_bt(bt_t *b);
