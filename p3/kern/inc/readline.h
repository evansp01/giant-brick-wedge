/** @file readline.h
 *  @brief Interface for keyboard interrupt functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef READLINE_H_
#define READLINE_H_

int readline(int len, char *buf, tcb_t *tcb);

#endif // READLINE_H_