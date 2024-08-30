#include <Dali.h>

#define LED_PIN PB14

enum TUseGroup { SHORT_ADDRESS,
                 GROUP_ADDRESS };

struct TDaliDevice {
  byte DeviceType = 0;
  byte ShortAdress = 0;
  byte Detected = 0;
  byte HighAdressByte = 0;
  byte MedAdressByte = 0;
  byte LowAdressByte = 0;
  byte Groups0_7 = 0;
  byte Groups8_15 = 0;
  byte FadeTime = 0;
  byte FadeRate = 0;
  byte LampFailure = 0;
  byte ActualLevel = 0;
};
TDaliDevice DaliDevice[64];

const int DALI_TX = PA8;
//const int DALI_RX_A = PA3; так должно быть
const int DALI_RX_A = PA2;  //версия платы v1


//COMMANDS
#define ON 0x22
#define OFF 0x23
#define ADD_TO_GROUP 0x24
#define REMOVE_FROM_GROUP 0x25
#define QUERY_GROUP 0x26
#define SET_SHORT_ADDRESS 0x29
#define DELETE_SHORT_ADDRESS 0x30
#define SET_DIM_LEVEL 0x27
#define SET_FADE_TIME 0x28
#define SET_FADE_RATE 0x33
#define SCAN 0x20
#define DETECT 0x21
#define EXTEND 0x31
#define GET_INFO 0x32

//COMMAND TYPES
#define Broadcast 0x50
#define SingleDevice 0x51
#define GroupDevice 0x52

//DALI
#define RECALL_OFF 0
#define RECALL_MAX_LEVEL 5
#define RECALL_MIN_LEVEL 6
#define RESET 32
#define QUERY_CONTROL_GEAR 145
#define QUERY_RANDOM_ADRESS_HIGH 194
#define QUERY_RANDOM_ADRESS_MED 195
#define QUERY_RANDOM_ADRESS_LOW 196
#define QUERY_DEVICE_TYPE 153


//--------------  SERVICE  ----------------//

void SendDaliCommand(TUseGroup UseGroup, byte Adress, byte Command) {
  byte byteaddr = (UseGroup * 128) + (Adress << 1) + 1;
  dali.transmit(byteaddr, Command);
}

int SendDaliCommandAndRecive(TUseGroup UseGroup, byte Adress, byte Command) {
  byte byteaddr = (UseGroup * 128) + (Adress << 1) + 1;
  dali.transmit(byteaddr, Command);
  delay(5);
  byte response = dali.receive();
  if (dali.getResponse) {
    return response;
  } else {
    return -1;
  }
}

void SendSpecialCommand(byte Command, byte Param) {
  dali.transmit(Command, Param);
}

void splitAdd(long input, uint8_t &highbyte, uint8_t &middlebyte, uint8_t &lowbyte) {
  highbyte = input >> 16;
  middlebyte = input >> 8;
  lowbyte = input;
}

int Send_Receive(byte first, byte second) {
  dali.transmit(first, second);
  delay(5);
  byte response = dali.receive();
  if (dali.getResponse) {
    return response;
  } else {
    delay(2);
    return 0;
  }
}


//--------------  RESPONSE  -------------//

void Respond_OK() {
  uint8_t buff[16] = { 0xAB, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0xBA };
  Serial.write(buff, 16);
}

void Update_Info(uint8_t addr) {
  uint8_t r = 0;

  if (DaliDevice[addr].Detected == 1) {

    //GET LAMP FAILURE
    dali.transmit(((addr << 1) + 1), 0x92);
    delay(10);
    r = dali.receive();
    if (r == 0x00) {
      DaliDevice[addr].LampFailure = 0;
    } else {
      DaliDevice[addr].LampFailure = 1;
    }

    // GET GROUPS
    r = SendDaliCommandAndRecive(SHORT_ADDRESS, addr, 0xC0);  // get groups from 0 to 7
    DaliDevice[addr].Groups0_7 = r;
    delay(10);
    r = SendDaliCommandAndRecive(SHORT_ADDRESS, addr, 0xC1);  // get groups from 8 to 15
    DaliDevice[addr].Groups8_15 = r;
    delay(10);

    //GET DEVICE TYPE
    SendSpecialCommand(0xC1, 8);
    delay(10);
    r = SendDaliCommandAndRecive(SHORT_ADDRESS, addr, QUERY_DEVICE_TYPE);
    if (r != -1) {
      DaliDevice[addr].DeviceType = r;
    }
    delay(10);

    // GET ACTUAL LEVEL
    r = SendDaliCommandAndRecive(SHORT_ADDRESS, addr, 0xA0);
    DaliDevice[addr].ActualLevel = r;
    delay(10);

    // GET FADE TIME - FADE RATE
    r = SendDaliCommandAndRecive(SHORT_ADDRESS, addr, 0xA5);
    uint8_t fadeTime = r >> 4;
    DaliDevice[addr].FadeTime = fadeTime;
    uint8_t fadeRate = r & 0b00001111;
    DaliDevice[addr].FadeRate = fadeRate;
  }
}

void Respond_INFO(uint8_t address) {

  Update_Info(address);
  delay(10);
  Serial.println("info");

  uint8_t buff[16];

  buff[0] = 0xAB;
  buff[15] = 0xBA;

  buff[1] = DaliDevice[address].ShortAdress;
  buff[2] = DaliDevice[address].Detected;
  buff[3] = DaliDevice[address].DeviceType;
  buff[4] = DaliDevice[address].Groups0_7;
  buff[5] = DaliDevice[address].Groups8_15;
  buff[6] = DaliDevice[address].FadeTime;
  buff[7] = DaliDevice[address].FadeRate;
  buff[8] = DaliDevice[address].LampFailure;
  buff[9] = DaliDevice[address].ActualLevel;
  buff[10] = 0xCC;
  buff[11] = 0xCC;
  buff[12] = 0xCC;
  buff[13] = 0xCC;
  buff[14] = 0xCC;

  Serial.write(buff, 16);
  delay(10);
}

void Respond_DETECT() {
  Serial.println("info");

  for (int i = 0; i < 64; ++i) {
    if (DaliDevice[i].Detected == 1) {
      uint8_t buff[16];

      buff[0] = 0xAB;
      buff[15] = 0xBA;

      buff[1] = DaliDevice[i].ShortAdress;
      buff[2] = DaliDevice[i].Detected;
      buff[3] = 0xDD;
      buff[4] = 0xDD;
      buff[5] = 0xDD;
      buff[6] = 0xDD;
      buff[7] = 0xDD;
      buff[8] = 0xDD;
      buff[9] = 0xDD;
      buff[10] = 0xDD;
      buff[11] = 0xDD;
      buff[12] = 0xDD;
      buff[13] = 0xDD;
      buff[14] = 0xDD;

      Serial.write(buff, 16);
      delay(10);
    }
  }
}


//------------  MY FUNCTIONS  -----------//

void initialise() {
  const int delaytime = 10;  //ms

  long low_longadd = 0x000000;
  long high_longadd = 0xFFFFFF;
  long longadd = (long)(low_longadd + high_longadd) / 2;
  uint8_t highbyte;
  uint8_t middlebyte;
  uint8_t lowbyte;
  uint8_t short_add = 0;

  int resp;

  delay(delaytime);
  dali.transmit(0b10100101, 0b00000000);  //initialise
  delay(delaytime);
  dali.transmit(0b10100101, 0b00000000);  //initialise
  delay(delaytime);

  dali.transmit(0b10100001, 0b00000000);  //terminate
  delay(delaytime);

  dali.transmit(0b10100101, 0b00000000);  //initialise
  delay(delaytime);
  dali.transmit(0b10100101, 0b00000000);  //initialise
  delay(delaytime);

  dali.transmit(0b11111111, 0b00100000);  //reset
  delay(delaytime);
  dali.transmit(0b11111111, 0b00100000);  //reset
  delay(delaytime);

  dali.transmit(0b10100011, 0b11111111);  //DTR0 DATA
  delay(delaytime);

  dali.transmit(0b11111111, 0b10000000);  //bring shortaddr FF
  delay(delaytime);
  dali.transmit(0b11111111, 0b10000000);  //bring shortaddr FF
  delay(delaytime);

  dali.transmit(0b10100111, 0b00000000);  //randomise
  delay(delaytime);
  dali.transmit(0b10100111, 0b00000000);  //randomise

  while ((longadd <= (0xFFFFFF - 2)) && (short_add <= 64)) {
    while ((high_longadd - low_longadd) > 1) {

      splitAdd(longadd, highbyte, middlebyte, lowbyte);  //divide 24bit adress into three 8bit adresses
      delay(delaytime);

      dali.transmit(0b10110001, highbyte);  //search HB
      delay(delaytime);

      dali.transmit(0b10110011, middlebyte);  //search MB
      delay(delaytime);

      dali.transmit(0b10110101, lowbyte);  //search LB
      delay(delaytime);

      resp = Send_Receive(0b10101001, 0b00000000);

      if (resp == 0) {
        low_longadd = longadd;
      } else {
        high_longadd = longadd;
      }

      longadd = (low_longadd + high_longadd) / 2;  //center

    }  // second while

    // Serial.print("low_addr  ");
    // Serial.println(low_longadd);
    // Serial.print("high_addr  ");
    // Serial.println(high_longadd);
    // Serial.print("longaddr  ");
    // Serial.println(longadd);

    if (high_longadd != 0xFFFFFF) {
      splitAdd(longadd + 1, highbyte, middlebyte, lowbyte);

      dali.transmit(0b10110001, highbyte);  //search HB
      delay(delaytime);

      dali.transmit(0b10110011, middlebyte);  //search MB
      delay(delaytime);

      dali.transmit(0b10110101, lowbyte);  //search LB
      delay(delaytime);

      dali.transmit(0b10110111, 1 + (short_add << 1));  //program short adress
      delay(delaytime);

      //tut
      dali.transmit(0b10111011, 0b00000000);  //BB00
      delay(delaytime);

      resp = Send_Receive(1 + (short_add << 1), 0b11000010);
      delay(delaytime);
      resp = Send_Receive(1 + (short_add << 1), 0b11000011);
      delay(delaytime);
      resp = Send_Receive(1 + (short_add << 1), 0b11000100);
      delay(delaytime);

      dali.transmit(0b10101011, 0b00000000);  //withdraw
      delay(delaytime);

      // dali.transmit(1 + (short_add << 1), ON_C);
      // delay(1000);
      // dali.transmit(1 + (short_add << 1), OFF_C);
      // delay(delaytime);

      dali.transmit(0b11000001, 0x08);
      delay(delaytime);

      resp = Send_Receive(1 + (short_add << 1), 0xFF);
      delay(delaytime);

      Serial.println(short_add);
      short_add++;

      high_longadd = 0xFFFFFF;
      longadd = (low_longadd + high_longadd) / 2;

    } else {
      if (dali.msgMode) {
        Serial.println("End");
      }
    }
  }  // first while


  dali.transmit(0b10100001, 0b00000000);  //terminate
  delay(10);
  dali.transmit(BROADCAST_C, ON_C);  //broadcast on
  //Serial.println("EST");
}

void DetectDevices() {
  int r = -1;

  for (int i = 0; i < 64; i++) {
    DaliDevice[i].Detected = 0;
    r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, QUERY_CONTROL_GEAR);
    //delay(100);

    // if (r == 0) {
    //   Serial.print("Adress: ");
    //   Serial.print(i);
    //   Serial.println(" NOT DETECTED ");

    // } else if (r == 255) {
    if (r == 255) {
      DaliDevice[i].Detected = 1;
      DaliDevice[i].ShortAdress = i;

      //GET GROUPS
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xC0);  // get groups from 0 to 7
      // DaliDevice[i].Groups0_7 = r;
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xC1);  // get groups from 8 to 15
      // DaliDevice[i].Groups8_15 = r;

      // GET BYTE CODES
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, QUERY_RANDOM_ADRESS_HIGH);
      // if (r != -1) {
      //   DaliDevice[i].HighAdressByte = r;
      // }
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, QUERY_RANDOM_ADRESS_MED);
      // if (r != -1) {
      //   DaliDevice[i].MedAdressByte = r;
      // }
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, QUERY_RANDOM_ADRESS_LOW);
      // if (r != -1) {
      //   DaliDevice[i].LowAdressByte = r;
      // }

      //GET DEVICE TYPE
      // SendSpecialCommand(0xC1, 8);
      // delay(10);
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, QUERY_DEVICE_TYPE);
      // if (r != -1) {
      //   DaliDevice[i].DeviceType = r;
      // }

      //GET ACTUAL LEVEL
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xA0);
      // DaliDevice[i].ActualLevel = r;
      //Serial.println(r, HEX);

      //GET LAMP FAILURE
      // r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0x92);
      // if (r == 0xFF) {
      //   DaliDevice[i].LampFailure = 1;
      // } else {
      //   DaliDevice[i].LampFailure = 0;
      // }

      //Serial.println(r, HEX);

      // GET FADE TIME - FADE RATE
      //r = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xA5);
      //Serial.print("Resp  ");
      //Serial.println(r, HEX);
      //uint8_t save = r;

      // uint8_t fadeTime = r >> 4;
      // DaliDevice[i].FadeTime = fadeTime;
      //Serial.print("Time  ");
      //Serial.println(fadeTime, HEX);

      //Serial.print("Save  ");
      //Serial.println(save, BIN);
      // uint8_t fadeRate = r & 0b00001111;
      // DaliDevice[i].FadeRate = fadeRate;
      //Serial.print("Rate  ");
      //Serial.println(fadeRate, HEX);

      // Serial.print("Adress: ");
      // Serial.print(i);
      // Serial.print(" DETECTED (Long Adress: 0x");
      // Serial.print(DaliDevice[i].HighAdressByte);
      // Serial.print(DaliDevice[i].MedAdressByte);
      // Serial.print(DaliDevice[i].LowAdressByte);
      // Serial.print(" Type: ");
      // Serial.print(DaliDevice[i].DeviceType);
      // Serial.println(")");
    }
  }
}

void AddToGroup(uint8_t deviceAddr, uint8_t groupNum) {
  uint8_t r;
  uint8_t byteaddr = (deviceAddr << 1) + 1;
  uint8_t command = (uint8_t)(0x6 << 4) + (uint8_t)groupNum;
  dali.transmit(byteaddr, command);
  delay(10);
  dali.transmit(byteaddr, command);
  delay(10);
}

void RemoveFromGroup(uint8_t deviceAddr, uint8_t groupNum) {
  uint8_t r;
  uint8_t byteaddr = (deviceAddr << 1) + 1;
  uint8_t command = (uint8_t)(0x7 << 4) + (uint8_t)groupNum;
  dali.transmit(byteaddr, command);
  delay(10);
  dali.transmit(byteaddr, command);
  delay(10);
}

void SetDimLevel(TUseGroup UseGroup, byte Adress, byte Power) {
  byte byteaddr = (UseGroup * 128) + (Adress << 1);
  dali.transmit(byteaddr, Power);
}

void SetShortAddress(uint8_t curNum, uint8_t desiredNum) {
  dali.transmit(0xA1, 0x00);
  delay(10);

  uint8_t desiredAddr = (desiredNum << 1) + 1;
  dali.transmit(0xA3, desiredAddr);
  delay(10);

  uint8_t byteaddr = (curNum << 1) + 1;
  dali.transmit(byteaddr, 0x80);
  delay(10);
  dali.transmit(byteaddr, 0x80);
}

void DeleteShortAddress(uint8_t curNum) {
  dali.transmit(0xA1, 0x00);
  delay(10);

  dali.transmit(0xA3, 0xFF);
  delay(10);

  uint8_t byteaddr = (curNum << 1) + 1;
  dali.transmit(byteaddr, 0x80);
  delay(10);
  dali.transmit(byteaddr, 0x80);
}

void Extend() {
  const int delaytime = 10;

  long low_longadd = 0x000000;
  long high_longadd = 0xFFFFFF;
  long longadd = (long)(low_longadd + high_longadd) / 2;
  uint8_t highbyte;
  uint8_t middlebyte;
  uint8_t lowbyte;

  dali.transmit(0xA5, 0x00);
  delay(10);
  dali.transmit(0xA5, 0x00);
  delay(10);

  int resp;
  for (int i = 0; i < 64; ++i) {
    resp = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0x91);
    if (resp == 0xFF) {
      int Hbyte = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xC2);
      int Mbyte = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xC3);
      int Lbyte = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xC4);

      dali.transmit(0xC1, 0x08);
      delay(10);

      int ver = SendDaliCommandAndRecive(SHORT_ADDRESS, i, 0xFF);
    }
  }
  dali.transmit(0xA5, 0xFF);
  delay(10);
  dali.transmit(0xA5, 0xFF);
  delay(10);
  dali.transmit(0xA1, 0x00);
  delay(10);
  dali.transmit(0xA5, 0xFF);
  delay(10);
  dali.transmit(0xA5, 0xFF);
  delay(10);

  dali.transmit(0xA7, 0xFF);
  delay(10);
  dali.transmit(0xA7, 0xFF);
  delay(10);


  while (longadd <= (0xFFFFFF - 2)) {
    while ((high_longadd - low_longadd) > 1) {

      splitAdd(longadd, highbyte, middlebyte, lowbyte);  //divide 24bit adress into three 8bit adresses
      delay(delaytime);

      dali.transmit(0b10110001, highbyte);  //search HB
      delay(delaytime);

      dali.transmit(0b10110011, middlebyte);  //search MB
      delay(delaytime);

      dali.transmit(0b10110101, lowbyte);  //search LB
      delay(delaytime);

      resp = Send_Receive(0b10101001, 0b00000000);

      if (resp == 0) {
        low_longadd = longadd;
      } else {
        high_longadd = longadd;
      }

      longadd = (low_longadd + high_longadd) / 2;  //center

    }  // second while

    // Serial.print("low_addr  ");
    // Serial.println(low_longadd, HEX);
    // Serial.print("high_addr  ");
    // Serial.println(high_longadd, HEX);
    // Serial.print("longaddr  ");
    // Serial.println(longadd, HEX);

    if (high_longadd != 0xFFFFFF) {

      splitAdd(longadd + 1, highbyte, middlebyte, lowbyte);

      dali.transmit(0b10110001, highbyte);  //search HB
      delay(delaytime);

      dali.transmit(0b10110011, middlebyte);  //search MB
      delay(delaytime);

      dali.transmit(0b10110101, lowbyte);  //search LB
      delay(delaytime);

      int freeAddr;
      for (int i = 0; i < 64; ++i) {
        if (DaliDevice[i].Detected == 0) {
          freeAddr = i;
          DaliDevice[i].Detected = 1;
          break;
        }
      }
      dali.transmit(0b10110111, 1 + (freeAddr << 1));  //program short adress
      delay(delaytime);
      Serial.println(freeAddr);

      //tut
      dali.transmit(0b10111011, 0b00000000);  //BB00
      delay(delaytime);

      resp = Send_Receive(1 + (freeAddr << 1), 0b11000010);
      delay(delaytime);
      resp = Send_Receive(1 + (freeAddr << 1), 0b11000011);
      delay(delaytime);
      resp = Send_Receive(1 + (freeAddr << 1), 0b11000100);
      delay(delaytime);

      dali.transmit(0b10101011, 0b00000000);  //withdraw
      delay(delaytime);

      // dali.transmit(1 + (short_add << 1), ON_C);
      // delay(1000);
      // dali.transmit(1 + (short_add << 1), OFF_C);
      // delay(delaytime);

      dali.transmit(0b11000001, 0x08);
      delay(delaytime);

      resp = Send_Receive(1 + (freeAddr << 1), 0xFF);
      delay(delaytime);

      high_longadd = 0xFFFFFF;
      longadd = (low_longadd + high_longadd) / 2;

    } else {
      if (dali.msgMode) {
        Serial.println("End");
      }
    }
  }  // first while
  dali.transmit(0b10100001, 0b00000000);
}

void CheckSerial() {
  byte serialbufflength = Serial.available();
  byte start_byte_flag = 0;
  byte stop_byte_flag = 0;
  char buff[14];

  if (serialbufflength >= 16) {  // если пришло 16 байт
    for (int i = 0; i < serialbufflength; i++) {
      if (Serial.read() == 0xAB) {
        start_byte_flag = true;
        break;
      }
    }
    if (start_byte_flag == true) {
      Serial.readBytes(buff, 14);
      if (Serial.read() == 0xBA) {
        stop_byte_flag = true;
      }
    }

    if (start_byte_flag && stop_byte_flag) {

      if (buff[0] == SCAN) {
        initialise();
        delay(50);
        DetectDevices();
        delay(50);
        Respond_DETECT();
      }

      else if (buff[0] == DETECT) {
        DetectDevices();
        delay(50);
        Respond_DETECT();
      }

      else if (buff[0] == ON) {

        if (buff[1] == SingleDevice) {
          SendDaliCommand(SHORT_ADDRESS, buff[2], RECALL_MAX_LEVEL);
        } else if (buff[1] == GroupDevice) {
          SendDaliCommand(GROUP_ADDRESS, buff[2], RECALL_MAX_LEVEL);
        } else if (buff[1] == Broadcast) {
          SendDaliCommand(GROUP_ADDRESS, 63, RECALL_MAX_LEVEL);
        }
        Respond_OK();
      }

      else if (buff[0] == OFF) {

        if (buff[1] == SingleDevice) {
          SendDaliCommand(SHORT_ADDRESS, buff[2], RECALL_OFF);
        } else if (buff[1] == GroupDevice) {
          SendDaliCommand(GROUP_ADDRESS, buff[2], RECALL_OFF);
        } else if (buff[1] == Broadcast) {
          SendDaliCommand(GROUP_ADDRESS, 63, RECALL_OFF);
        }
        Respond_OK();
      }

      else if (buff[0] == ADD_TO_GROUP) {
        AddToGroup(buff[1], buff[2]);
        Respond_OK();
      }

      else if (buff[0] == REMOVE_FROM_GROUP) {
        RemoveFromGroup(buff[1], buff[2]);
        Respond_OK();
      }

      else if (buff[0] == SET_DIM_LEVEL) {

        if (buff[1] == SingleDevice) {
          SetDimLevel(SHORT_ADDRESS, buff[2], buff[3]);
        } else if (buff[1] == GroupDevice) {
          SetDimLevel(GROUP_ADDRESS, buff[2], buff[3]);
        } else if (buff[1] == Broadcast) {
          SetDimLevel(GROUP_ADDRESS, 63, buff[3]);
        }
        Respond_OK();
      }

      else if (buff[0] == SET_FADE_TIME) {

        dali.transmit(0xA3, buff[3]);
        delay(10);

        if (buff[1] == SingleDevice) {
          SendDaliCommand(SHORT_ADDRESS, buff[2], 0x2E);
          delay(10);
          SendDaliCommand(SHORT_ADDRESS, buff[2], 0x2E);
        } else if (buff[1] == GroupDevice) {
          SendDaliCommand(GROUP_ADDRESS, buff[2], 0x2E);
          delay(10);
          SendDaliCommand(GROUP_ADDRESS, buff[2], 0x2E);
        } else if (buff[1] == Broadcast) {
          SendDaliCommand(GROUP_ADDRESS, 63, 0x2E);
          delay(10);
          SendDaliCommand(GROUP_ADDRESS, 63, 0x2E);
        }
        Respond_OK();
      }

      else if (buff[0] == SET_FADE_RATE) {

        dali.transmit(0xA3, buff[3]);
        delay(10);

        if (buff[1] == SingleDevice) {
          SendDaliCommand(SHORT_ADDRESS, buff[2], 0x2F);
          delay(10);
          SendDaliCommand(SHORT_ADDRESS, buff[2], 0x2F);
        } else if (buff[1] == GroupDevice) {
          SendDaliCommand(GROUP_ADDRESS, buff[2], 0x2F);
          delay(10);
          SendDaliCommand(GROUP_ADDRESS, buff[2], 0x2F);
        } else if (buff[1] == Broadcast) {
          SendDaliCommand(GROUP_ADDRESS, 63, 0x2F);
          delay(10);
          SendDaliCommand(GROUP_ADDRESS, 63, 0x2F);
        }
        Respond_OK();
      }

      else if (buff[0] == SET_SHORT_ADDRESS) {
        SetShortAddress(buff[1], buff[2]);
        delay(50);
        DetectDevices();
        delay(50);
        Respond_DETECT();
      }

      else if (buff[0] == DELETE_SHORT_ADDRESS) {
        DeleteShortAddress(buff[1]);
        delay(50);
        DetectDevices();
        delay(50);
        Respond_DETECT();
      }

      else if (buff[0] == EXTEND) {
        DetectDevices();
        delay(50);
        Extend();
        delay(50);
        DetectDevices();
        delay(50);
        Respond_DETECT();
      }

      else if (buff[0] == GET_INFO) {
        Respond_INFO(buff[1]);
      }
    }
  }
}





void setup() {
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < 10; ++i) {
    digitalWrite(LED_PIN, HIGH);
    delay(60);
    digitalWrite(LED_PIN, LOW);
    delay(60);
  }

  Serial.begin(115200);
  delay(2000);
  Serial.println("START");
  dali.setupTransmit(DALI_TX);
  dali.setupAnalogReceive(DALI_RX_A);
  dali.busTest();
  dali.msgMode = true;
  Serial.println(dali.analogLevel);
}
void loop() {
  CheckSerial();
  delay(100);
}