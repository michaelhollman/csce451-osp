#ifndef _operators_h_
#define _operators_h_

static const char OUTPUT[] = ">";
static const char INPUT[] = "<";
static const char APPEND[] = ">>";
static const char PIPE[] = "|";
static const char ON_SUCCESS[] = "&&";
static const char ON_FAIL[] = "||";
static const char ON_ANY[] = ";";

int is_operator(char* str);
int is_terminating_operator(char *str);

#endif
