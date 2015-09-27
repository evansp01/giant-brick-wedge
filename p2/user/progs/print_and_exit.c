/** @file print_and_exit.c
 *  @brief prints and exits
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall.h>

/** @brief A short test program which prints, and exits
 *  @return Returns with exit status 42 on success
 **/
int main()
{
    print(12*sizeof(char), "Hello World!");
    set_status(42);
    vanish();
    while (1) {
        continue;
    }
}
