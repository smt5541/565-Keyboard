// #ifndef MAGSTRIPE_H
// #define MAGSTRIPE_H

// #include <mbed.h>

// //Green = GND
// //Yellow = 5V
// //Brown = Data
// //Red = Clock
// //Orange = Card Loaded

// #define MAGSTRIPE_DATA PC_13
// #define MAGSTRIPE_CLOCK PA_12
// #define MAGSTRIPE_LOAD PC_12

// enum ReadDirection { READ_UNKNOWN, READ_FORWARD, READ_BACKWARD };

// class MagStripe {
//     public:
//         MagStripe();

//         //Initialize the library (attach interrupts)
//         void begin(unsigned char track);

//         //Deinitialize the library (deattach interrupts)
//         void stop();

//         //Check if there is a card present for reading
//         bool available();

//         //Reach the data from the card as ASCII
//         short read(char *data, unsigned char size);

//         //The direction of the last card read
//         enum ReadDirection read_direction();

//     private:
//         unsigned char track;
//         enum ReadDirection direction;

//         void reverse_bits();
//         bool verify_parity(unsigned char c);
//         bool verify_lrc(short start, short length);
//         short find_sentinel(unsigned char pattern);
//         short decode_bits(char *data, unsigned char size);
// };

// #endif