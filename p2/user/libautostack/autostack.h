/* If you want to use assembly language instead of C,
 * delete this autostack.c and provide an autostack.S
 * instead.
 */
#ifndef AUTOSTACK_H
#define AUTOSTACK_H
void install_threaded();
void get_stack_bounds(void** stack_high, void** stack_low);
#endif // AUTOSTACK_H
