/** @file get_esp.S
 *  
 *  @brief Contains the get_esp assembly function
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 **/

.global get_esp
get_esp:
    movl %esp, %eax
    ret

