
#include <RCS620S.h>

// Circuit
// RESET    - 1 RST
// Analog 5 - 2 SCL (internally pulled up)
// Analog 4 - 3 SDA (internally pulled up)
// GND      - 4 VSS
// 5V       - 5 VDD

// RCS620S
#define COMMAND_TIMEOUT               400
#define POLLING_INTERVAL              300
#define RCS620S_MAX_CARD_RESPONSE_LEN 30
#define BUZ 7

// FeliCa Service/System Code
#define CYBERNE_SYSTEM_CODE           0x0003
#define COMMON_SYSTEM_CODE            0xFE00
#define PASSNET_SERVICE_CODE          0x090F
#define EDY_SERVICE_CODE              0x170F
#define NANACO_SERVICE_CODE           0x564F
#define WAON_SERVICE_CODE             0x680B
#define TOKAI_STUDENT_ID_SYSTEM_CODE  0x809E
#define TOKAI_STUDENT_ID_SERVICE_CODE 0x100B

char Student_ID[8];
char sat_ID[6];

RCS620S rcs620s;
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);  
  pinMode(A6, INPUT);
  pinMode(BUZ, OUTPUT);

}

void loop() {
  uint32_t balance;
  uint32_t card_code;
  uint8_t buf[RCS620S_MAX_CARD_RESPONSE_LEN];

  rcs620s.timeout = COMMAND_TIMEOUT;
if(Serial.available()){
  byte b = Serial.read();
  if(b==0x01){
    Serial.println("OK");
  }
}
if (rcs620s.polling(TOKAI_STUDENT_ID_SYSTEM_CODE)) {
    if (requestService(TOKAI_STUDENT_ID_SERVICE_CODE)) {
      if (readEncryption(TOKAI_STUDENT_ID_SERVICE_CODE, 3, buf)) {
        for (int i = 0; i < 5; i++) {
          Student_ID[i] = buf[i + 23];          
        }
        for(int i=0; i<6;i++){
          sat_ID[i] = buf[i+16];
        }
      }
      if (readEncryption(TOKAI_STUDENT_ID_SERVICE_CODE, 4, buf)) {
        for (int i = 0; i < 4; i++) {
          Student_ID[i+5] = buf[i+12];
        }
      }
      Serial.print("OK,");
      for (int i = 0; i < 8; i++) {
          //lcd.print(Student_ID[i]);
          Serial.print(Student_ID[i]);          
      }
      Serial.print(",");
      for(int i=0; i<6; i++){
        Serial.print(sat_ID[i]);
      }
    }
   Serial.println(); 
  } 
  else {
    //LCD.clear();
    //LCD.move(0);
    //LCD.print("Touch");
    //LCD.move(0x44);
    //Serial.println("Card");
  }
  rcs620s.rfOff();
  delay(POLLING_INTERVAL);
}

// request service
int requestService(uint16_t serviceCode) {
  int ret;
  uint8_t buf[RCS620S_MAX_CARD_RESPONSE_LEN];
  uint8_t responseLen = 0;

  buf[0] = 0x02;
  memcpy(buf + 1, rcs620s.idm, 8);
  buf[9] = 0x01;
  buf[10] = (uint8_t)((serviceCode >> 0) & 0xff);
  buf[11] = (uint8_t)((serviceCode >> 8) & 0xff);

  ret = rcs620s.cardCommand(buf, 12, buf, &responseLen);

  if (!ret || (responseLen != 12) || (buf[0] != 0x03) ||
      (memcmp(buf + 1, rcs620s.idm, 8) != 0) || ((buf[10] == 0xff) && (buf[11] == 0xff))) {
    return 0;
  }

  return 1;
}

int readEncryption(uint16_t serviceCode, uint8_t blockNumber, uint8_t *buf) {
  int ret;
  uint8_t responseLen = 0;

  buf[0] = 0x06;
  memcpy(buf + 1, rcs620s.idm, 8);
  buf[9] = 0x01; // サービス数
  buf[10] = (uint8_t)((serviceCode >> 0) & 0xff);
  buf[11] = (uint8_t)((serviceCode >> 8) & 0xff);
  buf[12] = 0x01; // ブロック数
  buf[13] = 0x80;
  buf[14] = blockNumber;

  ret = rcs620s.cardCommand(buf, 15, buf, &responseLen);

  if (!ret || (responseLen != 28) || (buf[0] != 0x07) ||
      (memcmp(buf + 1, rcs620s.idm, 8) != 0)) {
    return 0;
  }

  return 1;
}

void printBalanceLCD(char *card_name, uint32_t *balance) {
  char result[8];
  sprintf(result, "%u", *balance);
  for(int i=0; i<8;i++){
    Serial.print(result[i]);
  } 
  buz_beep(2, 100);
  return;
}
void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "0x%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}
void buz_beep(int loop_num, int ms) {
  for (int i = 0; i < loop_num; i++) {
    pinMode(7, OUTPUT);
    digitalWrite(7, 1);
    delay(ms);
    digitalWrite(7, 0);
    delay(10);
  }
}
