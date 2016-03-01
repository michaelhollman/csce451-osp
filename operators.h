#ifndef _operators_h_
#define _operators_h_

static const char OP_OUTPUT[] = ">";
static const char OP_INPUT[] = "<";
static const char OP_APPEND[] = ">>";
static const char OP_PIPE[] = "|";
static const char OP_ON_SUCCESS[] = "&&";
static const char OP_ON_FAIL[] = "||";
static const char OP_ON_ANY[] = ";";

int is_file_operator(char* str);
int is_terminating_operator(char *str);

#endif
