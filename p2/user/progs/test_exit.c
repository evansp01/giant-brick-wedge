#include <syscall.h>

int main()
{
    set_status(42);
    vanish();
    while(1){
        continue;
    }
}
