
/*----------------------------------
 * char_append Function:
 * Arguments:
 *    char* buff: pointer to character array which we will append a
 *    char c: 
 *    uint16_t buff_size: size of buffer buff
 *    
 * Return value: 
 *    boolean: True if character appended, False if not appended (indicating buffer full)
 */
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
        int len = strlen(buff);
        if (len>buff_size) return false;
        buff[len] = c;
        buff[len+1] = '\0';
        return true;
}

/*----------------------------------
 * do_http_request Function:
 * Arguments:
 *    const char* host: null-terminated char-array containing host to connect to
 *    char* request: null-terminated char-arry containing properly formatted HTTP request
 *    char* response: char-array used as output for function to contain response
 *    uint16_t response_size: size of response buffer (in bytes)
 *    uint16_t response_timeout: duration we'll wait (in ms) for a response from server
 *    uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
 * Return value:
 *    void (none)
 */
void do_http_request(const char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}        


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

// void next_chunk(uint8_t* image_chunk) {
//   if (buffer_end <= IMAGE_SIZE) {
//     int counter = 0;
//     for (int i = buffer_start; i < buffer_end; i++) {
//       image_chunk[counter] = mit_608_dome[i];
//       counter++;
//     }
//     buffer_start += 16;
//     buffer_end += 16;
//   }
// };


void int_extractor(char* data_array, int* output_values, char delimiter){
    //your code here
   char delim[2];
   delim[0] = delimiter;
   delim[1] = '\0';
   char* val = strtok(data_array, delim);
   int i =0;
   while(val != NULL){
     output_values[i] += atoi(val);
     val = strtok(NULL, delim);
     i++;
   }
}


