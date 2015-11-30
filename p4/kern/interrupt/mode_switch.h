/** @file mode_switch.h
 *
 *  @brief Interface for mode switching
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef _MODE_SWITCH_H
#define _MODE_SWITCH_H

#include <stdint.h>
#include <idt.h>
#include <control_block.h>

/** @brief A helper macro which expands tokens given to it for NAME_ASM */
#define _NAME_ASM(name) name##_asm

/** @def NAME_ASM_H(name)
 *
 *  @brief Constructs the assembly wrapper function declaration
 *
 *  @param name Fault name
 *  @return void
 **/
#define NAME_ASM_H(name) void _NAME_ASM(name)()

/** @def NAME_ASM(name)
 *
 *  @brief Constructs the assembly handler name for a c function
 *
 *  @param name The name of the c function
 *  @return void
 **/
#define NAME_ASM(name) _NAME_ASM(name)

/** @brief A helper macro for INT_ASM */
#define _INT_ASM(number) interrupt##number##_asm

/** @def INT_ASM(number)
 *
 *  @brief Constructs the wrapper function name for a numbered interrupt
 *  @param number The index of the fault in the IDT
 **/
#define INT_ASM(number) _INT_ASM(number)

/** @def INT_ASM_H(number)
 *
 *  @brief Constructs a header for the wrapper for a numbered interrupt
 *  @param number The index of the interrupt in the IDT
 **/
#define INT_ASM_H(number) void _INT_ASM(number)()

/** @brief Switches to user mode
 *
 *  @param esp Stack pointer with values to be restored
 *  @return void
 **/
void go_to_user_mode(void *esp);

void set_regs();


/*****************************************************************************
 ********* FAULT INTERRUPT HANDLERS *****************************************
 *****************************************************************************/
/** @brief Wrapper for the divide error handler
 *  @return void
 */
INT_ASM_H(IDT_DE);

/** @brief Wrapper for the debug exception handler
 *  @return void
 */
INT_ASM_H(IDT_DB);

/** @brief Wrapper for the NMI exception handler
 *  @return void
 */
INT_ASM_H(IDT_NMI);

/** @brief Wrapper for the breakpoint exception handler
 *  @return void
 */
INT_ASM_H(IDT_BP);

/** @brief Wrapper for the overflow exception handler
 *  @return void
 */
INT_ASM_H(IDT_OF);

/** @brief Wrapper for the BOUND exception handler
 *  @return void
 */
INT_ASM_H(IDT_BR);

/** @brief Wrapper for the invalid opcode exception handler
 *  @return void
 */
INT_ASM_H(IDT_UD);

/** @brief Wrapper for the no math coprocessor exception handler
 *  @return void
 */
INT_ASM_H(IDT_NM);

/** @brief Wrapper for the double fault exception handler
 *  @return void
 */
INT_ASM_H(IDT_DF);

/** @brief Wrapper for the coprocessor segment overrun exception handler
 *  @return void
 */
INT_ASM_H(IDT_CSO);

/** @brief Wrapper for the invalid task segment selector exception handler
 *  @return void
 */
INT_ASM_H(IDT_TS);

/** @brief Wrapper for the segment not present exception handler
 *  @return void
 */
INT_ASM_H(IDT_NP);

/** @brief Wrapper for the stack segment fault exception handler
 *  @return void
 */
INT_ASM_H(IDT_SS);

/** @brief Wrapper for the general protection exception handler
 *  @return void
 */
INT_ASM_H(IDT_GP);

/** @brief Wrapper for the page fault exception handler
 *  @return void
 */
INT_ASM_H(IDT_PF);

/** @brief Wrapper for the x87 FPU floating point error exception handler
 *  @return void
 */
INT_ASM_H(IDT_MF);

/** @brief Wrapper for the alignment check exception handler
 *  @return void
 */
INT_ASM_H(IDT_AC);

/** @brief Wrapper for the machine check exception handler
 *  @return void
 */
INT_ASM_H(IDT_MC);

/** @brief Wrapper for the SIMD floating point exception handler
 *  @return void
 */
INT_ASM_H(IDT_XF);

/*****************************************************************************
 ********* DEVICE INTERRUPT HANDLERS *****************************************
 *****************************************************************************/
/** @brief Wrapper for the timer interrupt handler
 *  @return void
 */
NAME_ASM_H(timer_interrupt);

/** @brief Wrapper for the keyboard interrupt handler
 *  @return void
 */
NAME_ASM_H(keyboard_interrupt);

/*****************************************************************************
 ********* SYSCALL INTERRUPT HANDLERS*****************************************
 *****************************************************************************/

/** @brief Wrapper for fork syscall handler
 *  @return void
 */
NAME_ASM_H(fork_syscall);

/** @brief Wrapper for exec syscall handler
 *  @return void
 */
NAME_ASM_H(exec_syscall);

/** @brief Wrapper for set_status syscall handler
 *  @return void
 */
NAME_ASM_H(set_status_syscall);

/** @brief Wrapper for vanish syscall handler
 *  @return void
 */
NAME_ASM_H(vanish_syscall);

/** @brief Wrapper for task_vanish syscall handler
 *  @return void
 */
NAME_ASM_H(task_vanish_syscall);

/** @brief Wrapper for wait syscall handler
 *  @return void
 */
NAME_ASM_H(wait_syscall);

/** @brief Wrapper for gettid syscall handler
 *  @return void
 */
NAME_ASM_H(gettid_syscall);

/** @brief Wrapper for yield syscall handler
 *  @return void
 */
NAME_ASM_H(yield_syscall);

/** @brief Wrapper for deschedule syscall handler
 *  @return void
 */
NAME_ASM_H(deschedule_syscall);

/** @brief Wrapper for make_runnable syscall handler
 *  @return void
 */
NAME_ASM_H(make_runnable_syscall);

/** @brief Wrapper for get_ticks syscall handler
 *  @return void
 */
NAME_ASM_H(get_ticks_syscall);

/** @brief Wrapper for sleep syscall handler
 *  @return void
 */
NAME_ASM_H(sleep_syscall);

/** @brief Wrapper for thread fork syscall handler
 *  @return void
 */
NAME_ASM_H(thread_fork_syscall);

/** @brief Wrapper for new_pages syscall handler
 *  @return void
 */
NAME_ASM_H(new_pages_syscall);

/** @brief Wrapper for remove_pages syscall handler
 *  @return void
 */
NAME_ASM_H(remove_pages_syscall);

/** @brief Wrapper for getchar syscall handler
 *  @return void
 */
NAME_ASM_H(getchar_syscall);

/** @brief Wrapper for readline syscall handler
 *  @return void
 */
NAME_ASM_H(readline_syscall);

/** @brief Wrapper for print syscall handler
 *  @return void
 */
NAME_ASM_H(print_syscall);

/** @brief Wrapper for set_term_color syscall handler
 *  @return void
 */
NAME_ASM_H(set_term_color_syscall);

/** @brief Wrapper for set_cursor_pos syscall handler
 *  @return void
 */
NAME_ASM_H(set_cursor_pos_syscall);

/** @brief Wrapper for get_cursor_pos syscall handler
 *  @return void
 */
NAME_ASM_H(get_cursor_pos_syscall);

/** @brief Wrapper for halt syscall handler
 *  @return void
 */
NAME_ASM_H(halt_syscall);

/** @brief Wrapper for readfile syscall handler
 *  @return void
 */
NAME_ASM_H(readfile_syscall);

/** @brief Wrapper for misbehave syscall handler
 *  @return void
 */
NAME_ASM_H(misbehave_syscall);

/** @brief Wrapper for swexn syscall handler
 *  @return void
 */
NAME_ASM_H(swexn_syscall);

/** @brief Wrapper for udriv_register syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_register_syscall);

/** @brief Wrapper for udriv_deregister syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_deregister_syscall);

/** @brief Wrapper for udriv_send syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_send_syscall);

/** @brief Wrapper for udriv_wait syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_wait_syscall);

/** @brief Wrapper for udriv_inb syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_inb_syscall);

/** @brief Wrapper for udriv_outb syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_outb_syscall);

/** @brief Wrapper for udriv_mmap syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_mmap_syscall);

/*****************************************************************************
 ********* DEVICE INTERRUPT HANDLERS *****************************************
 *****************************************************************************/

/** @brief Wrapper for user interrupt 32
 *  @return void
 **/
INT_ASM_H(32);

/** @brief Wrapper for user interrupt 33
 *  @return void
 **/
INT_ASM_H(33);

/** @brief Wrapper for user interrupt 34
 *  @return void
 **/
INT_ASM_H(34);

/** @brief Wrapper for user interrupt 35
 *  @return void
 **/
INT_ASM_H(35);

/** @brief Wrapper for user interrupt 36
 *  @return void
 **/
INT_ASM_H(36);

/** @brief Wrapper for user interrupt 37
 *  @return void
 **/
INT_ASM_H(37);

/** @brief Wrapper for user interrupt 38
 *  @return void
 **/
INT_ASM_H(38);

/** @brief Wrapper for user interrupt 39
 *  @return void
 **/
INT_ASM_H(39);

/** @brief Wrapper for user interrupt 40
 *  @return void
 **/
INT_ASM_H(40);

/** @brief Wrapper for user interrupt 41
 *  @return void
 **/
INT_ASM_H(41);

/** @brief Wrapper for user interrupt 42
 *  @return void
 **/
INT_ASM_H(42);

/** @brief Wrapper for user interrupt 43
 *  @return void
 **/
INT_ASM_H(43);

/** @brief Wrapper for user interrupt 44
 *  @return void
 **/
INT_ASM_H(44);

/** @brief Wrapper for user interrupt 45
 *  @return void
 **/
INT_ASM_H(45);

/** @brief Wrapper for user interrupt 46
 *  @return void
 **/
INT_ASM_H(46);

/** @brief Wrapper for user interrupt 47
 *  @return void
 **/
INT_ASM_H(47);

/** @brief Wrapper for user interrupt 48
 *  @return void
 **/
INT_ASM_H(48);

/** @brief Wrapper for user interrupt 49
 *  @return void
 **/
INT_ASM_H(49);

/** @brief Wrapper for user interrupt 50
 *  @return void
 **/
INT_ASM_H(50);

/** @brief Wrapper for user interrupt 51
 *  @return void
 **/
INT_ASM_H(51);

/** @brief Wrapper for user interrupt 52
 *  @return void
 **/
INT_ASM_H(52);

/** @brief Wrapper for user interrupt 53
 *  @return void
 **/
INT_ASM_H(53);

/** @brief Wrapper for user interrupt 54
 *  @return void
 **/
INT_ASM_H(54);

/** @brief Wrapper for user interrupt 55
 *  @return void
 **/
INT_ASM_H(55);

/** @brief Wrapper for user interrupt 56
 *  @return void
 **/
INT_ASM_H(56);

/** @brief Wrapper for user interrupt 57
 *  @return void
 **/
INT_ASM_H(57);

/** @brief Wrapper for user interrupt 58
 *  @return void
 **/
INT_ASM_H(58);

/** @brief Wrapper for user interrupt 59
 *  @return void
 **/
INT_ASM_H(59);

/** @brief Wrapper for user interrupt 60
 *  @return void
 **/
INT_ASM_H(60);

/** @brief Wrapper for user interrupt 61
 *  @return void
 **/
INT_ASM_H(61);

/** @brief Wrapper for user interrupt 62
 *  @return void
 **/
INT_ASM_H(62);

/** @brief Wrapper for user interrupt 63
 *  @return void
 **/
INT_ASM_H(63);

/** @brief Wrapper for user interrupt 64
 *  @return void
 **/
INT_ASM_H(64);

/** @brief Wrapper for user interrupt 65
 *  @return void
 **/
INT_ASM_H(65);

/** @brief Wrapper for user interrupt 66
 *  @return void
 **/
INT_ASM_H(66);

/** @brief Wrapper for user interrupt 67
 *  @return void
 **/
INT_ASM_H(67);

/** @brief Wrapper for user interrupt 68
 *  @return void
 **/
INT_ASM_H(68);

/** @brief Wrapper for user interrupt 69
 *  @return void
 **/
INT_ASM_H(69);

/** @brief Wrapper for user interrupt 70
 *  @return void
 **/
INT_ASM_H(70);

/** @brief Wrapper for user interrupt 71
 *  @return void
 **/
INT_ASM_H(71);

/** @brief Wrapper for user interrupt 72
 *  @return void
 **/
INT_ASM_H(72);

/** @brief Wrapper for user interrupt 73
 *  @return void
 **/
INT_ASM_H(73);

/** @brief Wrapper for user interrupt 74
 *  @return void
 **/
INT_ASM_H(74);

/** @brief Wrapper for user interrupt 75
 *  @return void
 **/
INT_ASM_H(75);

/** @brief Wrapper for user interrupt 76
 *  @return void
 **/
INT_ASM_H(76);

/** @brief Wrapper for user interrupt 77
 *  @return void
 **/
INT_ASM_H(77);

/** @brief Wrapper for user interrupt 78
 *  @return void
 **/
INT_ASM_H(78);

/** @brief Wrapper for user interrupt 79
 *  @return void
 **/
INT_ASM_H(79);

/** @brief Wrapper for user interrupt 80
 *  @return void
 **/
INT_ASM_H(80);

/** @brief Wrapper for user interrupt 81
 *  @return void
 **/
INT_ASM_H(81);

/** @brief Wrapper for user interrupt 82
 *  @return void
 **/
INT_ASM_H(82);

/** @brief Wrapper for user interrupt 83
 *  @return void
 **/
INT_ASM_H(83);

/** @brief Wrapper for user interrupt 84
 *  @return void
 **/
INT_ASM_H(84);

/** @brief Wrapper for user interrupt 85
 *  @return void
 **/
INT_ASM_H(85);

/** @brief Wrapper for user interrupt 86
 *  @return void
 **/
INT_ASM_H(86);

/** @brief Wrapper for user interrupt 87
 *  @return void
 **/
INT_ASM_H(87);

/** @brief Wrapper for user interrupt 88
 *  @return void
 **/
INT_ASM_H(88);

/** @brief Wrapper for user interrupt 89
 *  @return void
 **/
INT_ASM_H(89);

/** @brief Wrapper for user interrupt 90
 *  @return void
 **/
INT_ASM_H(90);

/** @brief Wrapper for user interrupt 91
 *  @return void
 **/
INT_ASM_H(91);

/** @brief Wrapper for user interrupt 92
 *  @return void
 **/
INT_ASM_H(92);

/** @brief Wrapper for user interrupt 93
 *  @return void
 **/
INT_ASM_H(93);

/** @brief Wrapper for user interrupt 94
 *  @return void
 **/
INT_ASM_H(94);

/** @brief Wrapper for user interrupt 95
 *  @return void
 **/
INT_ASM_H(95);

/** @brief Wrapper for user interrupt 96
 *  @return void
 **/
INT_ASM_H(96);

/** @brief Wrapper for user interrupt 97
 *  @return void
 **/
INT_ASM_H(97);

/** @brief Wrapper for user interrupt 98
 *  @return void
 **/
INT_ASM_H(98);

/** @brief Wrapper for user interrupt 99
 *  @return void
 **/
INT_ASM_H(99);

/** @brief Wrapper for user interrupt 100
 *  @return void
 **/
INT_ASM_H(100);

/** @brief Wrapper for user interrupt 101
 *  @return void
 **/
INT_ASM_H(101);

/** @brief Wrapper for user interrupt 102
 *  @return void
 **/
INT_ASM_H(102);

/** @brief Wrapper for user interrupt 103
 *  @return void
 **/
INT_ASM_H(103);

/** @brief Wrapper for user interrupt 104
 *  @return void
 **/
INT_ASM_H(104);

/** @brief Wrapper for user interrupt 105
 *  @return void
 **/
INT_ASM_H(105);

/** @brief Wrapper for user interrupt 106
 *  @return void
 **/
INT_ASM_H(106);

/** @brief Wrapper for user interrupt 107
 *  @return void
 **/
INT_ASM_H(107);

/** @brief Wrapper for user interrupt 108
 *  @return void
 **/
INT_ASM_H(108);

/** @brief Wrapper for user interrupt 109
 *  @return void
 **/
INT_ASM_H(109);

/** @brief Wrapper for user interrupt 110
 *  @return void
 **/
INT_ASM_H(110);

/** @brief Wrapper for user interrupt 111
 *  @return void
 **/
INT_ASM_H(111);

/** @brief Wrapper for user interrupt 112
 *  @return void
 **/
INT_ASM_H(112);

/** @brief Wrapper for user interrupt 113
 *  @return void
 **/
INT_ASM_H(113);

/** @brief Wrapper for user interrupt 114
 *  @return void
 **/
INT_ASM_H(114);

/** @brief Wrapper for user interrupt 115
 *  @return void
 **/
INT_ASM_H(115);

/** @brief Wrapper for user interrupt 116
 *  @return void
 **/
INT_ASM_H(116);

/** @brief Wrapper for user interrupt 117
 *  @return void
 **/
INT_ASM_H(117);

/** @brief Wrapper for user interrupt 118
 *  @return void
 **/
INT_ASM_H(118);

/** @brief Wrapper for user interrupt 119
 *  @return void
 **/
INT_ASM_H(119);

/** @brief Wrapper for user interrupt 120
 *  @return void
 **/
INT_ASM_H(120);

/** @brief Wrapper for user interrupt 121
 *  @return void
 **/
INT_ASM_H(121);

/** @brief Wrapper for user interrupt 122
 *  @return void
 **/
INT_ASM_H(122);

/** @brief Wrapper for user interrupt 123
 *  @return void
 **/
INT_ASM_H(123);

/** @brief Wrapper for user interrupt 124
 *  @return void
 **/
INT_ASM_H(124);

/** @brief Wrapper for user interrupt 125
 *  @return void
 **/
INT_ASM_H(125);

/** @brief Wrapper for user interrupt 126
 *  @return void
 **/
INT_ASM_H(126);

/** @brief Wrapper for user interrupt 127
 *  @return void
 **/
INT_ASM_H(127);

/** @brief Wrapper for user interrupt 128
 *  @return void
 **/
INT_ASM_H(128);

/** @brief Wrapper for user interrupt 129
 *  @return void
 **/
INT_ASM_H(129);

/** @brief Wrapper for user interrupt 130
 *  @return void
 **/
INT_ASM_H(130);

/** @brief Wrapper for user interrupt 131
 *  @return void
 **/
INT_ASM_H(131);

/** @brief Wrapper for user interrupt 132
 *  @return void
 **/
INT_ASM_H(132);

/** @brief Wrapper for user interrupt 133
 *  @return void
 **/
INT_ASM_H(133);

/** @brief Wrapper for user interrupt 134
 *  @return void
 **/
INT_ASM_H(134);

/** @brief Wrapper for user interrupt 135
 *  @return void
 **/
INT_ASM_H(135);

/** @brief Wrapper for user interrupt 136
 *  @return void
 **/
INT_ASM_H(136);

/** @brief Wrapper for user interrupt 137
 *  @return void
 **/
INT_ASM_H(137);

/** @brief Wrapper for user interrupt 138
 *  @return void
 **/
INT_ASM_H(138);

/** @brief Wrapper for user interrupt 139
 *  @return void
 **/
INT_ASM_H(139);

/** @brief Wrapper for user interrupt 140
 *  @return void
 **/
INT_ASM_H(140);

/** @brief Wrapper for user interrupt 141
 *  @return void
 **/
INT_ASM_H(141);

/** @brief Wrapper for user interrupt 142
 *  @return void
 **/
INT_ASM_H(142);

/** @brief Wrapper for user interrupt 143
 *  @return void
 **/
INT_ASM_H(143);

/** @brief Wrapper for user interrupt 144
 *  @return void
 **/
INT_ASM_H(144);

/** @brief Wrapper for user interrupt 145
 *  @return void
 **/
INT_ASM_H(145);

/** @brief Wrapper for user interrupt 146
 *  @return void
 **/
INT_ASM_H(146);

/** @brief Wrapper for user interrupt 147
 *  @return void
 **/
INT_ASM_H(147);

/** @brief Wrapper for user interrupt 148
 *  @return void
 **/
INT_ASM_H(148);

/** @brief Wrapper for user interrupt 149
 *  @return void
 **/
INT_ASM_H(149);

/** @brief Wrapper for user interrupt 150
 *  @return void
 **/
INT_ASM_H(150);

/** @brief Wrapper for user interrupt 151
 *  @return void
 **/
INT_ASM_H(151);

/** @brief Wrapper for user interrupt 152
 *  @return void
 **/
INT_ASM_H(152);

/** @brief Wrapper for user interrupt 153
 *  @return void
 **/
INT_ASM_H(153);

/** @brief Wrapper for user interrupt 154
 *  @return void
 **/
INT_ASM_H(154);

/** @brief Wrapper for user interrupt 155
 *  @return void
 **/
INT_ASM_H(155);

/** @brief Wrapper for user interrupt 156
 *  @return void
 **/
INT_ASM_H(156);

/** @brief Wrapper for user interrupt 157
 *  @return void
 **/
INT_ASM_H(157);

/** @brief Wrapper for user interrupt 158
 *  @return void
 **/
INT_ASM_H(158);

/** @brief Wrapper for user interrupt 159
 *  @return void
 **/
INT_ASM_H(159);

/** @brief Wrapper for user interrupt 160
 *  @return void
 **/
INT_ASM_H(160);

/** @brief Wrapper for user interrupt 161
 *  @return void
 **/
INT_ASM_H(161);

/** @brief Wrapper for user interrupt 162
 *  @return void
 **/
INT_ASM_H(162);

/** @brief Wrapper for user interrupt 163
 *  @return void
 **/
INT_ASM_H(163);

/** @brief Wrapper for user interrupt 164
 *  @return void
 **/
INT_ASM_H(164);

/** @brief Wrapper for user interrupt 165
 *  @return void
 **/
INT_ASM_H(165);

/** @brief Wrapper for user interrupt 166
 *  @return void
 **/
INT_ASM_H(166);

/** @brief Wrapper for user interrupt 167
 *  @return void
 **/
INT_ASM_H(167);

/** @brief Wrapper for user interrupt 168
 *  @return void
 **/
INT_ASM_H(168);

/** @brief Wrapper for user interrupt 169
 *  @return void
 **/
INT_ASM_H(169);

/** @brief Wrapper for user interrupt 170
 *  @return void
 **/
INT_ASM_H(170);

/** @brief Wrapper for user interrupt 171
 *  @return void
 **/
INT_ASM_H(171);

/** @brief Wrapper for user interrupt 172
 *  @return void
 **/
INT_ASM_H(172);

/** @brief Wrapper for user interrupt 173
 *  @return void
 **/
INT_ASM_H(173);

/** @brief Wrapper for user interrupt 174
 *  @return void
 **/
INT_ASM_H(174);

/** @brief Wrapper for user interrupt 175
 *  @return void
 **/
INT_ASM_H(175);

/** @brief Wrapper for user interrupt 176
 *  @return void
 **/
INT_ASM_H(176);

/** @brief Wrapper for user interrupt 177
 *  @return void
 **/
INT_ASM_H(177);

/** @brief Wrapper for user interrupt 178
 *  @return void
 **/
INT_ASM_H(178);

/** @brief Wrapper for user interrupt 179
 *  @return void
 **/
INT_ASM_H(179);

/** @brief Wrapper for user interrupt 180
 *  @return void
 **/
INT_ASM_H(180);

/** @brief Wrapper for user interrupt 181
 *  @return void
 **/
INT_ASM_H(181);

/** @brief Wrapper for user interrupt 182
 *  @return void
 **/
INT_ASM_H(182);

/** @brief Wrapper for user interrupt 183
 *  @return void
 **/
INT_ASM_H(183);

/** @brief Wrapper for user interrupt 184
 *  @return void
 **/
INT_ASM_H(184);

/** @brief Wrapper for user interrupt 185
 *  @return void
 **/
INT_ASM_H(185);

/** @brief Wrapper for user interrupt 186
 *  @return void
 **/
INT_ASM_H(186);

/** @brief Wrapper for user interrupt 187
 *  @return void
 **/
INT_ASM_H(187);

/** @brief Wrapper for user interrupt 188
 *  @return void
 **/
INT_ASM_H(188);

/** @brief Wrapper for user interrupt 189
 *  @return void
 **/
INT_ASM_H(189);

/** @brief Wrapper for user interrupt 190
 *  @return void
 **/
INT_ASM_H(190);

/** @brief Wrapper for user interrupt 191
 *  @return void
 **/
INT_ASM_H(191);

/** @brief Wrapper for user interrupt 192
 *  @return void
 **/
INT_ASM_H(192);

/** @brief Wrapper for user interrupt 193
 *  @return void
 **/
INT_ASM_H(193);

/** @brief Wrapper for user interrupt 194
 *  @return void
 **/
INT_ASM_H(194);

/** @brief Wrapper for user interrupt 195
 *  @return void
 **/
INT_ASM_H(195);

/** @brief Wrapper for user interrupt 196
 *  @return void
 **/
INT_ASM_H(196);

/** @brief Wrapper for user interrupt 197
 *  @return void
 **/
INT_ASM_H(197);

/** @brief Wrapper for user interrupt 198
 *  @return void
 **/
INT_ASM_H(198);

/** @brief Wrapper for user interrupt 199
 *  @return void
 **/
INT_ASM_H(199);

/** @brief Wrapper for user interrupt 200
 *  @return void
 **/
INT_ASM_H(200);

/** @brief Wrapper for user interrupt 201
 *  @return void
 **/
INT_ASM_H(201);

/** @brief Wrapper for user interrupt 202
 *  @return void
 **/
INT_ASM_H(202);

/** @brief Wrapper for user interrupt 203
 *  @return void
 **/
INT_ASM_H(203);

/** @brief Wrapper for user interrupt 204
 *  @return void
 **/
INT_ASM_H(204);

/** @brief Wrapper for user interrupt 205
 *  @return void
 **/
INT_ASM_H(205);

/** @brief Wrapper for user interrupt 206
 *  @return void
 **/
INT_ASM_H(206);

/** @brief Wrapper for user interrupt 207
 *  @return void
 **/
INT_ASM_H(207);

/** @brief Wrapper for user interrupt 208
 *  @return void
 **/
INT_ASM_H(208);

/** @brief Wrapper for user interrupt 209
 *  @return void
 **/
INT_ASM_H(209);

/** @brief Wrapper for user interrupt 210
 *  @return void
 **/
INT_ASM_H(210);

/** @brief Wrapper for user interrupt 211
 *  @return void
 **/
INT_ASM_H(211);

/** @brief Wrapper for user interrupt 212
 *  @return void
 **/
INT_ASM_H(212);

/** @brief Wrapper for user interrupt 213
 *  @return void
 **/
INT_ASM_H(213);

/** @brief Wrapper for user interrupt 214
 *  @return void
 **/
INT_ASM_H(214);

/** @brief Wrapper for user interrupt 215
 *  @return void
 **/
INT_ASM_H(215);

/** @brief Wrapper for user interrupt 216
 *  @return void
 **/
INT_ASM_H(216);

/** @brief Wrapper for user interrupt 217
 *  @return void
 **/
INT_ASM_H(217);

/** @brief Wrapper for user interrupt 218
 *  @return void
 **/
INT_ASM_H(218);

/** @brief Wrapper for user interrupt 219
 *  @return void
 **/
INT_ASM_H(219);

/** @brief Wrapper for user interrupt 220
 *  @return void
 **/
INT_ASM_H(220);

/** @brief Wrapper for user interrupt 221
 *  @return void
 **/
INT_ASM_H(221);

/** @brief Wrapper for user interrupt 222
 *  @return void
 **/
INT_ASM_H(222);

/** @brief Wrapper for user interrupt 223
 *  @return void
 **/
INT_ASM_H(223);

/** @brief Wrapper for user interrupt 224
 *  @return void
 **/
INT_ASM_H(224);

/** @brief Wrapper for user interrupt 225
 *  @return void
 **/
INT_ASM_H(225);

/** @brief Wrapper for user interrupt 226
 *  @return void
 **/
INT_ASM_H(226);

/** @brief Wrapper for user interrupt 227
 *  @return void
 **/
INT_ASM_H(227);

/** @brief Wrapper for user interrupt 228
 *  @return void
 **/
INT_ASM_H(228);

/** @brief Wrapper for user interrupt 229
 *  @return void
 **/
INT_ASM_H(229);

/** @brief Wrapper for user interrupt 230
 *  @return void
 **/
INT_ASM_H(230);

/** @brief Wrapper for user interrupt 231
 *  @return void
 **/
INT_ASM_H(231);

/** @brief Wrapper for user interrupt 232
 *  @return void
 **/
INT_ASM_H(232);

/** @brief Wrapper for user interrupt 233
 *  @return void
 **/
INT_ASM_H(233);

/** @brief Wrapper for user interrupt 234
 *  @return void
 **/
INT_ASM_H(234);

/** @brief Wrapper for user interrupt 235
 *  @return void
 **/
INT_ASM_H(235);

/** @brief Wrapper for user interrupt 236
 *  @return void
 **/
INT_ASM_H(236);

/** @brief Wrapper for user interrupt 237
 *  @return void
 **/
INT_ASM_H(237);

/** @brief Wrapper for user interrupt 238
 *  @return void
 **/
INT_ASM_H(238);

/** @brief Wrapper for user interrupt 239
 *  @return void
 **/
INT_ASM_H(239);

/** @brief Wrapper for user interrupt 240
 *  @return void
 **/
INT_ASM_H(240);

/** @brief Wrapper for user interrupt 241
 *  @return void
 **/
INT_ASM_H(241);

/** @brief Wrapper for user interrupt 242
 *  @return void
 **/
INT_ASM_H(242);

/** @brief Wrapper for user interrupt 243
 *  @return void
 **/
INT_ASM_H(243);

/** @brief Wrapper for user interrupt 244
 *  @return void
 **/
INT_ASM_H(244);

/** @brief Wrapper for user interrupt 245
 *  @return void
 **/
INT_ASM_H(245);

/** @brief Wrapper for user interrupt 246
 *  @return void
 **/
INT_ASM_H(246);

/** @brief Wrapper for user interrupt 247
 *  @return void
 **/
INT_ASM_H(247);

/** @brief Wrapper for user interrupt 248
 *  @return void
 **/
INT_ASM_H(248);

/** @brief Wrapper for user interrupt 249
 *  @return void
 **/
INT_ASM_H(249);

/** @brief Wrapper for user interrupt 250
 *  @return void
 **/
INT_ASM_H(250);

/** @brief Wrapper for user interrupt 251
 *  @return void
 **/
INT_ASM_H(251);

/** @brief Wrapper for user interrupt 252
 *  @return void
 **/
INT_ASM_H(252);

/** @brief Wrapper for user interrupt 253
 *  @return void
 **/
INT_ASM_H(253);

/** @brief Wrapper for user interrupt 254
 *  @return void
 **/
INT_ASM_H(254);

/** @brief Wrapper for user interrupt 255
 *  @return void
 **/
INT_ASM_H(255);

#endif /* _MODE_SWITCH_H */
