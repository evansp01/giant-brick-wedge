/** @file setup_idt.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <timer_defines.h>
#include <keyhelp.h>
#include <idt.h>
#include <seg.h>
#include <simics.h>
#include <syscall_int.h>
#include <asm.h>
#include "mode_switch.h"
#include "setup_idt.h"

/** @brief Struct for Interrupt Descriptor Table (IDT) entries
 */
typedef struct {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t reserved;
    uint8_t gate_type : 3;
    uint8_t double_word : 1;
    uint8_t zero : 1;
    uint8_t privilege_level : 2;
    uint8_t present : 1;
    uint16_t offset_high;
} IDT_entry;

/** @brief Installs the handlers in the IDT
 *
 *  @return void
 */
void install_exceptions()
{
    // Set IDT entry fields and add handlers to IDT
    set_idt_exception(INT_ASM(IDT_DE), TRAP, IDT_DE);
    set_idt_exception(INT_ASM(IDT_DB), TRAP, IDT_DB);
    set_idt_exception(INT_ASM(IDT_NMI), TRAP, IDT_NMI);
    set_idt_exception(INT_ASM(IDT_BP), TRAP, IDT_BP);
    set_idt_exception(INT_ASM(IDT_OF), TRAP, IDT_OF);
    set_idt_exception(INT_ASM(IDT_BR), TRAP, IDT_BR);
    set_idt_exception(INT_ASM(IDT_UD), TRAP, IDT_UD);
    set_idt_exception(INT_ASM(IDT_NM), TRAP, IDT_NM);
    set_idt_exception(INT_ASM(IDT_DF), TRAP, IDT_DF);
    set_idt_exception(INT_ASM(IDT_CSO), TRAP, IDT_CSO);
    set_idt_exception(INT_ASM(IDT_TS), TRAP, IDT_TS);
    set_idt_exception(INT_ASM(IDT_NP), TRAP, IDT_NP);
    set_idt_exception(INT_ASM(IDT_SS), TRAP, IDT_SS);
    set_idt_exception(INT_ASM(IDT_GP), TRAP, IDT_GP);
    set_idt_exception(INT_ASM(IDT_PF), INTERRUPT, IDT_PF);
    set_idt_exception(INT_ASM(IDT_MF), TRAP, IDT_MF);
    set_idt_exception(INT_ASM(IDT_AC), TRAP, IDT_AC);
    set_idt_exception(INT_ASM(IDT_MC), TRAP, IDT_MC);
    set_idt_exception(INT_ASM(IDT_XF), TRAP, IDT_XF);
}

/** @brief Installs the system call handlers
 *
 *  @return void
 */
void install_syscalls()
{
    set_idt_syscall(NAME_ASM(fork_syscall), FORK_INT);
    set_idt_syscall(NAME_ASM(exec_syscall), EXEC_INT);
    set_idt_syscall(NAME_ASM(set_status_syscall), SET_STATUS_INT);
    set_idt_syscall(NAME_ASM(vanish_syscall), VANISH_INT);
    set_idt_syscall(NAME_ASM(task_vanish_syscall), TASK_VANISH_INT);
    set_idt_syscall(NAME_ASM(wait_syscall), WAIT_INT);

    set_idt_syscall(NAME_ASM(gettid_syscall), GETTID_INT);
    set_idt_syscall(NAME_ASM(yield_syscall), YIELD_INT);
    set_idt_syscall(NAME_ASM(deschedule_syscall), DESCHEDULE_INT);
    set_idt_syscall(NAME_ASM(make_runnable_syscall), MAKE_RUNNABLE_INT);
    set_idt_syscall(NAME_ASM(get_ticks_syscall), GET_TICKS_INT);
    set_idt_syscall(NAME_ASM(sleep_syscall), SLEEP_INT);
    set_idt_syscall(NAME_ASM(thread_fork_syscall), THREAD_FORK_INT);

    set_idt_syscall(NAME_ASM(new_pages_syscall), NEW_PAGES_INT);
    set_idt_syscall(NAME_ASM(remove_pages_syscall), REMOVE_PAGES_INT);

    set_idt_syscall(NAME_ASM(getchar_syscall), GETCHAR_INT);
    set_idt_syscall(NAME_ASM(readline_syscall), READLINE_INT);
    set_idt_syscall(NAME_ASM(print_syscall), PRINT_INT);
    set_idt_syscall(NAME_ASM(set_term_color_syscall), SET_TERM_COLOR_INT);
    set_idt_syscall(NAME_ASM(set_cursor_pos_syscall), SET_CURSOR_POS_INT);
    set_idt_syscall(NAME_ASM(get_cursor_pos_syscall), GET_CURSOR_POS_INT);

    set_idt_syscall(NAME_ASM(halt_syscall), HALT_INT);
    set_idt_syscall(NAME_ASM(readfile_syscall), READFILE_INT);
    set_idt_syscall(NAME_ASM(misbehave_syscall), MISBEHAVE_INT);
    set_idt_syscall(NAME_ASM(swexn_syscall), SWEXN_INT);
    
    set_idt_syscall(NAME_ASM(udriv_register_syscall), UDRIV_REGISTER_INT);
    set_idt_syscall(NAME_ASM(udriv_deregister_syscall), UDRIV_DEREGISTER_INT);
    set_idt_syscall(NAME_ASM(udriv_send_syscall), UDRIV_SEND_INT);
    set_idt_syscall(NAME_ASM(udriv_wait_syscall), UDRIV_WAIT_INT);
    set_idt_syscall(NAME_ASM(udriv_inb_syscall), UDRIV_INB_INT);
    set_idt_syscall(NAME_ASM(udriv_outb_syscall), UDRIV_OUTB_INT);
    set_idt_syscall(NAME_ASM(udriv_mmap_syscall), UDRIV_MMAP_INT);
}

/** @brief Installs the device driver handlers
 *
 *  @return void
 */
void install_devices()
{
    set_idt_device(INT_ASM(32), 32);
    set_idt_device(INT_ASM(33), 33);
    set_idt_device(INT_ASM(34), 34);
    set_idt_device(INT_ASM(35), 35);
    set_idt_device(INT_ASM(36), 36);
    set_idt_device(INT_ASM(37), 37);
    set_idt_device(INT_ASM(38), 38);
    set_idt_device(INT_ASM(39), 39);
    set_idt_device(INT_ASM(40), 40);
    set_idt_device(INT_ASM(41), 41);
    set_idt_device(INT_ASM(42), 42);
    set_idt_device(INT_ASM(43), 43);
    set_idt_device(INT_ASM(44), 44);
    set_idt_device(INT_ASM(45), 45);
    set_idt_device(INT_ASM(46), 46);
    set_idt_device(INT_ASM(47), 47);
    set_idt_device(INT_ASM(48), 48);
    set_idt_device(INT_ASM(49), 49);
    set_idt_device(INT_ASM(50), 50);
    set_idt_device(INT_ASM(51), 51);
    set_idt_device(INT_ASM(52), 52);
    set_idt_device(INT_ASM(53), 53);
    set_idt_device(INT_ASM(54), 54);
    set_idt_device(INT_ASM(55), 55);
    set_idt_device(INT_ASM(56), 56);
    set_idt_device(INT_ASM(57), 57);
    set_idt_device(INT_ASM(58), 58);
    set_idt_device(INT_ASM(59), 59);
    set_idt_device(INT_ASM(60), 60);
    set_idt_device(INT_ASM(61), 61);
    set_idt_device(INT_ASM(62), 62);
    set_idt_device(INT_ASM(63), 63);
    set_idt_device(INT_ASM(64), 64);
    set_idt_device(INT_ASM(65), 65);
    set_idt_device(INT_ASM(66), 66);
    set_idt_device(INT_ASM(67), 67);
    set_idt_device(INT_ASM(68), 68);
    set_idt_device(INT_ASM(69), 69);
    set_idt_device(INT_ASM(70), 70);
    set_idt_device(INT_ASM(71), 71);
    set_idt_device(INT_ASM(72), 72);
    set_idt_device(INT_ASM(73), 73);
    set_idt_device(INT_ASM(74), 74);
    set_idt_device(INT_ASM(75), 75);
    set_idt_device(INT_ASM(76), 76);
    set_idt_device(INT_ASM(77), 77);
    set_idt_device(INT_ASM(78), 78);
    set_idt_device(INT_ASM(79), 79);
    set_idt_device(INT_ASM(80), 80);
    set_idt_device(INT_ASM(81), 81);
    set_idt_device(INT_ASM(82), 82);
    set_idt_device(INT_ASM(83), 83);
    set_idt_device(INT_ASM(84), 84);
    set_idt_device(INT_ASM(85), 85);
    set_idt_device(INT_ASM(86), 86);
    set_idt_device(INT_ASM(87), 87);
    set_idt_device(INT_ASM(88), 88);
    set_idt_device(INT_ASM(89), 89);
    set_idt_device(INT_ASM(90), 90);
    set_idt_device(INT_ASM(91), 91);
    set_idt_device(INT_ASM(92), 92);
    set_idt_device(INT_ASM(93), 93);
    set_idt_device(INT_ASM(94), 94);
    set_idt_device(INT_ASM(95), 95);
    set_idt_device(INT_ASM(96), 96);
    set_idt_device(INT_ASM(97), 97);
    set_idt_device(INT_ASM(98), 98);
    set_idt_device(INT_ASM(99), 99);
    set_idt_device(INT_ASM(100), 100);
    set_idt_device(INT_ASM(101), 101);
    set_idt_device(INT_ASM(102), 102);
    set_idt_device(INT_ASM(103), 103);
    set_idt_device(INT_ASM(104), 104);
    set_idt_device(INT_ASM(105), 105);
    set_idt_device(INT_ASM(106), 106);
    set_idt_device(INT_ASM(107), 107);
    set_idt_device(INT_ASM(108), 108);
    set_idt_device(INT_ASM(109), 109);
    set_idt_device(INT_ASM(110), 110);
    set_idt_device(INT_ASM(111), 111);
    set_idt_device(INT_ASM(112), 112);
    set_idt_device(INT_ASM(113), 113);
    set_idt_device(INT_ASM(114), 114);
    set_idt_device(INT_ASM(115), 115);
    set_idt_device(INT_ASM(116), 116);
    set_idt_device(INT_ASM(117), 117);
    set_idt_device(INT_ASM(118), 118);
    set_idt_device(INT_ASM(119), 119);
    set_idt_device(INT_ASM(120), 120);
    set_idt_device(INT_ASM(121), 121);
    set_idt_device(INT_ASM(122), 122);
    set_idt_device(INT_ASM(123), 123);
    set_idt_device(INT_ASM(124), 124);
    set_idt_device(INT_ASM(125), 125);
    set_idt_device(INT_ASM(126), 126);
    set_idt_device(INT_ASM(127), 127);
    set_idt_device(INT_ASM(128), 128);
    set_idt_device(INT_ASM(129), 129);
    set_idt_device(INT_ASM(130), 130);
    set_idt_device(INT_ASM(131), 131);
    set_idt_device(INT_ASM(132), 132);
    set_idt_device(INT_ASM(133), 133);
    set_idt_device(INT_ASM(134), 134);
    set_idt_device(INT_ASM(135), 135);
    set_idt_device(INT_ASM(136), 136);
    set_idt_device(INT_ASM(137), 137);
    set_idt_device(INT_ASM(138), 138);
    set_idt_device(INT_ASM(139), 139);
    set_idt_device(INT_ASM(140), 140);
    set_idt_device(INT_ASM(141), 141);
    set_idt_device(INT_ASM(142), 142);
    set_idt_device(INT_ASM(143), 143);
    set_idt_device(INT_ASM(144), 144);
    set_idt_device(INT_ASM(145), 145);
    set_idt_device(INT_ASM(146), 146);
    set_idt_device(INT_ASM(147), 147);
    set_idt_device(INT_ASM(148), 148);
    set_idt_device(INT_ASM(149), 149);
    set_idt_device(INT_ASM(150), 150);
    set_idt_device(INT_ASM(151), 151);
    set_idt_device(INT_ASM(152), 152);
    set_idt_device(INT_ASM(153), 153);
    set_idt_device(INT_ASM(154), 154);
    set_idt_device(INT_ASM(155), 155);
    set_idt_device(INT_ASM(156), 156);
    set_idt_device(INT_ASM(157), 157);
    set_idt_device(INT_ASM(158), 158);
    set_idt_device(INT_ASM(159), 159);
    set_idt_device(INT_ASM(160), 160);
    set_idt_device(INT_ASM(161), 161);
    set_idt_device(INT_ASM(162), 162);
    set_idt_device(INT_ASM(163), 163);
    set_idt_device(INT_ASM(164), 164);
    set_idt_device(INT_ASM(165), 165);
    set_idt_device(INT_ASM(166), 166);
    set_idt_device(INT_ASM(167), 167);
    set_idt_device(INT_ASM(168), 168);
    set_idt_device(INT_ASM(169), 169);
    set_idt_device(INT_ASM(170), 170);
    set_idt_device(INT_ASM(171), 171);
    set_idt_device(INT_ASM(172), 172);
    set_idt_device(INT_ASM(173), 173);
    set_idt_device(INT_ASM(174), 174);
    set_idt_device(INT_ASM(175), 175);
    set_idt_device(INT_ASM(176), 176);
    set_idt_device(INT_ASM(177), 177);
    set_idt_device(INT_ASM(178), 178);
    set_idt_device(INT_ASM(179), 179);
    set_idt_device(INT_ASM(180), 180);
    set_idt_device(INT_ASM(181), 181);
    set_idt_device(INT_ASM(182), 182);
    set_idt_device(INT_ASM(183), 183);
    set_idt_device(INT_ASM(184), 184);
    set_idt_device(INT_ASM(185), 185);
    set_idt_device(INT_ASM(186), 186);
    set_idt_device(INT_ASM(187), 187);
    set_idt_device(INT_ASM(188), 188);
    set_idt_device(INT_ASM(189), 189);
    set_idt_device(INT_ASM(190), 190);
    set_idt_device(INT_ASM(191), 191);
    set_idt_device(INT_ASM(192), 192);
    set_idt_device(INT_ASM(193), 193);
    set_idt_device(INT_ASM(194), 194);
    set_idt_device(INT_ASM(195), 195);
    set_idt_device(INT_ASM(196), 196);
    set_idt_device(INT_ASM(197), 197);
    set_idt_device(INT_ASM(198), 198);
    set_idt_device(INT_ASM(199), 199);
    set_idt_device(INT_ASM(200), 200);
    set_idt_device(INT_ASM(201), 201);
    set_idt_device(INT_ASM(202), 202);
    set_idt_device(INT_ASM(203), 203);
    set_idt_device(INT_ASM(204), 204);
    set_idt_device(INT_ASM(205), 205);
    set_idt_device(INT_ASM(206), 206);
    set_idt_device(INT_ASM(207), 207);
    set_idt_device(INT_ASM(208), 208);
    set_idt_device(INT_ASM(209), 209);
    set_idt_device(INT_ASM(210), 210);
    set_idt_device(INT_ASM(211), 211);
    set_idt_device(INT_ASM(212), 212);
    set_idt_device(INT_ASM(213), 213);
    set_idt_device(INT_ASM(214), 214);
    set_idt_device(INT_ASM(215), 215);
    set_idt_device(INT_ASM(216), 216);
    set_idt_device(INT_ASM(217), 217);
    set_idt_device(INT_ASM(218), 218);
    set_idt_device(INT_ASM(219), 219);
    set_idt_device(INT_ASM(220), 220);
    set_idt_device(INT_ASM(221), 221);
    set_idt_device(INT_ASM(222), 222);
    set_idt_device(INT_ASM(223), 223);
    set_idt_device(INT_ASM(224), 224);
    set_idt_device(INT_ASM(225), 225);
    set_idt_device(INT_ASM(226), 226);
    set_idt_device(INT_ASM(227), 227);
    set_idt_device(INT_ASM(228), 228);
    set_idt_device(INT_ASM(229), 229);
    set_idt_device(INT_ASM(230), 230);
    set_idt_device(INT_ASM(231), 231);
    set_idt_device(INT_ASM(232), 232);
    set_idt_device(INT_ASM(233), 233);
    set_idt_device(INT_ASM(234), 234);
    set_idt_device(INT_ASM(235), 235);
    set_idt_device(INT_ASM(236), 236);
    set_idt_device(INT_ASM(237), 237);
    set_idt_device(INT_ASM(238), 238);
    set_idt_device(INT_ASM(239), 239);
    set_idt_device(INT_ASM(240), 240);
    set_idt_device(INT_ASM(241), 241);
    set_idt_device(INT_ASM(242), 242);
    set_idt_device(INT_ASM(243), 243);
    set_idt_device(INT_ASM(244), 244);
    set_idt_device(INT_ASM(245), 245);
    set_idt_device(INT_ASM(246), 246);
    set_idt_device(INT_ASM(247), 247);
    set_idt_device(INT_ASM(248), 248);
    set_idt_device(INT_ASM(249), 249);
    set_idt_device(INT_ASM(250), 250);
    set_idt_device(INT_ASM(251), 251);
    set_idt_device(INT_ASM(252), 252);
    set_idt_device(INT_ASM(253), 253);
    set_idt_device(INT_ASM(254), 254);
    set_idt_device(INT_ASM(255), 255);
}

/** @brief Installs a handler into the IDT
 *
 *  @param handler Pointer to the handler function
 *  @param type Type of exception
 *  @param index IDT index at which to install handler
 *  @return void
 */
void set_idt_exception(void* handler, int type, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, type, KERNEL, index);
}

/** @brief Installs a syscall handler into the IDT
 *
 *  @param handler Pointer to the handler function
 *  @param index IDT index at which to install handler
 *  @return void
 */
void set_idt_syscall(void* handler, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, TRAP, USER, index);
}

/** @brief Installs a device interrupt handler into the IDT
 *
 *  @param handler Pointer to the handler function
 *  @param type Type of exception
 *  @param index IDT index at which to install handler
 *  @return void
 */
void set_idt_device(void* handler, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, TRAP, KERNEL, index);
}

/** @brief Places the IDT entry in the IDT
 *
 *  @param index IDT entry index
 *  @param entry Struct to be installed in the IDT
 *  @return void
 **/
void install_idt(int index, IDT_entry* entry)
{
    *(((IDT_entry*)idt_base()) + index) = *entry;
}

/** @brief Fills in the IDT entry and adds it to the IDT
 *
 *  @param handler Address of fault handler
 *  @param segment Segment selector for destination code segment
 *  @param type The type of gate to install
 *  @param privilege Descriptor privilege level
 *  @param index IDT table index
 *  @return void
 **/
void set_idt(void* handler, int segment, int type, int privilege, int index)
{
    IDT_entry entry;
    entry.offset_low = (uint16_t)((uint32_t)handler);
    entry.segment = segment;
    entry.reserved = 0;
    entry.gate_type = type;
    entry.double_word = 1; // double word size
    entry.zero = 0;
    entry.privilege_level = privilege;
    entry.present = 1; // entry is preent
    entry.offset_high = (uint16_t)(((uint32_t)handler) >> 16);
    install_idt(index, &entry);
}
