#define STACK_PTR_ARRAY(name, type, ...) .name = (type*)&(type[]) {__VA_ARGS__},\
.name ## _len = sizeof((type[]) {__VA_ARGS__}) / sizeof(type)

#define CMD(...) {__VA_ARGS__}
#define ARG(...) {__VA_ARGS__}
#define CMD_ARGS(...) STACK_PTR_ARRAY(args, USCommandArg, __VA_ARGS__)
#define SUB_CMDS(...) STACK_PTR_ARRAY(subcmds, USCommand, __VA_ARGS__)

#define CMD_IMPL(name) USCommandReturn cmd_ ## name(UselessShell* _tm, USCommandArgValue* args)
#define AC_IMPL(name) void ac_ ## name(USTabCompleteQuery* tc)