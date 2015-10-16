/** @file fault.h
 *
 *  @brief Function header for fault handlers
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef _FAULT_H
#define _FAULT_H

#include <stdint.h>

/** @def CONSTRUCT_HANDLER_H(NAME)
 *
 *  @brief Constructs the assembly wrapper function declaration
 *
 *  @param NAME Fault name
 *  @return Void
 **/
#define CONSTRUCT_HANDLER_H(NAME) \
    void NAME ##_handler_asm()
    
/** @def CONSTRUCT_HANDLER_C(NAME)
 *
 *  @brief Constructs the C handler function name
 *
 *  @param NAME Fault name
 *  @return Void
 **/
#define CONSTRUCT_HANDLER_C(NAME) \
    void NAME ##_handler()

#define GET_LOW(ADDR)  ((int)ADDR & 0xFF)
#define GET_HIGH(ADDR) ((int)ADDR >> 8)

/** @brief Struct for Interrupt Descriptor Table (IDT) entries 
 */
typedef struct {
  uint16_t offset_low;
  uint16_t segment;
  uint8_t reserved;
  uint8_t fixed1 : 3;
  uint8_t D : 1;
  uint8_t fixed2 : 1;
  uint8_t DPL : 2;
  uint8_t P : 1;
  uint16_t offset_high;
} IDT_entry;

// fault_asm.S headers
/** @brief Wrapper for the divide error handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(divide);

/** @brief Wrapper for the debug exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(debug);

/** @brief Wrapper for the NMI exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(nmi);

/** @brief Wrapper for the breakpoint exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(breakpoint);

/** @brief Wrapper for the overflow exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(overflow);

/** @brief Wrapper for the BOUND exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(bound);

/** @brief Wrapper for the invalid opcode exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(opcode);

/** @brief Wrapper for the no math coprocessor exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(no_math);

/** @brief Wrapper for the double fault exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(double_fault);

/** @brief Wrapper for the coprocessor segment overrun exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(cso);

/** @brief Wrapper for the invalid task segment selector exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(tss);

/** @brief Wrapper for the segment not present exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(not_present);

/** @brief Wrapper for the stack segment fault exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(stack);

/** @brief Wrapper for the general protection exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(protection);

/** @brief Wrapper for the page fault exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(page);

/** @brief Wrapper for the x87 FPU floating point error exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(math);

/** @brief Wrapper for the alignment check exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(alignment);

/** @brief Wrapper for the machine check exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(machine);

/** @brief Wrapper for the SIMD floating point exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(fpu);

// fault.c headers
int handler_install();
void set_idt(void *handler, uint16_t segment, uint8_t privilege, int index);
void install_idt(int index, IDT_entry* entry);

CONSTRUCT_HANDLER_C(divide);
CONSTRUCT_HANDLER_C(debug);
CONSTRUCT_HANDLER_C(nmi);
CONSTRUCT_HANDLER_C(breakpoint);
CONSTRUCT_HANDLER_C(overflow);
CONSTRUCT_HANDLER_C(bound);
CONSTRUCT_HANDLER_C(opcode);
CONSTRUCT_HANDLER_C(no_math);
CONSTRUCT_HANDLER_C(double_fault);
CONSTRUCT_HANDLER_H(cso);
CONSTRUCT_HANDLER_H(tss);
CONSTRUCT_HANDLER_H(not_present);
CONSTRUCT_HANDLER_H(stack);
CONSTRUCT_HANDLER_H(protection);
CONSTRUCT_HANDLER_H(page);
CONSTRUCT_HANDLER_H(math);
CONSTRUCT_HANDLER_H(alignment);
CONSTRUCT_HANDLER_H(machine);
CONSTRUCT_HANDLER_H(fpu);

#endif /* _FAULT_H */