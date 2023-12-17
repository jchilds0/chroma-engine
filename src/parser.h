/*
 * Header for parser submodule
 */

#ifndef CHROMA_PARSER
#define CHROMA_PARSER

#include "chroma-typedefs.h"

int parser_tcp_start_server(char *, int);
extern void parser_read_socket(Engine *eng, int *page_num, int *action);

#endif // !CHROMA_PARSER
