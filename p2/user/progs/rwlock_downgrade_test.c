/** 
 * @file rwlock_downgrade_test.c
 * @brief Additional tests for reader/writer locks
 * 
 * Sequence: W0, W1(D), W2, W3, R4, R5, R6
 * Expected: - Downgrade on writer 1 should allow readers 4, 5, 6 to run
 *           - Writers 2, 3 should continue after readers have finished
 *
 * @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 */
#include <thread.h>
#include <mutex.h>
#include <cond.h>
#include <rwlock.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include "410_tests.h"
#include <test.h>
DEF_TEST_NAME("rwlock_test:");

#define NAME_OF_TEST "rwlock_test"
#define STACK_SIZE 4096
#define NUM_THREADS 7
#define R 0
#define W 1
#define D 2

rwlock_t lock;
mutex_t order_lock;
volatile int order = -1;
int init[NUM_THREADS] = {W, D, W, W, R, R, R};
int expected[NUM_THREADS] = {W, W, R, R, R, W, W};
int pass = 0;

int curr_order()
{
    mutex_lock(&order_lock);
    order++;
    mutex_unlock(&order_lock);
    return order;
}

void *reader(void *args)
{
    int thr_num = *((int *)args);
	rwlock_lock(&lock, RWLOCK_READ);
    
    int curr = curr_order();
    
    lprintf("Reader %d running", thr_num);
    if (expected[curr] != R) {
        lprintf("Wrong order, expected writer");
        pass = -1;
    }
    
    sleep(100);
    rwlock_unlock(&lock);
	return((void *)0);
}

void *writer(void *args)
{
	int thr_num = *((int *)args);
	rwlock_lock(&lock, RWLOCK_WRITE);
    
    int curr = curr_order();
    
	lprintf("Writer %d running", thr_num);
    if (expected[curr] != W) {
        lprintf("Wrong order, expected reader");
        pass = -1;
    }
    sleep(500);
    rwlock_unlock(&lock);
	return((void *)0);
}

void *downgrade(void *args)
{
	int thr_num = *((int *)args);
	rwlock_lock(&lock, RWLOCK_WRITE);
    
    int curr = curr_order();
    
	lprintf("Writer(to-be-downgraded) %d running", thr_num);
    if (expected[curr] != W) {
        lprintf("Wrong order, expected reader");
        pass = -1;
    }
    sleep(500);
    rwlock_downgrade(&lock);
    lprintf("Reader(downgraded) %d running", thr_num);
    sleep(100);
    rwlock_unlock(&lock);
	return((void *)0);
}

int main()
{
	int thr, tid1;
    int tid[NUM_THREADS];
    
    REPORT_LOCAL_INIT;
	REPORT_START_CMPLT;

	thr_init(STACK_SIZE);

	REPORT_ON_ERR(rwlock_init(&lock));
    REPORT_ON_ERR(mutex_init(&order_lock));
	
    for (thr = 0; thr < NUM_THREADS; thr++) {
        if (init[thr] == W) {
            tid1 = thr_create(writer, (void *)(&thr));
            lprintf("Added writer %d thread", thr);
        }
        else if (init[thr] == R) {
            tid1 = thr_create(reader, (void *)(&thr));
            lprintf("Added reader %d thread", thr);
        }
        else {
            tid1 = thr_create(downgrade, (void *)(&thr));
            lprintf("Added writer(downgrade) %d thread", thr);
        }
        
        if (tid1 < 0) {
            REPORT_MISC("Failed create");
            REPORT_END_FAIL;
            return -1;
        }
        tid[thr] = tid1;
    }
	
	for (thr = 0; thr < NUM_THREADS; thr++) {
        if (thr_join(tid[thr], NULL) < 0) {
            REPORT_MISC("Failed join");
            REPORT_END_FAIL;
            return -1;
        }
    }
    
    if (pass == 0)
        REPORT_END_SUCCESS;
    else
        REPORT_END_FAIL;

	thr_exit((void *)pass);
	return pass;
}
