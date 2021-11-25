#include "mbed.h"
#include "BLE.h"
#include <TextLCD.h>
#include <cstdio>
#include <string>

#define KB_ROWS 6
#define KB_COLS 11

DigitalIn row_1(PC_2, PullDown);
DigitalIn row_2(PC_3, PullDown);
DigitalIn row_3(PA_0, PullDown);
DigitalIn row_4(PA_1, PullDown);
DigitalIn row_5(PC_1, PullDown);
DigitalIn row_16(PC_0, PullDown);
DigitalIn kb_rows[KB_ROWS] = {row_1, row_2, row_3, row_4, row_5, row_16};
string kb_row_ids[KB_ROWS] = {"1", "2", "3", "4", "5", "16"};

DigitalOut col_5(PB_4);
DigitalOut col_6(PB_8);
DigitalOut col_7(PB_9);
DigitalOut col_8(PA_5);
DigitalOut col_9(PA_6);
DigitalOut col_10(PA_7);
DigitalOut col_11(PA_4);
DigitalOut col_12(PA_9);
DigitalOut col_13(PC_12);
DigitalOut col_14(PC_13);
DigitalOut col_15(PA_8);
DigitalOut kb_cols[KB_COLS] = {col_5, col_6, col_7, col_8, col_9, col_10, col_11, col_12, col_13, col_14, col_15};
string kb_col_ids[KB_COLS] = {"5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"};

bool kb_pressed[KB_ROWS][KB_COLS];
static bool ser_key_read = false;
static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);
BufferedSerial pc(USBTX, USBRX);
DigitalOut led_blue(LED1);
DigitalOut led_green(LED2);
DigitalOut led_red(LED3);
TextLCD lcd(PA_15, PC_10, PA_10, PC_6, PA_2, PA_3, TextLCD::LCD20x2);
static const int DATA_BUFFER_LEN = 108;
static char data[DATA_BUFFER_LEN];

void ser_print(string s) {
    pc.write(s.c_str(), s.length());
}

void lcd_init() {
    lcd.setCursor(TextLCD::CurOff_BlkOff);  
    lcd.printf("Hello World!");
    col_13.write(1);
}

void ser_key_process(char *buf) {
    string s;
    if (pc.read(buf, 1) != -EAGAIN) {
        s = to_string((int)buf[0]);
        s.append("\r\n");
        ser_print(s);
        ser_key_read = true;
    } else {
        ser_key_read = false;
    }
}

void lcd_display(char *buf) {
    if (ser_key_read) {
        if (buf[0] != '\r') {
            lcd.putc(buf[0]);
        } else {
            lcd.cls();
        }
    }
}

void kb_key_check() {
    for (int c = 0; c < KB_COLS; c++) {
        kb_cols[c].write(1);
        for (int r = 0; r < KB_ROWS; r++) {
            if (r == 2 && c == 5) {
                kb_cols[6].write(1);
            }
            if (r == 2 && c == 6) {
                kb_cols[5].write(1);
            }

            if (kb_rows[r]) {
                if (!kb_pressed[r][c]) {
                    ser_print("C"+kb_col_ids[c]+"R"+kb_row_ids[r]+" Pressed\n");
                }
                
                kb_pressed[r][c] = true;
            } else {
                if (kb_pressed[r][c]) {
                    ser_print("C"+kb_col_ids[c]+"R"+kb_row_ids[r]+" Released\n");
                }
                
                kb_pressed[r][c] = false;
            }

            if (r == 2 && c == 5) {
                kb_cols[6].write(0);
            }
            if (r == 2 && c == 6) {
                kb_cols[5].write(0);
            }
        }
        kb_cols[c].write(0);
    }
}

// main() runs in its own thread in the OS
int main()
{
    pc.set_blocking(false);
    lcd_init();
    ThisThread::sleep_for(1s);
    while (true) {
        char buf[1];
        ser_key_process(buf);
        lcd_display(buf);
        kb_key_check();
    }
}



