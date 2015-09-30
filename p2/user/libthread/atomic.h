#ifndef ATOMIC_H
#define ATOMIC_H

int atomic_xchg(int *ptr, int value);
int atomic_cas(int *ptr, int newval, int oldval);

#endif // ATOMIC_H
