#include <asm.h>
#include <ns16550.h>

#define BAUD_RATE 115200

#define LSB(val) GET_BYTE(val, 0)
#define MSB(val) GET_BYTE(val, 1)


void write_port(port_t port, reg_t reg, int value)
{
    outb(port + reg, value);
}

int read_port(port_t port, reg_t reg)
{
    return inb(port + reg)
}

void setup_serial_driver(port_t serial, int int_enable_flags)
{
    int rate = UART_CLOCK / BAUD_RATE;
    write_port(serial, REG_LINE_CNTL, LCR_DLAB);
    write_port(serial, REG_BAUD_LSB, LSB(rate));
    write_port(serial, REG_BAUD_MSB, MSB(rate));
    // setup the line control registry
    write_port(serial, REG_LINE_CNTL, CONF_8N1);
    // enable interrupts from the serial driver
    write_port(serial, REG_INT_EN, int_enable_flags);
}
