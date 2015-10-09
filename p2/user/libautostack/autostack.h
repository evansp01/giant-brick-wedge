/** @file autostack.h
 *  @brief Interface for autostack functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/
#ifndef AUTOSTACK_H
#define AUTOSTACK_H
void install_threaded();
void get_stack_bounds(void** stack_high, void** stack_low);
#endif // AUTOSTACK_H
