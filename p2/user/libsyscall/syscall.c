/*
 *
 *    #####          #######         #######         ######            ###
 *   #     #            #            #     #         #     #           ###
 *   #                  #            #     #         #     #           ###
 *    #####             #            #     #         ######             #
 *         #            #            #     #         #
 *   #     #            #            #     #         #                 ###
 *    #####             #            #######         #                 ###
 *
 *
 *   You should probably NOT EDIT THIS FILE in any way!
 *
 *   You should probably DELETE this file, insert all of your
 *   Project 2 stub files, and edit config.mk accordingly.
 *
 *   Alternatively, you can DELETE pieces from this file as
 *   you write your stubs.  But if you forget half-way through
 *   that that's the plan, you'll have a fun debugging problem!
 *
 */

#include <syscall.h>

int fork(void)
{
	return -1;
}

int exec(char *execname, char *argvec[])
{
	return -1;
}

int wait(int *status_ptr)
{
	return -1;
}

void halt(void)
{
	while (1)
		continue;
}

int readfile(char *filename, char *buf, int count, int offset)
{
	return -1;
}

void task_vanish(int status)
{
	status ^= status;
	status /= status;
	while (1)
		continue;
}

int new_pages(void * addr, int len)
{
	return -1;
}

int remove_pages(void * addr)
{
	return -1;
}

void misbehave(int mode)
{
	return;
}
