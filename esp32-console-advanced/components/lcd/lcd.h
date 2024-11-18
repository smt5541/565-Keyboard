/**
 * 2x20 LCD Driver
 * @author Seth Teichman
 */
#pragma once
#include <stdbool.h>
#include "vector.h"

typedef struct lcd_t {
  /** The dimensions of the LCD */
  vector2_int_t dimensions;

  /** The current display data, used for debugging */
  char** displayBuffer;

  /** The current cursor position of the cursor, 0 indexed */
  vector2_int_t cursorPos;

  /** Whether prints automatically wrap to the next row */
  bool autoWrap;
} lcd_t;

/**
 * Initialize the LCD Module
 * @param self The configuration to initialize with
 */
bool lcd_init(lcd_t *self);

/**
 * Set the cursor position, 0 indexed
 * @param self The LCD instance to set the cursor position of
 * @param x The cursor column position, 0 indexed
 * @param y The cursor row position, 0 indexed
 */
void lcd_set_cursor_pos(lcd_t *self, int x, int y);

/**
 * Dump the LCD Display Buffer
 */
void lcd_dump_buffer(lcd_t *self);

/**
 * Free the LCD Module
 * @param self The instance to free
 */
void lcd_free(lcd_t *self);

