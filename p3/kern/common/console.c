/** @file console.c
 *  @brief Implementation of the console device driver
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 */

#include <video_defines.h>
#include <stdint.h>
#include <console.h>
#include <asm.h>
#include <simics.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

// Console macros
/** @brief Gets the memory address of a character at a console position
 *  @param row Row of the char
 *  @param col Column of the char
 *  @return Memory address of the character at the given position
 **/
#define GET_CHR(row, col) \
    (CONSOLE_MEM_BASE + 2 * (((row) * CONSOLE_WIDTH) + (col)))
/** @brief Gets the memory address of a color at a console position
 *  @param row Row of the char
 *  @param col Column of the char
 *  @return Memory address of the color at the given position
 **/
#define GET_COLOR(row, col) (GET_CHR(row, col) + 1)
/** @brief Returns the address of the maximum line to copy when scrolling
 *  @return Address of the last line to copy
 **/
#define COPY_LINES_MAX GET_CHR(CONSOLE_HEIGHT - 1, 0)
/** @brief Returns the address of the last line of the console
 *  @return Address of the last line
 **/
#define END_CONSOLE GET_CHR(CONSOLE_HEIGHT, 0)
/** @brief Gets the upper 8 bits of a value
 *  @param val Value whose upper 8 bits is to be retrieved
 *  @return Upper 8 bits of the given value
 **/
#define GET_MSB(val) ((val) >> 8)
/** @brief Gets the lower 8 bits of a value
 *  @param val Value whose lower 8 bits is to be retrieved
 *  @return Lower 8 bits of the given value
 **/
#define GET_LSB(val) ((val) & 0xFF)
/** @brief Gets the address of the same column on the previous row
 *  @param val Current console address
 *  @return Previous console address
 **/
#define GET_PREV_LINE(val) ((val) + LINE_SIZE)
/** @brief Returns the size of a single console line in memory
 *  @return The size of a line on the console
 **/
#define LINE_SIZE (2 * CONSOLE_WIDTH)
/** @brief The value above which all colors are invalid in the color scheme
 **/
#define INVALID_COLOR 0x90

// Global variables
/** @brief Global console color */
static int global_color = FGND_WHITE | BGND_BLACK;
/** @brief Global variable to determine if cursor should be hidden */
static int cursor_hidden = 0;
/** @brief Global cursor row position */
static int cursor_row = 0;
/** @brief Global cursor column position */
static int cursor_col = 0;

/** @brief Is the row and colum pair provided a valid cursor position
 *
 *  @param row The row of the cursor
 *  @param col The column of the cursor
 *  @return a boolean integer
 **/
int cursor_valid(int row, int col)
{
    if (row < 0 || row >= CONSOLE_HEIGHT)
        return 0;
    if (col < 0 || col >= CONSOLE_WIDTH)
        return 0;
    return 1;
}

/** @brief Sets the hardware cursor to the given coordinate position.
 *
 *  Pointer can be set to a location off-screen to hide the cursor.
 *
 *  @param row Console row to set hardware cursor.
 *  @param col Console column to set hardware cursor.
 *  @return Void
 */
void set_cursor_hardware(int row, int col)
{
    // Calculate cursor position
    uint16_t position = (row * CONSOLE_WIDTH) + col;
    uint8_t MSB = GET_MSB(position);
    uint8_t LSB = GET_LSB(position);

    // Set MSB of cursor position
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, MSB);

    // Set LSB of cursor position
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, LSB);
}

/** @brief Scrolls the console display down by 1 line.
 *
 *  Copies the lower 79 lines of display up by 1 and clears the bottom line.
 *
 *  @return Void
 */
void scroll()
{
    int i;
    // Copy lines up by one
    for (i = CONSOLE_MEM_BASE; i < COPY_LINES_MAX; i += LINE_SIZE) {
        memcpy((void *)i, (void *)GET_PREV_LINE(i), LINE_SIZE);
    }
    // Blank out last line
    for (i = COPY_LINES_MAX; i < END_CONSOLE; i += 2) {
        *(char*)i = ' ';
    }
}

/** @brief Prints character ch at the current location
 *         of the cursor.
 *
 *  If the character is a newline, the cursor is
 *  be moved to the beginning of the next line (scrolling if necessary).
 *  If the character is a carriage return, the cursor
 *  is immediately reset to the beginning of the current
 *  line, causing any future output to overwrite any existing
 *  output on the line.  If backspace is encountered,
 *  the previous character is erased.  See the main console.c
 *  description for more backspace behavior.
 *
 *  @param ch the character to print
 *  @return The input character
 */
int putbyte(char ch)
{
    int row, col;

    // Get current cursor position
    get_cursor(&row, &col);

    switch (ch) {

    // Newline: Move cursor to start of next line, scrolling if necessary
    case '\n':
        if (row == CONSOLE_HEIGHT - 1)
            scroll();
        else
            row++;
        col = 0;
        break;

    // Carriage return: Reset cursor to beginning of current line
    case '\r':
        col = 0;
        break;

    /* Backspace: Erase previous character. If the cursor is already
       at the beginning of the screen nothing occurs. */
    case '\b':
        if (col != 0) {
            col--;
            *(char*)(GET_CHR(row, col)) = ' ';
        } else if (row != 0) {
            row--;
            col = CONSOLE_WIDTH - 1;
            *(char*)(GET_CHR(row, col)) = ' ';
        }
        break;

    // Standard character: Print at the current cursor location
    default:
        // Print character
        *(char*)(GET_CHR(row, col)) = ch;
        *(char*)(GET_COLOR(row, col)) = global_color;

        // Move cursor to the next position
        col++;
        if (col >= CONSOLE_WIDTH) {
            col = 0;
            if (row == CONSOLE_HEIGHT - 1)
                scroll();
            else
                row++;
        }
        break;
    }

    assert(set_cursor(row, col) == 0);

    return ch;
}

/** @brief Prints the string s, starting at the current
 *         location of the cursor.
 *
 *  If the string is longer than the current line, the
 *  string fills up the current line and then
 *  continues on the next line. If the string exceeds
 *  available space on the entire console, the screen
 *  scrolls up one line, and then the string continues
 *  on the new line.  If newline, carriage return, or
 *  backspace are encountered within the string, they
 *  are handled as per putbyte. If len is not a positive
 *  integer or is null, the function has no effect.
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *  @return Void.
 */
void putbytes(const char* s, int len)
{
    int i = 0;

    // Input validity check
    if ((s == NULL) || (len <= 0))
        return;



    int cursor_was_hidden = cursor_hidden;
    if(!cursor_was_hidden){
        hide_cursor();
    }

    for (i = 0; i < len; i++) {
        putbyte(s[i]);
    }

    if(!cursor_was_hidden){
        show_cursor();
    }
}

/** @brief Changes the foreground and background color
 *         of future characters printed on the console.
 *
 *  If the color code is invalid, the function has no effect.
 *
 *  @param color The new color code.
 *  @return 0 on success or integer error code less than 0 if
 *          color code is invalid.
 */
int set_term_color(int color)
{
    if (color >= INVALID_COLOR) {
        return -1;
    }
    global_color = color;
    return 0;
}

/** @brief Writes the current foreground and background
 *         color of characters printed on the console
 *         into the argument color.
 *  @param color The address to which the current color
 *         information will be written.
 *  @return Void.
 */
void get_term_color(int* color)
{
    if (color == NULL)
        return;
    *color = global_color;
}

/** @brief Sets the position of the cursor to the
 *         position (row, col).
 *
 *  Subsequent calls to putbytes should cause the console
 *  output to begin at the new position. If the cursor is
 *  currently hidden, a call to set_cursor() does not show
 *  the cursor.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *  @return 0 on success or integer error code less than 0 if
 *          cursor location is invalid.
 */
int set_cursor(int row, int col)
{
    // Check if the cursor location given is valid
    if (!cursor_valid(row, col))
        return -1;

    // Update the global position
    cursor_row = row;
    cursor_col = col;
    // Update actual hardware position is cursor is shown
    if (!cursor_hidden) {
        set_cursor_hardware(row, col);
    }
    return 0;
}

/** @brief Writes the current position of the cursor
 *         into the arguments row and col.
 *  @param row The address to which the current cursor
 *         row will be written.
 *  @param col The address to which the current cursor
 *         column will be written.
 *  @return Void.
 */
void get_cursor(int* row, int* col)
{
    if ((row == NULL) || (col == NULL))
        return;

    *row = cursor_row;
    *col = cursor_col;
}

/** @brief Hides the cursor.
 *
 *  Subsequent calls to putbytes do not cause the
 *  cursor to show again.
 *
 *  @return Void.
 */
void hide_cursor()
{
    // Do nothing if cursor already hidden
    if (cursor_hidden)
        return;

    // Move cursor position offscreen
    set_cursor_hardware(CONSOLE_HEIGHT, CONSOLE_WIDTH);

    cursor_hidden = 1;
}

/** @brief Shows the cursor.
 *
 *  If the cursor is already shown, the function has no effect.
 *
 *  @return Void.
 */
void show_cursor()
{
    // Do nothing if cursor already shown
    if (!cursor_hidden)
        return;

    // Restore cursor position onscreen from global variables
    set_cursor_hardware(cursor_row, cursor_col);

    cursor_hidden = 0;
}

/** @brief Clears the entire console.
 *
 * The cursor is reset to the first row and column
 *
 *  @return Void.
 */
void clear_console()
{
    int i;

    // Clear entire console screen
    for (i = CONSOLE_MEM_BASE; i < END_CONSOLE; i += 2) {
        *(char*)i = ' ';
        *(char*)(i + 1) = global_color;
    }

    // Reset cursor position
    assert(set_cursor(0, 0) == 0);
}

/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *  @return Void.
 */
void draw_char(int row, int col, int ch, int color)
{
    if (!cursor_valid(row, col))
        return;

    // Display char with given color at given location
    *(char*)(GET_CHR(row, col)) = ch;
    *(char*)(GET_COLOR(row, col)) = color;
}

/** @brief Returns the character displayed at position (row, col).
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The character at (row, col).
 */
char get_char(int row, int col)
{
    // Check validity of inputs
    if (!cursor_valid(row, col))
        return 0;

    return *(char*)(GET_CHR(row, col));
}
