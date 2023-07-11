#include "se8r01.h"
const int NUM_SLIDERS = 5;
//***************************************************
#define TX_ADR_WIDTH    4   // 5 unsigned chars TX(RX) address width
#define TX_PLOAD_WIDTH  NUM_SLIDERS*2  // 32 unsigned chars TX payload
//***************************************************

byte gtemp[5];
byte k=0;

unsigned char TX_ADDRESS[TX_ADR_WIDTH] = {0x10,0x15,0x25,0x44}; // Unique address

unsigned char rx_buf[TX_PLOAD_WIDTH] = {0};

void setup() {
  pinMode(CEq,  OUTPUT);
  pinMode(SCKq, OUTPUT);
  pinMode(CSNq, OUTPUT);
  pinMode(MOSIq,  OUTPUT);
  pinMode(MISOq, INPUT);
  pinMode(IRQq, INPUT);

  Serial.begin(9600);
  init_io();                        // Initialize IO port
  unsigned char status=SPI_Read(STATUS);
  Serial.print("status = ");    
  Serial.println(status,HEX);     
  Serial.println("*******************Radio starting*****************");
  
  digitalWrite(CEq, 0);
  delay(1);
  se8r01_powerup();
  se8r01_calibration();
  se8r01_setup();
  radio_settings();
  SPI_RW_Reg(WRITE_REG|iRF_BANK0_CONFIG, 0x3f);
  Serial.println("*****************Pairing device*******************");
  digitalWrite(CEq, 1);
}

void loop() {
  if(digitalRead(IRQq)==LOW)
    {
    delay(1);      //read reg too close after irq low not good
    unsigned char status = SPI_Read(STATUS);  
  
    if(status&STA_MARK_RX)                                           // if receive data ready (TX_DS) interrupt
    {
      String builtString = String("");
      SPI_Read_Buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);             // read playload to rx_buf
      SPI_RW_Reg(FLUSH_RX,0); // clear RX_FIFO
      for(byte i=0; i<TX_PLOAD_WIDTH; i+=2)
      {
        int integerValue = (rx_buf[i] << 8) | rx_buf[i+1];
        builtString += integerValue;
        if (i+2<TX_PLOAD_WIDTH)
        {
          builtString += String("|");
        }
      }
      Serial.println(builtString);
      }
     SPI_RW_Reg(WRITE_REG+STATUS,0xff);
  }
  delay(1);
}
