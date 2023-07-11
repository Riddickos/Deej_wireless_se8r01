#include "se8r01.h"
byte gtemp[5];
byte k=0;
const int NUM_SLIDERS = 5;

const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A4};
int analogSliderValues[NUM_SLIDERS];
//***************************************************
#define TX_ADR_WIDTH    4   // 5 unsigned chars TX(RX) address width
#define TX_PLOAD_WIDTH  NUM_SLIDERS*2  // 32 unsigned chars TX payload

unsigned char TX_ADDRESS[TX_ADR_WIDTH] = {0x10,0x15,0x25,0x44}; // Unique address

unsigned char tx_buf[TX_PLOAD_WIDTH] = {0};
//***************************************************

void setup() {
  pinMode(CEq,  OUTPUT);
  pinMode(SCKq, OUTPUT);
  pinMode(CSNq, OUTPUT);
  pinMode(MOSIq,  OUTPUT);
  pinMode(MISOq, INPUT);
  pinMode(IRQq, INPUT);
  
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

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
  SPI_RW_Reg(WRITE_REG|iRF_BANK0_CONFIG, 0x3E);
  Serial.println("************Transmitter initialized***************");
  digitalWrite(CEq, 1);
}

void loop() {
  if (readSliderValues()){
    convertValueToBuffer();
    transmitData();
  }
  delay(50);
}

bool readSliderValues() {
  int slidersThrottling = 5;
  bool dataChanged = false;
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int sliderValue = analogRead(analogInputs[i]);
    if (analogSliderValues[i] >= sliderValue + slidersThrottling || analogSliderValues[i] <= sliderValue - slidersThrottling){
      analogSliderValues[i] = sliderValue;
      Serial.print("Slider #" + String(i) + String(": ") + String(sliderValue) + " ");
      dataChanged = true;
    }
  }
  if (dataChanged)
    Serial.println("");
  return dataChanged;
}

void convertValueToBuffer() {
  int j = 0;
  for (int i = 0; i < NUM_SLIDERS; i++) {
    unsigned char highByte = (analogSliderValues[i] >> 8) & 0xFF; // Extract the high byte
    unsigned char lowByte = analogSliderValues[i] & 0xFF; // Extract the low byte
    tx_buf[j]= highByte;
    tx_buf[j+1]= lowByte;
    j+=2;
  }
}

void transmitData() {
  SPI_RW_Reg(FLUSH_TX, 0);
  SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);     
  SPI_RW_Reg(WRITE_REG+STATUS,0xff);   // clear RX_DR or TX_DS or MAX_RT interrupt flag
}
