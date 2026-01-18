#ifndef UTILS_CLI_1
#define UTILS_CLI_1
#include <Utils/string_view.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_FLAG_CNT 10

struct map_cstr_cstr {
    String_View name;
    String_View val;
    String_View desc;
};

void cli_flag_push(const char* name, const char* desc);
String_View cli_flag_get_val(String_View name);
void cli_parse(int* argc, char*** argv);

#endif
#ifdef UTILS_CLI_IMPL
#undef UTILS_CLI_IMPL

struct map_cstr_cstr Flags[MAX_FLAG_CNT];
uint8_t flag_cnt = 0;

void cli_flag_push(const char* name, const char* desc)
{
    assert(flag_cnt < MAX_FLAG_CNT);
    Flags[flag_cnt++] = (struct map_cstr_cstr) {
        .name = STR(name),
        .desc = STR(desc),
    };
}

void cli_flags_print_help(void)
{
    printf("\n\n-------------------\n");
    for (uint8_t i = 0; i < flag_cnt; i++) {
        printf("\n%-10.*s %.*s", Str_Fmt(Flags[i].name), Str_Fmt(Flags[i].desc));
    }
    printf("\n\n-------------------\n");
}

String_View cli_flag_get_val(String_View name)
{
    for (uint8_t i = 0; i < flag_cnt; i++) {
        if (Flags[i].val.len && sv_compare(Flags[i].name, name)) {
            return Flags[i].val;
        }
    }
    printf("Value of Flag '%.*s' not passed!", Str_Fmt(name));
    cli_flags_print_help();
    exit(1);
}

void cli_flag_set_val(String_View name, String_View val)
{
    for (uint8_t i = 0; i < flag_cnt; i++) {
        if (sv_compare(Flags[i].name, name)) {
            Flags[i].val = val;
            return;
        }
    }
    printf("Invalid Flag: %.*s:%.*s", Str_Fmt(name), Str_Fmt(val));
    cli_flags_print_help();
    exit(1);
}

String_View cli_get_next_arg(int* argc, char*** argv)
{
    assert(*argc > 0);

    char* arg = **argv;

    *argc -= 1;
    *argv += 1;

    return STR(arg);
}

void cli_parse(int* argc, char*** argv)
{

    cli_flag_push("prog", "name of the program being executed");
    cli_flag_set_val(STR("prog"), cli_get_next_arg(argc, argv));

    while (*argc > 0) {
        String_View flag = cli_get_next_arg(argc, argv);
        if (sv_compare(flag, STR("-h")) || sv_compare(flag, STR("--help"))) {
            cli_flags_print_help();
            exit(0);
        }
        String_View val = cli_get_next_arg(argc, argv);
        cli_flag_set_val(flag, val);
    }
}

#endif