#ifndef COMMAND_H
#define COMMAND_H

// todo: remove command.cpp hpp. Put it in console. then consider moving console to game layer
typedef void Proc(String8* args);

typedef struct CommandInfo{
    String8 name;
    u32 min_args;
    u32 max_args;
    Proc* proc;
} CommandInfo;

#define COMMANDS_COUNT_MAX KB(1)
global CommandInfo commands[COMMANDS_COUNT_MAX];
global s32         commands_count = 0;

#define ARGS_COUNT_MAX 50
global String8 command_args[ARGS_COUNT_MAX];
global s32     command_args_count = 0;

static void add_command(String8 name, u32 min, u32 max, Proc* proc);
static void command_help(String8* args);
static void command_exit(String8* args);
static void command_load(String8* args);
static void command_save(String8* args);
static void command_add(String8* args);
static void command_saves(String8* args);

static void init_commands();
static u64 parse_line_args(String8 line);
static void run_command(String8 line);

#endif
