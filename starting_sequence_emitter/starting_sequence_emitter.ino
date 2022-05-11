#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h



#define RXD1 9
#define TXD1 10

byte buf[] = { 0b00000000 };
byte starting_sequence[] = { 0b11111111, 0b00000000 };
byte final_padding[] = { 0b00000000, 0b00000000, 0b00000000, 0b00000000 };

#define dome_608_width 128
#define dome_608_height 128

static const uint8_t  PROGMEM mit_608_dome[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x20, 0x40, 0x00, 0x11, 0x11, 0x40, 0x10, 0x20, 0x44,
   0x24, 0x49, 0x92, 0x24, 0x49, 0x29, 0x49, 0x82, 0x04, 0x25, 0x00, 0x84,
   0x04, 0x41, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
   0x08, 0x00, 0x44, 0x00, 0x00, 0x00, 0x40, 0x88, 0x00, 0x00, 0x00, 0x08,
   0x08, 0x42, 0x08, 0x00, 0x00, 0x44, 0x10, 0x08, 0x08, 0x00, 0x00, 0x00,
   0x22, 0x49, 0x92, 0x80, 0x20, 0x00, 0x41, 0x90, 0x48, 0x80, 0x80, 0x20,
   0x40, 0x92, 0x84, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x04, 0x00, 0x02,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01,
   0x42, 0x10, 0x08, 0x00, 0x01, 0x01, 0x12, 0x04, 0x04, 0x00, 0x08, 0x81,
   0x04, 0x41, 0x08, 0x20, 0x00, 0x80, 0x00, 0x08, 0x08, 0x10, 0x40, 0x10,
   0x80, 0x88, 0x00, 0x00, 0x10, 0x04, 0x00, 0x02, 0x08, 0x01, 0x90, 0x20,
   0x20, 0x40, 0x80, 0x00, 0x08, 0x00, 0x10, 0x02, 0x00, 0x80, 0x20, 0x40,
   0x00, 0x00, 0x02, 0x00, 0x01, 0x01, 0x02, 0x20, 0x00, 0x11, 0x21, 0x20,
   0x00, 0x00, 0x00, 0x04, 0x21, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
   0x10, 0x00, 0x00, 0x04, 0x21, 0x04, 0x01, 0x00, 0x00, 0x00, 0x80, 0x04,
   0x22, 0x44, 0x04, 0x00, 0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x44, 0x00,
   0x40, 0x00, 0x04, 0x10, 0x00, 0x00, 0x48, 0x24, 0x20, 0x49, 0x02, 0x20,
   0x84, 0x10, 0x00, 0x12, 0x01, 0x40, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08, 0x20,
   0x09, 0x44, 0x80, 0x00, 0x00, 0x00, 0x08, 0x80, 0x10, 0x42, 0x44, 0x00,
   0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x08, 0x12, 0x20, 0x42, 0x40, 0x00,
   0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x02, 0x00, 0x80, 0x00, 0x20,
   0x02, 0x00, 0x00, 0x12, 0x42, 0x00, 0x00, 0x01, 0x08, 0x90, 0x00, 0x40,
   0x48, 0x04, 0x10, 0x00, 0x48, 0x08, 0x09, 0x00, 0x00, 0x24, 0x09, 0x82,
   0x00, 0x00, 0x04, 0x04, 0x00, 0x10, 0x82, 0x04, 0x05, 0x02, 0x00, 0x40,
   0x08, 0x00, 0x00, 0x00, 0x20, 0x02, 0x20, 0x00, 0x01, 0x80, 0x20, 0x10,
   0x15, 0x20, 0x40, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x40, 0x02, 0x80, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x08, 0x04,
   0x01, 0x10, 0x40, 0x00, 0x02, 0x08, 0x01, 0x20, 0x23, 0x04, 0x00, 0x88,
   0x08, 0x48, 0x00, 0x10, 0x20, 0x40, 0x02, 0x01, 0x10, 0x10, 0x08, 0x02,
   0x0a, 0x01, 0x02, 0x00, 0x80, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x48,
   0x44, 0x00, 0x01, 0x20, 0x02, 0x20, 0x20, 0x11, 0x00, 0x80, 0x00, 0x00,
   0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0x40, 0x40, 0x55, 0x04, 0x00, 0x80,
   0x04, 0x01, 0x80, 0x20, 0x00, 0x00, 0x80, 0x00, 0x40, 0x10, 0x04, 0x52,
   0xa7, 0x01, 0x04, 0x00, 0x10, 0x04, 0x04, 0x02, 0x90, 0x24, 0x01, 0x88,
   0x04, 0x00, 0x00, 0x40, 0x5d, 0x91, 0x20, 0x09, 0x00, 0x80, 0x00, 0x40,
   0x02, 0x00, 0x08, 0x00, 0x00, 0x21, 0x08, 0x88, 0xeb, 0x02, 0x00, 0x80,
   0x20, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x11, 0x10, 0x80, 0x80, 0x20,
   0xad, 0x05, 0x04, 0x10, 0x04, 0x00, 0x40, 0x04, 0x20, 0x42, 0x10, 0x80,
   0x00, 0x00, 0x10, 0x4a, 0x6d, 0x41, 0x80, 0x00, 0x00, 0x21, 0x00, 0x00,
   0x02, 0x00, 0x20, 0x00, 0x00, 0x04, 0x00, 0x08, 0xdb, 0x08, 0x10, 0x00,
   0x20, 0x00, 0x01, 0x48, 0x00, 0x08, 0x01, 0x11, 0x22, 0x80, 0x80, 0x52,
   0xb6, 0x51, 0x01, 0x22, 0x01, 0x04, 0x88, 0x00, 0x88, 0x00, 0x00, 0x00,
   0x40, 0x08, 0x00, 0x54, 0xdb, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x8a, 0xbb, 0x75, 0x49, 0x00,
   0x88, 0x90, 0x10, 0x24, 0x01, 0x90, 0x24, 0x22, 0x02, 0x00, 0x08, 0x54,
   0xaf, 0x4a, 0x00, 0x42, 0x00, 0x00, 0x80, 0x00, 0x48, 0x02, 0x00, 0x00,
   0x40, 0x10, 0x40, 0xa9, 0xdd, 0xba, 0x00, 0x04, 0x01, 0x02, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xaa, 0xb7, 0x4c, 0x08, 0x00,
   0x08, 0x00, 0x08, 0x44, 0x00, 0x00, 0x10, 0x21, 0x02, 0x20, 0x42, 0x49,
   0x66, 0xcd, 0x20, 0x00, 0x20, 0x10, 0x40, 0x00, 0x22, 0x24, 0x01, 0x00,
   0x00, 0x00, 0x80, 0xaa, 0xa9, 0x5b, 0x00, 0x12, 0x00, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x04, 0x08, 0x02, 0x24, 0x55, 0x73, 0x5b, 0x03, 0x40,
   0x02, 0x80, 0x04, 0x24, 0x40, 0x00, 0x20, 0x20, 0x00, 0x40, 0x20, 0xa9,
   0x55, 0x2d, 0x10, 0x00, 0x10, 0x10, 0x40, 0x80, 0x02, 0x00, 0x01, 0x00,
   0x20, 0x00, 0x4a, 0x4a, 0xd7, 0xad, 0x85, 0x00, 0x00, 0x01, 0x00, 0x00,
   0x14, 0x12, 0x00, 0x00, 0x01, 0x44, 0x48, 0xa9, 0x7d, 0x8b, 0x06, 0x04,
   0x20, 0x00, 0x04, 0x00, 0x80, 0x00, 0x22, 0x02, 0x00, 0x00, 0x95, 0x52,
   0x6b, 0xbf, 0x0d, 0x40, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x40, 0x00, 0x24, 0x55, 0xdf, 0xed, 0x06, 0x10, 0x00, 0x02, 0x00, 0x00,
   0x48, 0x05, 0x00, 0x00, 0x02, 0x48, 0xa9, 0xa4, 0xfb, 0xbb, 0x95, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x91, 0x14, 0x02, 0x00, 0x00, 0x40, 0x92, 0x4a,
   0xaf, 0xaf, 0x02, 0x00, 0x08, 0x08, 0x00, 0x12, 0x24, 0x15, 0x10, 0x20,
   0x10, 0x24, 0x55, 0xa9, 0x7d, 0xf5, 0x0e, 0x00, 0x40, 0x00, 0x10, 0xa0,
   0xa4, 0x52, 0x02, 0x01, 0x00, 0xa0, 0xaa, 0xaa, 0xf7, 0x9b, 0x02, 0x22,
   0x00, 0x40, 0x80, 0x00, 0x52, 0xad, 0x00, 0x00, 0x41, 0x44, 0x52, 0xa5,
   0x5e, 0x73, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa9, 0x17, 0x00,
   0x00, 0x94, 0x4a, 0xa9, 0xf7, 0xde, 0x02, 0x00, 0x00, 0x01, 0x00, 0x20,
   0x54, 0x52, 0x0a, 0x00, 0x50, 0xa1, 0x54, 0x55, 0xde, 0xaa, 0x24, 0x00,
   0x08, 0x00, 0x00, 0x00, 0x40, 0x8a, 0xba, 0x08, 0x20, 0x45, 0xa9, 0xaa,
   0x77, 0x57, 0x01, 0x02, 0x00, 0x00, 0x00, 0x40, 0x8a, 0xa4, 0x4a, 0x00,
   0x48, 0x55, 0x55, 0x55, 0xee, 0x4d, 0x02, 0x80, 0x00, 0x00, 0x80, 0x4a,
   0xb2, 0x2a, 0x55, 0x40, 0x50, 0xaa, 0xaa, 0xaa, 0x5b, 0x2b, 0x08, 0x10,
   0x00, 0x20, 0x09, 0x00, 0xa4, 0xaa, 0x0a, 0x00, 0x4a, 0x55, 0x55, 0x55,
   0x6e, 0x5b, 0x21, 0x00, 0x10, 0x02, 0x00, 0x80, 0x88, 0x54, 0x75, 0x01,
   0x54, 0xd5, 0xaa, 0xaa, 0xdb, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
   0x20, 0x49, 0x4a, 0x01, 0x54, 0x5b, 0x25, 0x55, 0xb5, 0xdb, 0x83, 0x00,
   0x01, 0x00, 0x00, 0x00, 0x89, 0x92, 0xa8, 0x06, 0x2d, 0xd5, 0x6a, 0xab,
   0xd5, 0x56, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x24, 0x45, 0x81,
   0xaa, 0x6a, 0x85, 0x54, 0xfb, 0xdf, 0x1d, 0x08, 0x00, 0x00, 0x00, 0x20,
   0x42, 0x49, 0x29, 0x01, 0xdd, 0xaa, 0xb5, 0xaa, 0xad, 0x6a, 0x07, 0x00,
   0x00, 0x00, 0x00, 0x04, 0x88, 0x92, 0x44, 0x00, 0x55, 0x6f, 0x57, 0x55,
   0x5d, 0xed, 0x2d, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x2a, 0xa9, 0x02,
   0x55, 0x55, 0x55, 0xb5, 0xb7, 0xb7, 0x06, 0x20, 0x10, 0x80, 0x24, 0x90,
   0x44, 0x41, 0x92, 0x00, 0xfa, 0x6e, 0xad, 0xaa, 0xb6, 0xee, 0x5d, 0x85,
   0x42, 0x15, 0x84, 0x02, 0x08, 0x08, 0x02, 0x09, 0x50, 0xad, 0x59, 0xad,
   0x5b, 0xbb, 0x2a, 0x52, 0x14, 0xa2, 0x52, 0x54, 0x51, 0xa5, 0x54, 0x52,
   0xed, 0x5a, 0xb7, 0xaa, 0xf7, 0xee, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00, 0xa8, 0x55, 0xed, 0xba, 0x5a, 0xb5, 0x96, 0x00,
   0x10, 0x04, 0x84, 0x00, 0x00, 0x04, 0x40, 0x04, 0x52, 0xfb, 0xaa, 0xda,
   0xd7, 0xff, 0x5d, 0x24, 0x41, 0x40, 0x00, 0x24, 0x48, 0x90, 0x04, 0x10,
   0xa0, 0xd6, 0xff, 0x5a, 0xb5, 0xaa, 0xb7, 0x00, 0x00, 0x08, 0x08, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xca, 0x75, 0xea, 0x6a, 0x76, 0x6f, 0xad, 0x01,
   0x84, 0x00, 0x80, 0x00, 0x01, 0x01, 0x88, 0x20, 0xa8, 0xd5, 0xde, 0xdd,
   0xdf, 0xba, 0x5b, 0x24, 0x11, 0x90, 0x08, 0x12, 0x24, 0x48, 0x20, 0x04,
   0x50, 0xf5, 0xbb, 0xaa, 0xba, 0x77, 0x5b, 0x55, 0x55, 0xab, 0x52, 0xa2,
   0x04, 0x81, 0x02, 0x20, 0xd5, 0x56, 0xef, 0xab, 0x6f, 0xed, 0xad, 0xaa,
   0x54, 0xa9, 0x5a, 0x5d, 0xfd, 0xbe, 0xfa, 0xad, 0x5e, 0xfd, 0xba, 0x76,
   0xdd, 0xad, 0xad, 0xda, 0x77, 0xbb, 0x56, 0xb5, 0x49, 0xa1, 0x0a, 0x55,
   0xf9, 0xab, 0xef, 0x5d, 0xbf, 0x7b, 0xb7, 0x24, 0x88, 0x4a, 0xa9, 0x55,
   0xbb, 0xbd, 0xf5, 0xee, 0x6e, 0x77, 0xb5, 0xd2, 0xb7, 0x64, 0x2d, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, 0xed, 0xf7, 0xaf,
   0x5d, 0xdd, 0x5b, 0xa4, 0x12, 0x10, 0x11, 0x00, 0x00, 0x00, 0x04, 0x40,
   0xd7, 0xb6, 0xde, 0x6a, 0x37, 0x12, 0x8b, 0x00, 0x00, 0x01, 0x80, 0x24,
   0x11, 0x89, 0x20, 0x09, 0xde, 0xef, 0xdd, 0xd7, 0xbf, 0xac, 0x56, 0xb4,
   0x72, 0x74, 0xb1, 0x52, 0xa2, 0x20, 0x92, 0x20, 0x7d, 0xb5, 0x77, 0xb5,
   0xfb, 0xf9, 0x2a, 0xe0, 0xe5, 0xe1, 0xf5, 0xf4, 0xf5, 0xf1, 0xf0, 0xf2,
   0xea, 0xef, 0xee, 0xef, 0x6f, 0xdb, 0x95, 0xec, 0xf9, 0xbd, 0xd9, 0xfd,
   0xdd, 0xed, 0xf5, 0xfc, 0xac, 0xba, 0xbb, 0x55, 0xff, 0xbb, 0x16, 0xd0,
   0xe9, 0x71, 0x79, 0xa9, 0xb9, 0xb9, 0xf5, 0xf1, 0xb9, 0xff, 0xf7, 0xef,
   0xdf, 0xef, 0x8d, 0xe8, 0xf9, 0xb9, 0xd9, 0x5c, 0xd5, 0xd5, 0xd8, 0xd8,
   0xac, 0x6a, 0x5f, 0xbf, 0x7b, 0xbd, 0x3d, 0xd1, 0xa9, 0x75, 0xb1, 0xf9,
   0x59, 0xd9, 0xbb, 0xf9, 0xd8, 0xfe, 0xfa, 0xfa, 0xff, 0xff, 0xaf, 0x6c,
   0x75, 0xd9, 0xdc, 0x54, 0xf5, 0x75, 0x75, 0xad, 0x54, 0xdb, 0xef, 0xef,
   0xef, 0xf7, 0xaf, 0xdb, 0xd5, 0xf1, 0xb9, 0xd9, 0xb5, 0xe9, 0x69, 0xf9,
   0xb8, 0xff, 0x6d, 0xfd, 0x7b, 0xbf, 0x5d, 0x75, 0xb9, 0xb4, 0xf4, 0xbc,
   0xdc, 0xdd, 0xb6, 0xd5, 0xd6, 0xb6, 0xbf, 0xef, 0xff, 0xfe, 0xbf, 0xeb,
   0xeb, 0xf5, 0xa9, 0xf1, 0xf9, 0xd9, 0xe9, 0xda, 0xaa, 0xf7, 0x77, 0xdd,
   0xff, 0xff, 0x5b, 0xb7, 0xf5, 0xb4, 0x7e, 0x5d, 0x59, 0x75, 0x5d, 0x6d,
   0xed, 0xbe, 0xbd, 0xf7, 0xee, 0x6d, 0x7f, 0xf5, 0xdd, 0xf5, 0x68, 0xf1,
   0xf9, 0xd9, 0xb1, 0xa9, 0xaa, 0xfb, 0xf7, 0xef, 0xff, 0xff, 0xab, 0xf7,
   0xeb, 0xbb, 0xfd, 0xde, 0xda, 0x6a, 0x6d, 0xfd, 0xdc, 0xff, 0xbf, 0xbb,
   0xfd, 0x7f, 0x7f, 0xad, 0xfb, 0xf5, 0xf5, 0xf8, 0xf9, 0xd9, 0xe9, 0xa9,
   0xf5, 0xf6, 0x7b, 0xe3, 0x7f, 0xfb, 0xab, 0xfb, 0x6d, 0x7b, 0xdb, 0xb5,
   0x79, 0x6d, 0xbf, 0x76, 0xad, 0xbf, 0xef, 0x77, 0xfb, 0xff, 0x7f, 0xeb,
   0xed, 0x77, 0xbb, 0xf5, 0xeb, 0xfb, 0xea, 0x6d, 0xfb, 0x6a, 0xdd, 0xee,
   0x7f, 0xdb, 0xaa, 0xf7, 0xb7, 0xfa, 0x75, 0xdb, 0x5e, 0xd7, 0x5f, 0xf7,
   0xad, 0xff, 0xf7, 0xbf, 0xff, 0xff, 0xf7, 0xae, 0xfd, 0x75, 0x77, 0xfb,
   0xfa, 0xbe, 0xfa, 0x5e, 0xff, 0xde, 0xbe, 0xfb, 0xfd, 0x6d, 0xaf, 0xfb,
   0xab, 0xde, 0xfd, 0xad, 0xb7, 0xdb, 0x6f, 0xfb, 0xeb, 0xfb, 0xeb, 0xfe,
   0xfd, 0xfd, 0x6d, 0xef, 0xff, 0xf9, 0xf5, 0xfb, 0x7e, 0xff, 0xfd, 0x6f,
   0x5f, 0x6f, 0xff, 0xdb, 0xff, 0xff, 0xbb, 0xf5, 0xea, 0xb7, 0xdd, 0xdc,
   0xfb, 0xb5, 0xb7, 0xfb, 0xfb, 0xfd, 0xdd, 0xff, 0xfb, 0xfa, 0xb7, 0xef,
   0xff, 0xfa, 0xfb, 0x7b, 0xd7, 0xff, 0x7e, 0x6f, 0xef, 0xb7, 0x7f, 0xf7,
   0xff, 0xaf, 0xdd, 0xfa, 0xa9, 0xbb, 0xfa, 0xda, 0xff, 0xf7, 0xeb, 0xff,
   0xbf, 0xff, 0xfe, 0xbf, 0xff, 0xfd, 0xfb, 0xaf, 0xbb, 0xf6, 0xeb, 0x7b,
   0xbd, 0xde, 0x7f, 0xdb, 0xfa, 0xed, 0xfb, 0xfd, 0xff, 0xdf, 0x57, 0xf5,
   0xf7, 0xad, 0xb6, 0xf6, 0xf7, 0xf7, 0xef, 0xff, 0x6f, 0x7f, 0xdf, 0xff,
   0xf7, 0xfb, 0xfe, 0xbf, 0x56, 0xfb, 0xed, 0xad, 0x7f, 0xdf, 0x7d, 0xbb,
   0xfd, 0xf6, 0xfb, 0xef, 0xdf, 0xdf, 0x5b, 0xed, 0xff, 0xaf, 0xbf, 0xf6,
   0xda, 0xfb, 0xf7, 0xef, 0xef, 0xdf, 0xdf, 0xfe, 0x7d, 0x7b, 0xff, 0xbf,
   0x6d, 0x7d, 0xf5, 0xdf, 0x7f, 0xff, 0xef, 0xbe, 0xdd, 0xfd, 0xfe, 0xff,
   0xf7, 0xef, 0xb5, 0xed, 0xbf, 0xf7, 0x5f, 0x75, 0xf7, 0x7b, 0xbf, 0xf7,
   0xff, 0xf7, 0xfb, 0xef, 0x5e, 0xbd, 0xff, 0x7d, 0xf5, 0x56, 0xfb, 0xef,
   0xff, 0xf7, 0xff, 0xbf, 0x7d, 0xef, 0xfb, 0xff, 0xfb, 0xf7, 0xaa, 0xd7,
   0xaf, 0xff, 0x56, 0x7d, 0x7b, 0x7f, 0x77, 0xff, 0xf7, 0xbd, 0xdf, 0xff,
   0x57, 0xad, 0x6f, 0xb5, 0xba, 0xaa, 0xdd, 0xeb, 0xef, 0xf6, 0xee, 0xb6,
   0xdf, 0xff, 0x7f, 0xff, 0xfd, 0xbb, 0xda, 0x6e, 0xeb, 0x76, 0x6b, 0xbd,
   0xfd, 0xff, 0xff, 0xff, 0xff, 0xb6, 0xfd, 0xfb, 0x05, 0xa5, 0x4a, 0x49,
   0x95, 0x4a, 0x4a, 0xf5, 0xef, 0xde, 0xdf, 0xfd, 0xef, 0xff, 0xdb, 0xef,
   0xa8, 0x94, 0x90, 0x94, 0x24, 0xa9, 0x2a, 0xbd, 0xfd, 0xfb, 0x7a, 0xef,
   0xff, 0x7f, 0xff, 0xff, 0xad, 0xb5, 0xb7, 0xb5, 0xad, 0xaa, 0x52, 0xf1,
   0xdb, 0xf7, 0xff, 0x7f, 0xbf, 0xfe, 0xf7, 0xfa, 0xbb, 0x6e, 0xdd, 0xee,
   0xf6, 0x55, 0x7f, 0xdf, 0xff, 0x7f, 0xf7, 0xde, 0xf7, 0xff, 0xde, 0xef,
   0xf7, 0xdb, 0xdb, 0xbb, 0x5d, 0x7f, 0xf5, 0x7a, 0x7f, 0xf7, 0xde, 0x7d,
   0xff, 0xfb, 0x7f, 0xff, 0xad, 0x6e, 0x6d, 0x6d, 0xab, 0xaa, 0xff, 0xef,
   0xdb, 0xff, 0xf7, 0xf7, 0x6f, 0xdf, 0xfd, 0xbd, 0xb5, 0xad, 0xad, 0xd5,
   0xda, 0x6a, 0xfd, 0x6d, 0xff, 0xde, 0x7e, 0xff, 0xfe, 0xfb, 0xd7, 0xfb,
   0xef, 0xf6, 0x76, 0xbf, 0xb7, 0xdf, 0xfe, 0xfb, 0xf7, 0xff, 0xf7, 0xb7,
   0xef, 0xfe, 0xfd, 0xff, 0xbd, 0xdd, 0xde, 0xb5, 0x76, 0xb5, 0xff, 0xaf,
   0xdf, 0xdd, 0xde, 0xfe, 0xfd, 0xdf, 0x7f, 0xef, 0xeb, 0xf6, 0xb5, 0xf6,
   0xad, 0xdb, 0xfe, 0xfa, 0xfb, 0xff, 0xf7, 0xef, 0xdf, 0xfd, 0xee, 0xfb,
   0xde, 0xae, 0xf7, 0xad, 0xbd, 0xde, 0xfd, 0x77, 0x7f, 0x6f, 0xff, 0x7d,
   0xff, 0xef, 0xff, 0xdf, 0xeb, 0x75, 0xad, 0xb6, 0xeb, 0xb5, 0xff, 0xfd,
   0xfb, 0xff, 0xb7, 0xdf, 0xdb, 0x7b, 0xfb, 0xff, 0x5e, 0xdf, 0xef, 0xb6,
   0x5e, 0x6b, 0xfd, 0xd7, 0xd7, 0xfd, 0xfe, 0xfd, 0xff, 0xff, 0xf7, 0xff,
   0xfb, 0xb5, 0xba, 0xfb, 0x75, 0xdd, 0xff, 0x7e, 0xff, 0xb7, 0xff, 0xbf,
   0xf7, 0xff, 0xff, 0xde, 0x57, 0xf7, 0xef, 0xae, 0xd7, 0x5b, 0xfd, 0xfb,
   0x7f, 0xff, 0xb7, 0xf7, 0xdf, 0xff, 0xff, 0xff };

const int IMAGE_SIZE = 128 * 128;
const int size = 16; //sizeof test_3_image / sizeof test_3_image[0];
uint8_t PROGMEM image_chunk[size] = { 0 };
int buffer_start;
int buffer_end;
int chunk_count;

int started;
int ended;

enum transmission_state{GET_IMAGE, DELAY1, TRANSMIT, DELAY2};
enum transmission_state state;


void next_chunk(uint8_t* image_chunk) {
  if (buffer_end <= IMAGE_SIZE) {
    int counter = 0;
    for (int i = buffer_start; i < buffer_end; i++) {
      image_chunk[counter] = mit_608_dome[i];
      counter++;
    }
    buffer_start += 16;
    buffer_end += 16;
  }
};

void setup() {
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // put your setup code here, to run once:
  Serial.begin(19200, SERIAL_8N1); // initialize UART with baud rate of 9600 bps
  Serial1.begin(19200, SERIAL_8N1, RXD1, TXD1);

  
  buffer_start = 0;
  buffer_end = 16;
  chunk_count = 0;
  
  state = GET_IMAGE;
  started = 0;
  ended = 0;

}


void loop() {
  // put your main code here, to run repeatedly:
//  Serial1.write(buf, 1);

  switch(state) {
    case GET_IMAGE:
      // Encode image
      memset(image_chunk, 0, sizeof(size));
      next_chunk(image_chunk);
      tft.drawXBitmap(0, 30+chunk_count, image_chunk, 128, 1, TFT_RED);
      chunk_count++;
      state = DELAY1;
      break;
    case DELAY1:
//      delay(10);
      state = TRANSMIT;
      break;
    case TRANSMIT:
      // emitter
      if (!started) {
        Serial1.write(starting_sequence, 2);
        started = 1;
      }
      Serial1.write(image_chunk, size); 
//      else if (ended) {
//        // fill rest of screen
//        Serial1.write(buf, 1);
//      }
      state = DELAY2;
      break;
    case DELAY2:
//      delay(10);
      if (chunk_count < 128) {
        state = GET_IMAGE;
      } else if (!ended) {
        Serial1.write(final_padding, 4);
        ended = 1;
      }
      break;
  }




}