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

extern void parser_init_sockets(void);
extern void parser_check_socket(int server_socket);
extern int parser_tcp_start_server(int port);
extern int parser_tcp_start_client(char *addr, int port);

extern int parser_parse_graphic(Engine *eng, PageStatus *status);
extern int parser_parse_hub(Engine *eng);
extern int parser_update_template(Engine *eng, int page_num);

#endif // !CHROMA_PARSER
