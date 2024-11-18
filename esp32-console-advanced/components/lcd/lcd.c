/**
 * 2x20 LCD Driver
 * @author Seth Teichman
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "math_ext.h"
#include "lcd.h"
#include "esp_log.h"

static const char *TAG = "lcd";

bool lcd_init(lcd_t *self) {
  self->displayBuffer = (char **) malloc(self->dimensions.y * sizeof(char*));
  for (int y = 0; y < self->dimensions.y; y++) {
    self->displayBuffer[y] = (char*) malloc(self->dimensions.x * sizeof(char));
    for (int x = 0; x < self->dimensions.x; x++) {
      self->displayBuffer[y][x] = ' ';
    }
  }
  lcd_set_cursor_pos(self, 0, 0);
  self->autoWrap = true;
  return true;
}

void lcd_set_cursor_pos(lcd_t *self, int x, int y) {
  self->cursorPos.x = MATH_EXT_CLAMP(x, 0, self->dimensions.x - 1);
  self->cursorPos.y = MATH_EXT_CLAMP(y, 0, self->dimensions.y - 1);
}

void lcd_dump_buffer(lcd_t *self) {
  if (self->displayBuffer == NULL) return;
  ESP_LOGI(TAG, "[Begin LCD Display Buffer]");
  for (int y = 0; y < self->dimensions.y; y++) {
      ESP_LOGI(TAG, "%.*s", self->dimensions.x, self->displayBuffer[y]);
  }
  ESP_LOGI(TAG, "[End LCD Display Buffer]");
}

void lcd_free(lcd_t *self) {
  for (int y = 0; y < self->dimensions.y; y++) {
    free(self->displayBuffer[y]);
    self->displayBuffer[y] = NULL;
  }
  free(self->displayBuffer);
  self->displayBuffer = NULL;
}