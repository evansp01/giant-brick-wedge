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

// Console macros
#define GET_CHR(row, col) (CONSOLE_MEM_BASE+2*((row*CONSOLE_WIDTH)+col))
#define GET_COLOR(row, col) (CONSOLE_MEM_BASE+2*((row*CONSOLE_WIDTH)+col)+1)
#define COPY_LINES_MAX (CONSOLE_MEM_BASE+2*((CONSOLE_HEIGHT-1)*CONSOLE_WIDTH))
#define END_CONSOLE (CONSOLE_MEM_BASE+2*(CONSOLE_HEIGHT*CONSOLE_WIDTH))
#define GET_MSB(val) (val >> 8)
#define GET_LSB(val) (val & 0xFF)
#define GET_POS(MSB, LSB) ((MSB << 8)|LSB)
#define GET_ROW(val) (val / CONSOLE_WIDTH)
#define GET_COL(val) (val % CONSOLE_WIDTH)
#define INVALID_COLOR 0x90

// Global variables
static int global_color = FGND_WHITE | BGND_BLACK;
static int cursor_hidden = 0;
static int cursor_row = 0;
static int cursor_col = 0;

/** @brief Sets the hardware cursor to the given coordinate position.
 *
 *  Pointer can be set to a location off-screen to hide the cursor.
 *
 *  @param row Console row to set hardware cursor.
 *  @param col Console column to set hardware cursor.
 *  @return Void
 */
void set_cursor_hardware( int row, int col )
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

/** @brief Retrieves the coordinate position of the hardware cursor.
 *
 *  @param row Pointer to store row of hardware cursor.
 *  @param col Pointer to store column of hardware cursor.
 *  @return Void
 */
void get_cursor_hardware( int *row, int *col )
{
  // Get MSB of cursor position
  outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
  uint8_t MSB = inb(CRTC_DATA_REG);
  
  // Get LSB of cursor position
  outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
  uint8_t LSB = inb(CRTC_DATA_REG);

  // Calculate cursor position
  uint16_t position = GET_POS(MSB, LSB);
  *row = GET_ROW(position);
  *col = GET_COL(position);
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
  /* Copy lines up by one */
  for (i = CONSOLE_MEM_BASE; i < COPY_LINES_MAX; i+=2) {
    *(char *)i = *(char *)(i+(2*CONSOLE_WIDTH)); 
    *(char *)(i+1) = *(char *)(i+(2*CONSOLE_WIDTH)+1);
  }
  /* Blank out last line */
  for (i = COPY_LINES_MAX; i < END_CONSOLE; i+=2) {
    *(char *)i = ' ';
  }
}

int putbyte( char ch )
{
  int row, col;
  
  // Get current cursor position
  get_cursor(&row, &col);
  
  switch (ch) {
    
    // Newline: Move cursor to start of next line, scrolling if necessary
    case '\n':
      if (row == CONSOLE_HEIGHT-1)
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
       at the beginning of the line nothing occurs. */
    case '\b':
      // TODO: backspace should backup lines, or readline buffer shorter
      if (col != 0) {
        col--;
        *(char *)(GET_CHR(row, col)) = ' ';
      }
      break;
    
    // Standard character: Print at the current cursor location
    default:
      // Print character
      *(char *)(GET_CHR(row, col)) = ch; 
      *(char *)(GET_COLOR(row, col)) = global_color;
      
      // Move cursor to the next position
      col++;
      if (col >= CONSOLE_WIDTH) {
        col = 0;
        if (row == CONSOLE_HEIGHT-1)
          scroll();
        else
          row++;
      }
      break;
  }
  
  assert(set_cursor(row, col) == 0);
  
  return ch;
}

void putbytes( const char *s, int len )
{
  int i = 0;
  
  // Input validity check
  if ((s == NULL) || (len <= 0))
    return;
  
  for (i = 0; i < len; i++) {
    putbyte(s[i]);
  }
}

int set_term_color( int color )
{
    if (color >= INVALID_COLOR) {
        return -1;
    }
  global_color = color;
  return 0;
}

void get_term_color( int *color )
{  
  if (color == NULL)
    return;
  *color = global_color;
}

int set_cursor( int row, int col )
{
  // Check if the cursor location given is valid
  if ((row < 0)||(row >= CONSOLE_HEIGHT)||(col < 0)||(col >= CONSOLE_WIDTH))
    return -1;
  
  // Update the global position if cursor is hidden
  if (cursor_hidden) {
    cursor_row = row;
    cursor_col = col;
  }
  // Update actual hardware position is cursor is shown
  else
    set_cursor_hardware(row, col);
  
  return 0;
}

void get_cursor( int *row, int *col )
{ 
  if ((row == NULL)||(col == NULL))
    return;

  *row = 0;
  *col = 0;
  
  // Return stored values if cursor is hidden
  if (cursor_hidden) {
    *row = cursor_row;
    *col = cursor_col;
  }
  // Return actual hardware values if cursor is shown
  else
    get_cursor_hardware(row, col);
}

void hide_cursor()
{
  // Do nothing if cursor already hidden
  if (cursor_hidden)
    return;
  
  // Store current cursor position in global variables
  get_cursor_hardware(&cursor_row, &cursor_col);
  
  // Move cursor position offscreen
  set_cursor_hardware(CONSOLE_HEIGHT, CONSOLE_WIDTH);
  
  cursor_hidden = 1;
}

void show_cursor()
{
  // Do nothing if cursor already shown
  if (!cursor_hidden)
    return;
  
  // Restore cursor position onscreen from global variables
  set_cursor_hardware(cursor_row, cursor_col);
  
  cursor_hidden = 0;
}

void clear_console()
{
  int i;
  
  // Clear entire console screen
  for (i = CONSOLE_MEM_BASE; i < END_CONSOLE; i+=2) {
    *(char *)i = ' ';
    *(char *)(i+1) = FGND_BLACK|BGND_BLACK;
  }
  
  // Reset cursor position
  if (set_cursor(0, 0) != 0)
    lprintf("Error: Cursor location is invalid");
}

void draw_char( int row, int col, int ch, int color )
{
  // Check validity of inputs
  if ((row < 0)||(row >= CONSOLE_HEIGHT)||(col < 0)||(col >= CONSOLE_WIDTH))
    return;
  
  // Display char with given color at given location
  *(char *)(GET_CHR(row, col)) = ch;
  *(char *)(GET_COLOR(row, col)) = color;
}

char get_char( int row, int col )
{
  // Check validity of inputs
  if ((row < 0)||(row >= CONSOLE_HEIGHT)||(col < 0)||(col >= CONSOLE_WIDTH))
    return 0;
  
  return *(char *)(GET_CHR(row, col));
}
