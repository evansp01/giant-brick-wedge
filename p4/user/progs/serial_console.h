#ifndef H_SERIAL_CONSOLE
#define H_SERIAL_CONSOLE

void print_message(int len, int suggest_id);
int get_next_char(char* c);
int send_to_print(int len, char* buf);
void init_console();


#include "serial_console.c"
#endif //H_SERIAL_CONSOLE
