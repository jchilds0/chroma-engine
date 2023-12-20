/*
 * Header for parser module
 *
 * Exposes the functions
 *
 *    int parser_tcp_start_server(char *addr, int port);
 *    void parser_read_socket(Engine *eng, int *page_num, int *action);
 *  
 * to the program. parser_tcp_start_server starts up
 * a tcp server for a given address and port. 
 * parser_read_socket listens to the tcp server for an 
 * incoming message with a timeout. If a message is recieved,
 * the message is parsed, updating the relevant geometry data
 * and the page_num and action passed to the function.
 */

#ifndef CHROMA_PARSER
#define CHROMA_PARSER

#include "chroma-typedefs.h"

extern int parser_tcp_start_server(char *addr, int port);
extern void parser_read_socket(Engine *eng, int *page_num, int *action);

#endif // !CHROMA_PARSER
