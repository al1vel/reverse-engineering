#define BOARD_LED_PIN PB14
#define RESTART_PIN PA0
#define USER_CODE_FLASH0x0801B000 0x0801B000

#define BOOTLOADER_FLASH ((uint32)0x08000000)

#define SET_REG(addr, val) \
  do { *(volatile uint32 *)(addr) = val; } while (0)
#define GET_REG(addr) (*(volatile uint32 *)(addr))

#define FLASH ((uint32)0x40022000)

#define FLASH_ACR (FLASH + 0x00)
#define FLASH_KEYR (FLASH + 0x04)
#define FLASH_OPTKEYR (FLASH + 0x08)
#define FLASH_SR (FLASH + 0x0C)
#define FLASH_CR (FLASH + 0x10)
#define FLASH_AR (FLASH + 0x14)
#define FLASH_OBR (FLASH + 0x1C)
#define FLASH_WRPR (FLASH + 0x20)

#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xCDEF89AB
#define FLASH_RDPRT 0x00A5
#define FLASH_SR_BSY 0x01
#define FLASH_CR_PER 0x02
#define FLASH_CR_PG 0x01
#define FLASH_CR_START 0x40

#define SCS 0xE000E000
#define SCB (SCS + 0xD00)
#define SCB_VTOR (SCB + 0x08)
#define STK (SCS + 0x10)
#define STK_CTRL (STK + 0x00)

bool flashErasePage(uint32 pageAddr) {
  uint32 rwmVal = GET_REG(FLASH_CR);
  rwmVal = FLASH_CR_PER;
  SET_REG(FLASH_CR, rwmVal);

  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  SET_REG(FLASH_AR, pageAddr);
  SET_REG(FLASH_CR, FLASH_CR_START | FLASH_CR_PER);
  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}

  rwmVal = 0x00;
  SET_REG(FLASH_CR, rwmVal);

  // Serial1.print("Erased page  ");
  // Serial1.println(pageAddr, HEX);
  return true;
}

bool coolEraser(int start, int stop) {
  int counter = start;
  uint32_t curAddress = 0x08000000 + (0x400 * start);
  uint32_t stopAddress = 0x08000000 + (0x400 * stop);
  while (curAddress <= stopAddress) {
    if (!flashErasePage(curAddress)) {
      return false;
    }
    //String message = "Page erased " + String(counter);
    //Serial1.println(message);
    curAddress += 0x400;
    counter += 1;
  }
  return true;
}

void flashLock() {
  SET_REG(FLASH_CR, 0x00000080);
}

void flashUnlock() {
  SET_REG(FLASH_KEYR, FLASH_KEY1);
  SET_REG(FLASH_KEYR, FLASH_KEY2);
}

void RestartDevice() {
  digitalWrite(BOARD_LED_PIN, HIGH);
  pinMode(RESTART_PIN, OUTPUT);
  digitalWrite(RESTART_PIN, LOW);
  delay(100);
  digitalWrite(RESTART_PIN, HIGH);
}

void setMspAndJump(uint32 usrAddr) {
  typedef void (*funcPtr)(void);
  uint32 jumpAddr = *(uint32 *)(usrAddr + 0x04);
  funcPtr usrMain = (funcPtr)jumpAddr;
  SET_REG(SCB_VTOR, (uint32)(usrAddr));
  asm volatile("msr msp, %0" ::"g"(*(volatile uint32 *)usrAddr));
  usrMain(); /* go! */
}

bool WriteWordFromOriginal(uint32_t addr, uint32_t word) {
  uint16_t *flashAddr = (uint16_t *)addr;
  uint32_t lhWord = (uint32_t)word & 0x0000FFFF;
  uint32_t hhWord = ((uint32_t)word & 0xFFFF0000) >> 16;

  uint32_t rwmVal = GET_REG(FLASH_CR);
  SET_REG(FLASH_CR, FLASH_CR_PG);

  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  *(flashAddr + 0x01) = (uint16_t)hhWord;
  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  *(flashAddr) = (uint16_t)lhWord;
  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}

  rwmVal &= 0xFFFFFFFE;
  SET_REG(FLASH_CR, rwmVal);

  if (*(uint32_t *)addr != word) {
    return false;
  }

  // Serial1.print("ADDR  ");
  // Serial1.print(addr, HEX);
  // Serial1.print("  CLUSTER  ");
  // Serial1.println(word, HEX);
  return true;
}

bool WriteWord(uint32_t addr, uint8_t *buffer) {
  uint16_t *flashAddr = (uint16_t *)addr;
  uint32_t hhWord = ((uint32_t)buffer[3] << 8) + (uint32_t)buffer[2];
  uint32_t lhWord = ((uint32_t)buffer[1] << 8) + (uint32_t)buffer[0];

  uint32_t rwmVal = GET_REG(FLASH_CR);
  SET_REG(FLASH_CR, FLASH_CR_PG);

  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  *(flashAddr + 0x01) = (uint16_t)hhWord;
  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  *(flashAddr) = (uint16_t)lhWord;
  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}

  rwmVal &= 0xFFFFFFFE;
  SET_REG(FLASH_CR, rwmVal);

  // if (*(uint32_t *)addr != word) {
  //     return false;
  // }

  // Serial1.print("ADDR  ");
  // Serial1.print(addr, HEX);
  // Serial1.print("  CLUSTER  ");
  // Serial1.print(hhWord, HEX);
  // Serial1.print("  ");
  // Serial1.println(lhWord, HEX);
  return true;
}


//COMMANDS
#define START 'S'
#define START_OK 'P'
#define ENDOFFILE 'E'
#define FCleared 'C'
#define Ready_for_receiving 'R'
#define WaitCom 'W'
#define GET4BYTES 'G'
#define GET64BYTES 'A'
#define Finished 'F'
#define StartReboot 'X'

//STAGES
#define Init 1
#define WaitingCommand 2
#define Receiving4 3
#define Receiving64 4
#define Finishing 5
#define WaitingForBegin 6
#define Restarting 7

//ERROR-CODES
#define NoStartCommand 'N'
#define FlashWriteInvalid 'I'

uint8_t STAGE;
uint8_t COMMAND;

uint8_t buff64[64];
uint8_t buff4[4];

uint32_t address = 0x0801B000;
uint32_t *userSpace = (uint32_t *)(address);

int startCounter = 0;


void setup() {
  pinMode(BOARD_LED_PIN, OUTPUT);
  Serial1.begin(115200);
  STAGE = WaitingForBegin;
}


void loop() {

  switch (STAGE) {

    case WaitingForBegin:
      {
        if (startCounter < 5) {
          digitalWrite(BOARD_LED_PIN, HIGH);
          delay(100);
          digitalWrite(BOARD_LED_PIN, LOW);
          delay(1000);

          if (Serial1.available() > 0) {
            COMMAND = Serial1.read();
            if (COMMAND == START) {
              STAGE = Init;
            } else {
              Serial1.println("Wrong command in beginning");
            }
          } else {
            Serial1.println(NoStartCommand);
            startCounter += 1;
          }
        } else {
          setMspAndJump(USER_CODE_FLASH0x0801B000);
        }
        break;
      }

    case Init:
      {
        Serial1.println(START_OK);
        flashUnlock();
        coolEraser(108, 127);
        Serial1.println(FCleared);
        STAGE = WaitingCommand;
        break;
      }


    case WaitingCommand:
      {
        while (Serial1.available() < 1) {}

        COMMAND = Serial1.read();

        if (COMMAND == GET4BYTES) {
          STAGE = Receiving4;
        } else if (COMMAND == GET64BYTES) {
          STAGE = Receiving64;
        } else if (COMMAND == ENDOFFILE) {
          STAGE = Finishing;
        } else {
          Serial1.print("Wrong command in waiting  ");
          Serial1.println(COMMAND, HEX);
        }
        break;
      }


    case Receiving4:
      {
        Serial1.println(Ready_for_receiving);

        while (Serial1.available() < 4) {}

        Serial1.readBytes(buff4, 4);

        if (!WriteWord((uint32_t)userSpace, buff4)) {
          Serial1.println("FAILED TO WRITE");
        } else {
          userSpace += 1;
          Serial1.println(WaitCom);
        }
        STAGE = WaitingCommand;
        break;
      }


    case Receiving64:
      {
        Serial1.println(Ready_for_receiving);

        while (Serial1.available() < 16) {}

        Serial1.readBytes(buff64, 64);

        uint8_t *buffer_pointer = buff64;
        for (int i = 0; i < 16; ++i) {
          WriteWord((uint32_t)userSpace, buffer_pointer);
          userSpace += 1;
          buffer_pointer += 4;
        }
        STAGE = WaitingCommand;
        Serial1.println(WaitCom);
        break;
      }


    case Finishing:
      {
        flashLock();
        Serial1.println(Finished);
        STAGE = Restarting;
        break;
      }


    case Restarting:
      {
        for (int i = 3; i > 0; --i) {
          Serial1.print("Restart in ");
          Serial1.print(i);
          Serial1.println(" seconds...");
          delay(1000);
        }
        Serial1.println(StartReboot);
        Serial1.flush();
        Serial1.end();
        RestartDevice();
      }
  }
}