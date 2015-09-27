#include <syscall.h>
#include <simics.h>

int main()
{
    /* Test getchar() */
    lprintf("Press a key...");
    lprintf("getchar(): %c", getchar());
    
    /* Test readline() */
    int len = 100;
    char buf[len];
    lprintf("readline(): %d", readline(len, buf));
    lprintf("buffer: %s", buf);
    
    /* Test print() */
    int len = 30;
    char *buf = "Hello this is a test message!\n";
    lprintf("print(): %d", print(len, buf));
    
    /* Test set_term_color() */
    int len = 30;
    char *buf = "Hello this is a test message!\n";
    print(len, buf);
    lprintf("set_term_color(0x24): %d", set_term_color(0x24));
    print(len, buf);
    lprintf("set_term_color(ERROR): %d", set_term_color(0x9F));
    
    /* Test set_cursor_pos() and get_cursor_pos() */
    int row, col;
    lprintf("get_cursor_pos(): %d", get_cursor_pos(&row, &col));
    lprintf("row: %d, col: %d", row, col);
    lprintf("set_cursor_pos(20, 20): %d", set_cursor_pos(20, 20));
    lprintf("get_cursor_pos(): %d", get_cursor_pos(&row, &col));
    lprintf("row: %d, col: %d", row, col);
    
    while(1){
        continue;
    }
}
