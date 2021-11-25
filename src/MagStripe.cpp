// #include "MagStripe.h"

// #include <ctime>
// #include <mbed.h>

// BufferedSerial ser(USBTX, USBRX);

// //Enough bits to store the data from any of the three tracks
// #define BIT_BUFFER_LEN 800

// //Variables for use by interrupt handlers
// static volatile bool next_bit = 0;
// static volatile unsigned char bits[BIT_BUFFER_LEN/8];
// static volatile short num_bits = 0;

// //Input pins
// DigitalIn p_rdt(MAGSTRIPE_DATA);
// DigitalIn p_rcl(MAGSTRIPE_CLOCK);
// DigitalIn p_cls(MAGSTRIPE_LOAD);

// //Functions to manipulate the bit buffer
// static void bits_set(short index, bool bit);
// static bool bits_get(short index);

// //Interrupt pins
// InterruptIn data_interrupt(MAGSTRIPE_DATA);
// InterruptIn clock_interrupt(MAGSTRIPE_CLOCK);

// //Interrupt handlers
// static void handle_data(void);
// static void handle_clock(void);

// MagStripe::MagStripe():
//     direction(READ_UNKNOWN)
// {}

// void MagStripe::begin(unsigned char track) {
//     this->track = track;
    
//     data_interrupt.fall(handle_data);
//     data_interrupt.rise(handle_data);
//     clock_interrupt.fall(handle_clock);
// }

// void MagStripe::stop() {
//     data_interrupt.fall(NULL);
//     data_interrupt.rise(NULL);
//     clock_interrupt.fall(NULL);
// }

// bool MagStripe::available() {
//     return p_cls == 0;
// }

// short MagStripe::read(char *data, unsigned char size) {
//     //Fail if no card is present
//     if (!this->available()) {
//         return -1;
//     }

//     //Empty the bit buffer
//     num_bits = 0;
//     next_bit = 0;

//     //Wait while the data is being read by interrupts
//     while (this->available()) {}

//     //Decode the raw bits
//     short chars = this->decode_bits(data, size);
//     this->direction = READ_FORWARD;

//     //If the data looks bad, reverse and try again...
//     if (chars < 0) {
//         this->reverse_bits();
//         chars = this->decode_bits(data, size);
//         this->direction = READ_BACKWARD;
//     } else {
//         ser.write(data, BIT_BUFFER_LEN/8);
//     }

//     //The card could not be read successfully
//     if (chars < 0) {
//         this->direction = READ_UNKNOWN;
//     }

//     return chars;
// }

// enum ReadDirection MagStripe::read_direction() {
//     return this->direction;
// }

// void MagStripe::reverse_bits() {
//     for (short i = 0; i < num_bits / 2; i++) {
//         bool b = bits_get(i);

//         bits_set(i, bits_get(num_bits - i - 1));
//         bits_set(num_bits - i - 1, b);
//     }
// }

// bool MagStripe::verify_parity(unsigned char c) {
//     unsigned char parity = 0;

//     for (unsigned char i = 0; i < 8; i++) {
//         parity += (c >> i) & 1;
//     }

//     //The parity must be odd
//     return parity % 2 != 0;
// }

// bool MagStripe::verify_lrc(short start, short length) {
//     unsigned char parity_bit = (this->track == 1 ? 7 : 5);

//     //Count number of ones per col, ignoring parity bits
//     for (short i = 0; i < (parity_bit - 1); i++) {
//         short parity = 0;

//         for (short j = i; j < length; j += parity_bit) {
//             parity += bits_get(start + j);
//         }

//         //Even parity is what we want
//         if (parity % 2 != 0) {
//             return false;
//         }
//     }

//     return true;
// }

// short MagStripe::find_sentinel(unsigned char pattern) {
//     unsigned char bit_accum = 0;
//     unsigned char bit_length = (this->track == 1 ? 7 : 5);

//     for (short i = 0; i < num_bits; i++) {
//         bit_accum >>= 1; //rotate bits to the right
//         bit_accum |= bits_get(i) << (bit_length - 1); //add the current bit

//         //Stop when the sentinel pattern is found
//         if (bit_accum == pattern) {
//             return i - (bit_length - 1);
//         }
//     }

//     //No sentinel was found
//     return -1;
// }

// short MagStripe::decode_bits(char *data, unsigned char size) {
//     short bit_count = 0;
//     unsigned char chars = 0;
//     unsigned char bit_accum = 0;
//     unsigned char bit_length = (this->track == 1 ? 7 : 5);

//     short bit_start = this->find_sentinel(this->track == 1 ? 0x45 : 0x0b);
//     // if (bit_start < 0) { //start sentinel not found
//     //     return -1;
//     // }

//     for (short i = bit_start; i < num_bits; i++) {
//         bit_accum >>= 1; //rotate bits to the right
//         bit_accum |= (bits_get(i) << (bit_length - 1)); //add the current bit

//         bit_count ++;

//         if (bit_count % bit_length == 0) {
//             // if (chars >= size) { //buffer is too small
//             //     return -1;
//             // }

//             if (bit_accum == 0) { //reached end of data
//                 break;
//             }

//             // if (!verify_parity(bit_accum)) { //parity must be odd
//             //     return -1;
//             // }

//             //Remove parity bit
//             bit_accum &= ~(1 << (bit_length -1));

//             //Convert the character to ASCII
//             data[chars] = bit_accum + (this->track == 1 ? 0x20 : 0x30);
//             chars++;

//             //Reset
//             bit_accum = 0;
//         }
//     }

//     //Turn the data into a null terminated string
//     data[chars] = '\0';

//     // if (data[chars - 2] != '?') { //end sentinel mislocated
//     //     return -1;
//     // }

//     //Verify LRC (even parity across columns)
//     // if (!verify_lrc(bit_start, chars * bit_length)) {
//     //     return -1;
//     // }

//     return chars;
// }

// static void bits_set(short index, bool bit) {
//     volatile unsigned char *b = &bits[index / 8];
//     unsigned char m = 1 << (index % 8);

//     *b = bit ? (*b | m) : (*b & m);
// }

// static bool bits_get(short index) {
//     return bits[index / 8] & (1 << (index % 8));
// }

// static void handle_data() {
//     next_bit = !next_bit;
// }

// static void handle_clock() {
//     //Avoid a crash if there is an overflow
//     if (num_bits >= BIT_BUFFER_LEN) {
//         return;
//     }

//     bits_set(num_bits, next_bit);
//     num_bits++;
// }