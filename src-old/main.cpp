#include "mbed.h"
#include <TextLCD.h>
#include <cstdio>
#include <string>
#include "MagStripe.h"

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
BufferedSerial pc(USBTX, USBRX);

DigitalOut led_wait(PB_13);
DigitalOut led_ofln(PB_14);
DigitalOut led_msgp(PB_15);
DigitalOut led_blnk(PB_0);
DigitalOut leds[4] = {led_wait, led_ofln, led_msgp, led_blnk};
bool led_state[4] = {1, 1, 1, 1};

TextLCD lcd(PA_15, PC_10, PA_10, PC_6, PA_2, PA_3, TextLCD::LCD20x2);

MagStripe msr = MagStripe();
static const int DATA_BUFFER_LEN = 108;
static char data[DATA_BUFFER_LEN];

void ser_print(string s) {
    pc.write(s.c_str(), s.length());
}

void lcd_init() {
    lcd.setCursor(TextLCD::CurOff_BlkOff);  
    lcd.printf("Hello World!");
}

void led_init() {
    led_wait.write(1);
    led_ofln.write(1);
    led_msgp.write(1);
    led_blnk.write(1);
    
}

void set_led(int id, bool on) {
    if (on) {
        led_state[id] = 0;
        leds[id].write(0);
    } else {
        led_state[id] = 1;
        leds[id].write(1);
    }
}

void toggle_led(int id) {
    if (led_state[id] == 0)
        led_state[id] = 1;
    else
        led_state[id] = 0;
    leds[id].write(led_state[id]);
}


void ser_key_process(char *buf) {
    string s;
    if (pc.read(buf, 1) != -EAGAIN) {
        switch (buf[0]) {
            case '1':
            toggle_led(0);
            break;
            case '2':
            toggle_led(1);
            break;
            case '3':
            toggle_led(2);
            break;
            case '4':
            toggle_led(3);
            break;
        }
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

/**
 * The loop function for keymatrix handling
**/
void kb_key_check() {
    for (int c = 0; c < KB_COLS; c++) { //Iterate through columns
        kb_cols[c].write(1); //Turn on this column
        for (int r = 0; r < KB_ROWS; r++) { //Iterate through rows

            //Logic to handle the fact that numpad 0 is actually two keys:
            //By turning the second column on, we avoid catching the key in one column but not the other
            if (r == 2 && c == 5) { //If this is the left side of the numpad 0
                kb_cols[6].write(1); //Turn on the right side of the numpad 0
            }
            if (r == 2 && c == 6) { //If this is the right side of the numpad 0
                kb_cols[5].write(1); //Turn on the left side of the numpad 0
            }

            if (kb_rows[r]) { //If this row is on
                if (!kb_pressed[r][c]) { //If this key was not previously pressed
                    ser_print("C"+kb_col_ids[c]+"R"+kb_row_ids[r]+" Pressed\n");
                }
                
                kb_pressed[r][c] = true; //Flag this key as pressed
            } else { //If this row is not on
                if (kb_pressed[r][c]) { //If this key was previously pressed
                    ser_print("C"+kb_col_ids[c]+"R"+kb_row_ids[r]+" Released\n");
                }
                
                kb_pressed[r][c] = false; //Flag this key as not pressed
            }

            //Logic to reset to normal state after special numpad 0 logic above
            if (r == 2 && c == 5) { //If this is the left side of the numpad 0
                kb_cols[6].write(0); //Turn off the right side of the numpad 0
            }
            if (r == 2 && c == 6) { //If this is the right side of the numpad 0
                kb_cols[5].write(0); //Turn off the left side of the numpad 0
            }
        }
        kb_cols[c].write(0); //Turn off this column
    }
}

void msr_read() {
    if (!msr.available()) {
        data[0] = '\0';
        return;
    }
    set_led(3, true);

    short chars = msr.read(data, DATA_BUFFER_LEN);

    set_led(3, false);

    if (chars < 0) {
        ser_print("Error Reading Card\n");
        return;
    }

    ser_print("Card Read Success! ");
    pc.write(data, chars);
    ser_print("\n");
}

// main() runs in its own thread in the OS
int main()
{
    pc.set_blocking(false);
    msr.begin(2);
    lcd_init();
    led_init();
    set_led(0, true);
    ThisThread::sleep_for(1s);
    set_led(0, false);
    while (true) {
        char buf[1];
        ser_key_process(buf);
        lcd_display(buf);
        kb_key_check();
        msr_read();
    }
}



