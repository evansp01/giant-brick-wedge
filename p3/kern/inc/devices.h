/** @file devices.h
 *  @brief Interface for timer and keyboard interrupt functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef DEVICES_H_
#define DEVICES_H_

int readline(int len, char *buf, tcb_t *tcb);

#endif // DEVICES_H_