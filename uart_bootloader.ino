#define BOARD_LED_PIN PB14
#define RESTART_PIN PA0

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

  Serial1.print("Erased page  ");
  Serial1.println(pageAddr, HEX);
  return true;
}

// bool flashErasePages(uint32 pageAddr, uint16 n) {
//   while (n-- > 7) {
//     if (!flashErasePage(pageAddr + 0x400 * n)) {
//       return false;
//     }
//     String message = "Page erased " + String(n);
//     Serial1.println(message);
//   }
//   return true;
// }

bool coolEraser(int start, int stop) {
  int counter = start;
  uint32_t curAddress = 0x08000000 + (0x400 * start);
  uint32_t stopAddress = 0x08000000 + (0x400 * stop);
  while (curAddress <= stopAddress) {
    if (!flashErasePage(curAddress)) {
      return false;
    }
    String message = "Page erased " + String(counter);
    Serial1.println(message);
    curAddress += 0x400;
    counter += 1;
  }
  return true;
}

// bool flashWriteWord(uint32 addr, uint32 word) {
//   volatile uint16 *flashAddr = (volatile uint16 *)addr;
//   volatile uint32 lhWord = (volatile uint32)word & 0x0000FFFF;
//   volatile uint32 hhWord = ((volatile uint32)word & 0xFFFF0000) >> 16;

//   uint32 rwmVal = GET_REG(FLASH_CR);
//   SET_REG(FLASH_CR, FLASH_CR_PG);

//   while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
//   *(flashAddr) = (volatile uint16)hhWord;
//   while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
//   *(flashAddr + 0x01) = (volatile uint16)lhWord;
//   while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}

//   rwmVal &= 0xFFFFFFFE;
//   SET_REG(FLASH_CR, rwmVal);

//   if (*(volatile uint32 *)addr != word) {
//     return false;
//   }
//   return true;
// }

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

// bool write_data_to_flash(uint32_t flash_address, uint32_t data) {

//   uint32 rwmVal = GET_REG(FLASH_CR);
//   SET_REG(FLASH_CR, FLASH_CR_PG);

//   while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}  // Ожидание завершения предыдущей операции

//   *((volatile uint32_t *)flash_address) = data;  // Запись данных в Flash-память

//   while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}  // Ожидание завершения операции программирования
//   rwmVal &= 0xFFFFFFFE;
//   SET_REG(FLASH_CR, rwmVal);
//   return true;
// }


static void FLASH_Program_HalfWord(uint32_t Address, uint16_t Data) {

  /* Proceed to program the new data */
  SET_REG(FLASH_CR, FLASH_CR_PG);

  /* Write data in the address */
  *(__IO uint16_t *)Address = Data;

  Serial1.print("Addr  ");
  Serial1.print(Address, HEX);
  Serial1.print("  cluster  ");
  Serial1.println(Data, HEX);
}

bool HAL_FLASH_Program(uint32_t Address, uint32_t Data) {
  uint32 rwmVal = GET_REG(FLASH_CR);

  volatile uint16_t hData = (volatile uint32)Data & 0x0000FFFF;
  volatile uint16_t lData = ((volatile uint32)Data & 0xFFFF0000) >> 16;

  // Serial1.print("Data  ");
  // Serial1.print(Data, HEX);
  // Serial1.print("  lData  ");
  // Serial1.print(lData, HEX);
  // Serial1.print("  hData  ");
  // Serial1.println(hData, HEX);

  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  FLASH_Program_HalfWord(Address, hData);

  while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
  FLASH_Program_HalfWord(Address + 0x2, lData);

  rwmVal &= 0xFFFFFFFE;
  SET_REG(FLASH_CR, rwmVal);

  Serial1.print("test   ");
  Serial1.println((*(volatile uint32 *)Address), HEX);

  return true;
}

bool WriteWordFromOriginal(uint32_t addr, uint32_t word) {
    uint16_t *flashAddr = (uint16_t *)addr;
    uint32_t lhWord = (uint32_t)word & 0x0000FFFF;
    uint32_t hhWord = ((uint32_t)word & 0xFFFF0000) >> 16;

    uint32_t rwmVal = GET_REG(FLASH_CR);
    SET_REG(FLASH_CR, FLASH_CR_PG);

    /* apparently we need not write to FLASH_AR and can
       simply do a native write of a half word */
    while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
    *(flashAddr + 0x01) = (uint16_t)hhWord;
    while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}
    *(flashAddr) = (uint16_t)lhWord;
    while (GET_REG(FLASH_SR) & FLASH_SR_BSY) {}

    rwmVal &= 0xFFFFFFFE;
    SET_REG(FLASH_CR, rwmVal);

    /* verify the write */
    if (*(uint32_t *)addr != word) {
        return false;
    }
    Serial1.print("ADDR  ");
    Serial1.print(addr, HEX);
    Serial1.print("  CLUSTER  ");
    Serial1.println(word, HEX);
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
#define Finished 'F'
#define StartReboot 'X'

//STAGES
#define Init 1
#define WaitingCommand 2
#define Receiving 3
#define Finishing 4
#define WaitingForBegin 5
#define Restarting 6

//ERROR-CODES
#define NoStartCommand 'N'
#define FlashWriteInvalid 'I'

uint32_t STAGE;    //отслеживает текущее состояние у пациента
uint32_t COMMAND;  //доктор управляет пациентом

uint8_t buff[4];
uint32_t cluster;
uint32_t address = 0x08000000;
uint32_t *userSpace = (uint32_t *)(address);
int counter = 0;


void setup() {
  pinMode(BOARD_LED_PIN, OUTPUT);
  Serial1.begin(115200);
  STAGE = WaitingForBegin;
}

int startCounter = 0;


void loop() {

  switch (STAGE) {

    case WaitingForBegin:
      if (startCounter < 10) {
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
      }
      break;


    case Init:
      Serial1.println(START_OK);
      flashUnlock();
      //coolEraser(0, 107);
      Serial1.println(FCleared);
      STAGE = WaitingCommand;
      break;


    case WaitingCommand:

      while (Serial1.available() == 0) {}

      if (Serial1.available() > 0) {
        COMMAND = Serial1.read();

        if (COMMAND == GET4BYTES) {
          STAGE = Receiving;
        } else if (COMMAND == ENDOFFILE) {
          STAGE = Finishing;
        } else {
          Serial1.print("Wrong command in waiting  ");
          //Serial1.println(COMMAND);
        }
      }
      break;


    case Receiving:
      Serial1.println(Ready_for_receiving);

      while (Serial1.available() < 4) {}

      if (Serial1.available() >= 4) {
        if (counter % 1024 == 0) {
          flashErasePage((uint32_t)userSpace);
        }
        Serial1.readBytes(buff, 4);

        cluster = ((uint32_t)buff[3] << 24) + ((uint32_t)buff[2] << 16) + ((uint32_t)buff[1] << 8) + (uint32_t)buff[0];

        if (!WriteWordFromOriginal((uint32_t)userSpace, cluster)) {
          Serial1.println("FAILED TO WRITE");
        } else {
          // Serial1.print("Addr  ");
          // Serial1.print(address, HEX);
          // Serial1.print("  cluster  ");
          // Serial1.println(cluster);
          //address += 0x4;
          userSpace += 1;
          counter += 4;
          Serial1.println(WaitCom);
        }
        STAGE = WaitingCommand;
      }
      break;


    case Finishing:
      flashLock();
      Serial1.println(Finished);
      STAGE = Restarting;
      break;


    case Restarting:
      for (int i = 3; i > 0; --i) {
        Serial1.print("Restart in ");
        Serial1.print(i);
        Serial1.println(" seconds...");
        delay(500);
      }
      Serial1.println(StartReboot);
      Serial1.flush();
      Serial1.end();
      delay(1000);
      while (true) {
        digitalWrite(BOARD_LED_PIN, HIGH);
        delay(100);
        digitalWrite(BOARD_LED_PIN, LOW);
        delay(100);
      }
  }
}