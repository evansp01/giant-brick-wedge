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
#include <interrupt.h>
#include <udriv_kern.h>
#include <string.h>

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


static void install_exceptions();
static void install_syscalls();


/** @brief Get the address of the idt entry at index
 *
 *  @param index IDT entry index
 *  @return void
 **/
IDT_entry* get_idt(int index)
{
    return (((IDT_entry*)idt_base()) + index);
}


void install_idt()
{
    install_exceptions();
    // zero idt from 32-255
    memset(get_idt(33), 0, (256 - 33) * sizeof(IDT_entry));
    // install timer
    set_idt_device(NAME_ASM(timer_interrupt), TIMER_IDT_ENTRY);
    install_syscalls();
   }

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
    *get_idt(index) = entry;
}

/** @brief Install the idt entry for a user interrupt
 *  
 *  @param interrupt The interrupt to install an entry for
 *  @return Zero on success, less than zero on failure
 **/
int install_user_device(int interrupt)
{
    if (get_idt(interrupt)->present) {
        return -1;
    }
    switch (interrupt) {
    case 33:
        set_idt_device(INT_ASM(33), 33);
        break;
    case 34:
        set_idt_device(INT_ASM(34), 34);
        break;
    case 35:
        set_idt_device(INT_ASM(35), 35);
        break;
    case 36:
        set_idt_device(INT_ASM(36), 36);
        break;
    case 37:
        set_idt_device(INT_ASM(37), 37);
        break;
    case 38:
        set_idt_device(INT_ASM(38), 38);
        break;
    case 39:
        set_idt_device(INT_ASM(39), 39);
        break;
    case 40:
        set_idt_device(INT_ASM(40), 40);
        break;
    case 41:
        set_idt_device(INT_ASM(41), 41);
        break;
    case 42:
        set_idt_device(INT_ASM(42), 42);
        break;
    case 43:
        set_idt_device(INT_ASM(43), 43);
        break;
    case 44:
        set_idt_device(INT_ASM(44), 44);
        break;
    case 45:
        set_idt_device(INT_ASM(45), 45);
        break;
    case 46:
        set_idt_device(INT_ASM(46), 46);
        break;
    case 47:
        set_idt_device(INT_ASM(47), 47);
        break;
    case 48:
        set_idt_device(INT_ASM(48), 48);
        break;
    case 49:
        set_idt_device(INT_ASM(49), 49);
        break;
    case 50:
        set_idt_device(INT_ASM(50), 50);
        break;
    case 51:
        set_idt_device(INT_ASM(51), 51);
        break;
    case 52:
        set_idt_device(INT_ASM(52), 52);
        break;
    case 53:
        set_idt_device(INT_ASM(53), 53);
        break;
    case 54:
        set_idt_device(INT_ASM(54), 54);
        break;
    case 55:
        set_idt_device(INT_ASM(55), 55);
        break;
    case 56:
        set_idt_device(INT_ASM(56), 56);
        break;
    case 57:
        set_idt_device(INT_ASM(57), 57);
        break;
    case 58:
        set_idt_device(INT_ASM(58), 58);
        break;
    case 59:
        set_idt_device(INT_ASM(59), 59);
        break;
    case 60:
        set_idt_device(INT_ASM(60), 60);
        break;
    case 61:
        set_idt_device(INT_ASM(61), 61);
        break;
    case 62:
        set_idt_device(INT_ASM(62), 62);
        break;
    case 63:
        set_idt_device(INT_ASM(63), 63);
        break;
    case 64:
        set_idt_device(INT_ASM(64), 64);
        break;
    case 65:
        set_idt_device(INT_ASM(65), 65);
        break;
    case 66:
        set_idt_device(INT_ASM(66), 66);
        break;
    case 67:
        set_idt_device(INT_ASM(67), 67);
        break;
    case 68:
        set_idt_device(INT_ASM(68), 68);
        break;
    case 69:
        set_idt_device(INT_ASM(69), 69);
        break;
    case 70:
        set_idt_device(INT_ASM(70), 70);
        break;
    case 71:
        set_idt_device(INT_ASM(71), 71);
        break;
    case 72:
        set_idt_device(INT_ASM(72), 72);
        break;
    case 73:
        set_idt_device(INT_ASM(73), 73);
        break;
    case 74:
        set_idt_device(INT_ASM(74), 74);
        break;
    case 75:
        set_idt_device(INT_ASM(75), 75);
        break;
    case 76:
        set_idt_device(INT_ASM(76), 76);
        break;
    case 77:
        set_idt_device(INT_ASM(77), 77);
        break;
    case 78:
        set_idt_device(INT_ASM(78), 78);
        break;
    case 79:
        set_idt_device(INT_ASM(79), 79);
        break;
    case 80:
        set_idt_device(INT_ASM(80), 80);
        break;
    case 81:
        set_idt_device(INT_ASM(81), 81);
        break;
    case 82:
        set_idt_device(INT_ASM(82), 82);
        break;
    case 83:
        set_idt_device(INT_ASM(83), 83);
        break;
    case 84:
        set_idt_device(INT_ASM(84), 84);
        break;
    case 85:
        set_idt_device(INT_ASM(85), 85);
        break;
    case 86:
        set_idt_device(INT_ASM(86), 86);
        break;
    case 87:
        set_idt_device(INT_ASM(87), 87);
        break;
    case 88:
        set_idt_device(INT_ASM(88), 88);
        break;
    case 89:
        set_idt_device(INT_ASM(89), 89);
        break;
    case 90:
        set_idt_device(INT_ASM(90), 90);
        break;
    case 91:
        set_idt_device(INT_ASM(91), 91);
        break;
    case 92:
        set_idt_device(INT_ASM(92), 92);
        break;
    case 93:
        set_idt_device(INT_ASM(93), 93);
        break;
    case 94:
        set_idt_device(INT_ASM(94), 94);
        break;
    case 95:
        set_idt_device(INT_ASM(95), 95);
        break;
    case 96:
        set_idt_device(INT_ASM(96), 96);
        break;
    case 97:
        set_idt_device(INT_ASM(97), 97);
        break;
    case 98:
        set_idt_device(INT_ASM(98), 98);
        break;
    case 99:
        set_idt_device(INT_ASM(99), 99);
        break;
    case 100:
        set_idt_device(INT_ASM(100), 100);
        break;
    case 101:
        set_idt_device(INT_ASM(101), 101);
        break;
    case 102:
        set_idt_device(INT_ASM(102), 102);
        break;
    case 103:
        set_idt_device(INT_ASM(103), 103);
        break;
    case 104:
        set_idt_device(INT_ASM(104), 104);
        break;
    case 105:
        set_idt_device(INT_ASM(105), 105);
        break;
    case 106:
        set_idt_device(INT_ASM(106), 106);
        break;
    case 107:
        set_idt_device(INT_ASM(107), 107);
        break;
    case 108:
        set_idt_device(INT_ASM(108), 108);
        break;
    case 109:
        set_idt_device(INT_ASM(109), 109);
        break;
    case 110:
        set_idt_device(INT_ASM(110), 110);
        break;
    case 111:
        set_idt_device(INT_ASM(111), 111);
        break;
    case 112:
        set_idt_device(INT_ASM(112), 112);
        break;
    case 113:
        set_idt_device(INT_ASM(113), 113);
        break;
    case 114:
        set_idt_device(INT_ASM(114), 114);
        break;
    case 115:
        set_idt_device(INT_ASM(115), 115);
        break;
    case 116:
        set_idt_device(INT_ASM(116), 116);
        break;
    case 117:
        set_idt_device(INT_ASM(117), 117);
        break;
    case 118:
        set_idt_device(INT_ASM(118), 118);
        break;
    case 119:
        set_idt_device(INT_ASM(119), 119);
        break;
    case 120:
        set_idt_device(INT_ASM(120), 120);
        break;
    case 121:
        set_idt_device(INT_ASM(121), 121);
        break;
    case 122:
        set_idt_device(INT_ASM(122), 122);
        break;
    case 123:
        set_idt_device(INT_ASM(123), 123);
        break;
    case 124:
        set_idt_device(INT_ASM(124), 124);
        break;
    case 125:
        set_idt_device(INT_ASM(125), 125);
        break;
    case 126:
        set_idt_device(INT_ASM(126), 126);
        break;
    case 127:
        set_idt_device(INT_ASM(127), 127);
        break;
    case 128:
        set_idt_device(INT_ASM(128), 128);
        break;
    case 129:
        set_idt_device(INT_ASM(129), 129);
        break;
    case 130:
        set_idt_device(INT_ASM(130), 130);
        break;
    case 131:
        set_idt_device(INT_ASM(131), 131);
        break;
    case 132:
        set_idt_device(INT_ASM(132), 132);
        break;
    case 133:
        set_idt_device(INT_ASM(133), 133);
        break;
    case 134:
        set_idt_device(INT_ASM(134), 134);
        break;
    case 135:
        set_idt_device(INT_ASM(135), 135);
        break;
    case 136:
        set_idt_device(INT_ASM(136), 136);
        break;
    case 137:
        set_idt_device(INT_ASM(137), 137);
        break;
    case 138:
        set_idt_device(INT_ASM(138), 138);
        break;
    case 139:
        set_idt_device(INT_ASM(139), 139);
        break;
    case 140:
        set_idt_device(INT_ASM(140), 140);
        break;
    case 141:
        set_idt_device(INT_ASM(141), 141);
        break;
    case 142:
        set_idt_device(INT_ASM(142), 142);
        break;
    case 143:
        set_idt_device(INT_ASM(143), 143);
        break;
    case 144:
        set_idt_device(INT_ASM(144), 144);
        break;
    case 145:
        set_idt_device(INT_ASM(145), 145);
        break;
    case 146:
        set_idt_device(INT_ASM(146), 146);
        break;
    case 147:
        set_idt_device(INT_ASM(147), 147);
        break;
    case 148:
        set_idt_device(INT_ASM(148), 148);
        break;
    case 149:
        set_idt_device(INT_ASM(149), 149);
        break;
    case 150:
        set_idt_device(INT_ASM(150), 150);
        break;
    case 151:
        set_idt_device(INT_ASM(151), 151);
        break;
    case 152:
        set_idt_device(INT_ASM(152), 152);
        break;
    case 153:
        set_idt_device(INT_ASM(153), 153);
        break;
    case 154:
        set_idt_device(INT_ASM(154), 154);
        break;
    case 155:
        set_idt_device(INT_ASM(155), 155);
        break;
    case 156:
        set_idt_device(INT_ASM(156), 156);
        break;
    case 157:
        set_idt_device(INT_ASM(157), 157);
        break;
    case 158:
        set_idt_device(INT_ASM(158), 158);
        break;
    case 159:
        set_idt_device(INT_ASM(159), 159);
        break;
    case 160:
        set_idt_device(INT_ASM(160), 160);
        break;
    case 161:
        set_idt_device(INT_ASM(161), 161);
        break;
    case 162:
        set_idt_device(INT_ASM(162), 162);
        break;
    case 163:
        set_idt_device(INT_ASM(163), 163);
        break;
    case 164:
        set_idt_device(INT_ASM(164), 164);
        break;
    case 165:
        set_idt_device(INT_ASM(165), 165);
        break;
    case 166:
        set_idt_device(INT_ASM(166), 166);
        break;
    case 167:
        set_idt_device(INT_ASM(167), 167);
        break;
    case 168:
        set_idt_device(INT_ASM(168), 168);
        break;
    case 169:
        set_idt_device(INT_ASM(169), 169);
        break;
    case 170:
        set_idt_device(INT_ASM(170), 170);
        break;
    case 171:
        set_idt_device(INT_ASM(171), 171);
        break;
    case 172:
        set_idt_device(INT_ASM(172), 172);
        break;
    case 173:
        set_idt_device(INT_ASM(173), 173);
        break;
    case 174:
        set_idt_device(INT_ASM(174), 174);
        break;
    case 175:
        set_idt_device(INT_ASM(175), 175);
        break;
    case 176:
        set_idt_device(INT_ASM(176), 176);
        break;
    case 177:
        set_idt_device(INT_ASM(177), 177);
        break;
    case 178:
        set_idt_device(INT_ASM(178), 178);
        break;
    case 179:
        set_idt_device(INT_ASM(179), 179);
        break;
    case 180:
        set_idt_device(INT_ASM(180), 180);
        break;
    case 181:
        set_idt_device(INT_ASM(181), 181);
        break;
    case 182:
        set_idt_device(INT_ASM(182), 182);
        break;
    case 183:
        set_idt_device(INT_ASM(183), 183);
        break;
    case 184:
        set_idt_device(INT_ASM(184), 184);
        break;
    case 185:
        set_idt_device(INT_ASM(185), 185);
        break;
    case 186:
        set_idt_device(INT_ASM(186), 186);
        break;
    case 187:
        set_idt_device(INT_ASM(187), 187);
        break;
    case 188:
        set_idt_device(INT_ASM(188), 188);
        break;
    case 189:
        set_idt_device(INT_ASM(189), 189);
        break;
    case 190:
        set_idt_device(INT_ASM(190), 190);
        break;
    case 191:
        set_idt_device(INT_ASM(191), 191);
        break;
    case 192:
        set_idt_device(INT_ASM(192), 192);
        break;
    case 193:
        set_idt_device(INT_ASM(193), 193);
        break;
    case 194:
        set_idt_device(INT_ASM(194), 194);
        break;
    case 195:
        set_idt_device(INT_ASM(195), 195);
        break;
    case 196:
        set_idt_device(INT_ASM(196), 196);
        break;
    case 197:
        set_idt_device(INT_ASM(197), 197);
        break;
    case 198:
        set_idt_device(INT_ASM(198), 198);
        break;
    case 199:
        set_idt_device(INT_ASM(199), 199);
        break;
    case 200:
        set_idt_device(INT_ASM(200), 200);
        break;
    case 201:
        set_idt_device(INT_ASM(201), 201);
        break;
    case 202:
        set_idt_device(INT_ASM(202), 202);
        break;
    case 203:
        set_idt_device(INT_ASM(203), 203);
        break;
    case 204:
        set_idt_device(INT_ASM(204), 204);
        break;
    case 205:
        set_idt_device(INT_ASM(205), 205);
        break;
    case 206:
        set_idt_device(INT_ASM(206), 206);
        break;
    case 207:
        set_idt_device(INT_ASM(207), 207);
        break;
    case 208:
        set_idt_device(INT_ASM(208), 208);
        break;
    case 209:
        set_idt_device(INT_ASM(209), 209);
        break;
    case 210:
        set_idt_device(INT_ASM(210), 210);
        break;
    case 211:
        set_idt_device(INT_ASM(211), 211);
        break;
    case 212:
        set_idt_device(INT_ASM(212), 212);
        break;
    case 213:
        set_idt_device(INT_ASM(213), 213);
        break;
    case 214:
        set_idt_device(INT_ASM(214), 214);
        break;
    case 215:
        set_idt_device(INT_ASM(215), 215);
        break;
    case 216:
        set_idt_device(INT_ASM(216), 216);
        break;
    case 217:
        set_idt_device(INT_ASM(217), 217);
        break;
    case 218:
        set_idt_device(INT_ASM(218), 218);
        break;
    case 219:
        set_idt_device(INT_ASM(219), 219);
        break;
    case 220:
        set_idt_device(INT_ASM(220), 220);
        break;
    case 221:
        set_idt_device(INT_ASM(221), 221);
        break;
    case 222:
        set_idt_device(INT_ASM(222), 222);
        break;
    case 223:
        set_idt_device(INT_ASM(223), 223);
        break;
    case 224:
        set_idt_device(INT_ASM(224), 224);
        break;
    case 225:
        set_idt_device(INT_ASM(225), 225);
        break;
    case 226:
        set_idt_device(INT_ASM(226), 226);
        break;
    case 227:
        set_idt_device(INT_ASM(227), 227);
        break;
    case 228:
        set_idt_device(INT_ASM(228), 228);
        break;
    case 229:
        set_idt_device(INT_ASM(229), 229);
        break;
    case 230:
        set_idt_device(INT_ASM(230), 230);
        break;
    case 231:
        set_idt_device(INT_ASM(231), 231);
        break;
    case 232:
        set_idt_device(INT_ASM(232), 232);
        break;
    case 233:
        set_idt_device(INT_ASM(233), 233);
        break;
    case 234:
        set_idt_device(INT_ASM(234), 234);
        break;
    case 235:
        set_idt_device(INT_ASM(235), 235);
        break;
    case 236:
        set_idt_device(INT_ASM(236), 236);
        break;
    case 237:
        set_idt_device(INT_ASM(237), 237);
        break;
    case 238:
        set_idt_device(INT_ASM(238), 238);
        break;
    case 239:
        set_idt_device(INT_ASM(239), 239);
        break;
    case 240:
        set_idt_device(INT_ASM(240), 240);
        break;
    case 241:
        set_idt_device(INT_ASM(241), 241);
        break;
    case 242:
        set_idt_device(INT_ASM(242), 242);
        break;
    case 243:
        set_idt_device(INT_ASM(243), 243);
        break;
    case 244:
        set_idt_device(INT_ASM(244), 244);
        break;
    case 245:
        set_idt_device(INT_ASM(245), 245);
        break;
    case 246:
        set_idt_device(INT_ASM(246), 246);
        break;
    case 247:
        set_idt_device(INT_ASM(247), 247);
        break;
    case 248:
        set_idt_device(INT_ASM(248), 248);
        break;
    case 249:
        set_idt_device(INT_ASM(249), 249);
        break;
    case 250:
        set_idt_device(INT_ASM(250), 250);
        break;
    case 251:
        set_idt_device(INT_ASM(251), 251);
        break;
    case 252:
        set_idt_device(INT_ASM(252), 252);
        break;
    case 253:
        set_idt_device(INT_ASM(253), 253);
        break;
    case 254:
        set_idt_device(INT_ASM(254), 254);
        break;
    case 255:
        set_idt_device(INT_ASM(255), 255);
        break;
    default:
        return -1;
    }
    return 0;
}
