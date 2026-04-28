#ifndef USER_SHELL_PARSER_H
#define USER_SHELL_PARSER_H

int parser_is_blank_input(const char *line);
int parser_parse_input(char *line, char ***argv_out);
void parser_free_argv(char **argv);

#endif
