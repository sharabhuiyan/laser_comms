#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "dome_image.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


int x = 0;
uint16_t timer;

const int size = sizeof test_2_image / sizeof test_2_image[0];
const int message_size = 2 * size + 8;
uint8_t message[message_size] = { 0 };
int bit_count;


void set_bit(uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  A[i] = A[i] | flag; // set the bit and the kth position in A[i]
};

void clear_bit(uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  flag = ~flag;
  A[i] = A[i] & flag; // rest the bit at kth position
};

int test_bit(const uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  if (A[i] & flag) {
    return 1;
  } else {
    return 0;
  }
};

// overloaded
int test_bit(uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  if (A[i] & flag) {
    return 1;
  } else {
    return 0;
  }
};


void manchester_encode() {
  // add synchronizing pattern & ending pattern 
  message[0] = 0xFF; message[1] = 0x00; message[2] = 0xFF; message[3] = 0x00;
  message[message_size-4] = 0x00; message[message_size-3] = 0xFF;
  message[message_size-2] = 0x00; message[message_size-1] = 0xFF;

  // encode rest of bits
  int num_bits = 8*size;
  int offset = 32;
  // loop through every bit in image
  for (int k = 0; k < num_bits; k++) {
    // check bit 
    int bit = test_bit(test_2_image, k);
    // encode bit in message
    if (bit == 1) {
      set_bit(message, 2*k + offset);
      clear_bit(message, 2*k + 1 + offset);
    } else {
      clear_bit(message, 2*k + offset);
      set_bit(message, 2*k + 1 + offset);
    }
  }
};


void setup() {
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Initialize the analog resolution
  analogReadResolution(12);
  
  Serial.begin(115200);

  pinMode(11, OUTPUT); 

  tft.drawXBitmap(0, 30, test_2_image, test_2_width, test_2_height, TFT_RED);

  // encode image
  memset(message, 0, sizeof(message)); // for automatically-allocated arrays
  manchester_encode();

  // Serial.println("original image");
  // Serial.println(test_2_image);
  
  // Serial.println("encoded image");
  // Serial.println(message);

  Serial.println("original image");
  for(int i = 0; i < size; i++) {
    printf("%d ", test_2_image[i]);
  }
  printf("\n");
  for(int i = 0; i < 8*size; i++) {
    printf("%d ", test_bit(test_2_image,i));
  }
  printf("\n");

  Serial.println("encoded image");
  for(int i = 0; i < message_size; i++) {
    printf("%d ", message[i]);
  }
  printf("\n");
  for(int i = 0; i < 8*message_size; i++) {
    printf("%d ", test_bit(message,i));
  }
  printf("\n");

  bit_count = 0;

  timer = millis();

}


void one_signal(int upscale) {
  digitalWrite(11, HIGH);  // on
  delay(1*upscale); // waits 1*upscale ms
};

void zero_signal(int upscale) {
  digitalWrite(11, LOW);  // off
  delay(1*upscale); // waits 1*upscale ms

}

void send_synch_seq() {
  // SEND SYNCHRONIZING SEQUENCE
  int count = 0;
  while (count < 32) {
    if (count < 8) {
      Serial.println("first 16 bits");
      one_signal(15);
    } else if (count < 16) {
      Serial.println("second 16 bits");
      zero_signal(15);
    } else if (count < 24) {
      Serial.println("third 16 bits");
      one_signal(15);
    } else if (count < 32) {
      Serial.println("last 16 bits");
      zero_signal(15);
    }
    count++;
  }
  Serial.print(".");
  zero_signal(1);
}


void loop() {
  // // receiver
  // float photosensor = analogRead(6);
  // float actual_v = photosensor * 3.3 / 4096;

  // turn on laser for some time for alignment

  // if (millis() - timer < 5 && bit_count < 8*message_size) {
  //   one_signal(1);
  // }

  // one_signal(1);
  // zero_signal(1);

  // SEND SYNCHRONIZING SEQUENCE

  // while (bit_count < 32) {
  //   if (bit_count < 8) {
  //     Serial.println("first 16 bits");
  //     one_signal(15);
  //   } else if (bit_count < 16) {
  //     Serial.println("second 16 bits");
  //     zero_signal(15);
  //   } else if (bit_count < 24) {
  //     Serial.println("third 16 bits");
  //     one_signal(15);
  //   } else if (bit_count < 32) {
  //     Serial.println("last 16 bits");
  //     zero_signal(15);
  //   }
  //   bit_count++;
  // }
  // Serial.print(".");
  // zero_signal(1);
  // bit_count++;

  // if (bit_count > 3000) {
  //   Serial.println();
  //   bit_count = 0;
  // }

  // one_signal(5);
  // zero_signal(5);


  // emitter
  if (bit_count < 8*message_size) {
    int bit = test_bit(message, bit_count);
    if (bit == 1) {
      one_signal(15);
    } else {
      zero_signal(15);
    }
    bit_count++;
  }

  if (bit_count >= 8*message_size) {
    zero_signal(1);
  }
  //   // resend after 1 seconds
  //   if (millis() - timer > 10000) {
  //     bit_count = 0;
  //     timer = millis();
  //   }
  // }


}
