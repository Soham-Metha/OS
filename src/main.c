#include <Utils/cmd_line.h>
#include <Utils/files.h>
#include <Utils/mem_manager.h>
#include <Utils/string_view.h>
#include <proj.h>

int main(int argc, char** argv)
{
    cli_flag_push("--say", "pass the string to be printed!");
    cli_parse(&argc, &argv);

    printf("%.*s\n", Str_Fmt(cli_flag_get_val(STR("--say"))));
    return 0;
}
