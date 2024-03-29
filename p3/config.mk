###########################################################################
# This is the include file for the make file.
# You should have to edit only this file to get things to build.
###########################################################################

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 8

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = ack actual_wait bistromath cat cho cho2 cho_variant chow \
           coolness deschedule_hang exec_basic exec_basic_helper exec_nonexist \
           fib fork_bomb fork_exit_bomb fork_test1 fork_wait fork_wait_bomb \
           getpid_test1 halt_test knife loader_test1 loader_test2 make_crash \
           make_crash_helper mem_eat_test mem_permissions merchant \
           minclone_mem new_pages peon print_basic readline_basic \
           register_test remove_pages_test1 remove_pages_test2 \
           slaughter sleep_test1 stack_test1 swexn_basic_test \
           swexn_cookie_monster swexn_dispatch swexn_regs \
           swexn_stands_for_swextensible swexn_uninstall_test wait_getpid \
		   wild_test1 work yield_desc_mkrun

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS = readline_threaded print_threaded beauty_sleep does_it_leak \
               does_it_leak_too

###########################################################################
# Data files provided by course staff to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the 410user/files
# directory.
#
410FILES =

###########################################################################
# Data files you have created which you wish to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the user/files
# directory.
#
STUDENTFILES =

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o array_queue.o frame_alloc.o thread.o \
			  thread_asm.o mutex.o atomic.o cond.o sem.o exit.o rwlock.o



# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = set_status.o vanish.o fork.o exec.o wait.o \
			   task_vanish.o new_pages.o remove_pages.o readfile.o halt.o \
			   misbehave.o gettid.o yield.o deschedule.o make_runnable.o \
			   get_ticks.o sleep.o swexn.o getchar.o readline.o  print.o \
			   set_term_color.o set_cursor_pos.o get_cursor_pos.o



###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o
#
# Kernel object files you provide in from kern/
#
KERN_SYSCALL = syscall/fork.o syscall/syscall.o syscall/exec.o syscall/swexn.o \
               syscall/halt.o syscall/console_syscalls.o syscall/wait_vanish.o \
               syscall/readline.o 
KERN_COMMON = common/int_hash.o common/malloc_wrappers.o common/console.o \
              common/control_block.o common/get_esp.o
KERN_LOCK = lock/mutex.o lock/cond.o
KERN_INTERRUPT = interrupt/fault_print.o interrupt/fault.o \
				 interrupt/mode_switch.o interrupt/mode_switch_asm.o \
				 interrupt/setup_idt.o
KERN_SCHEDULER = scheduler/scheduler.o scheduler/switch_asm.o \
				 scheduler/switch.o scheduler/sleep.o scheduler/timer.o
KERN_VM = vm/vm_asm.o vm/frame_alloc.o vm/vm.o vm/vm_user.o vm/ppd.o \
		  vm/page_fault.o

KERNEL_OBJS = kernel.o
KERNEL_OBJS +=${KERN_SYSCALL}
KERNEL_OBJS +=${KERN_COMMON}
KERNEL_OBJS +=${KERN_LOCK}
KERNEL_OBJS +=${KERN_INTERRUPT}
KERNEL_OBJS +=${KERN_SCHEDULER}
KERNEL_OBJS +=${KERN_VM}

###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#
410REQPROGS = idle init shell

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS =
