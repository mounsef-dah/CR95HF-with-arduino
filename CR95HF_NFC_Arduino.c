#include <SPI.h>
#include <stdio.h>
#include <stdlib.h>

const int SPI_SS = 10;
const int IRQ_IN = 8;
const int SSI_0 = 9;

void resetCR95HF(){
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x01); //control bit for Reset command
  digitalWrite(SPI_SS, HIGH);
}

void CR95HFDLL_STCMD(uint8_t DH){
      digitalWrite(SPI_SS, LOW);
      SPI.transfer(0x00); //control bit for SEND command
      SPI.transfer(0x07); //command code for Idle
      SPI.transfer(0x0E); //data lenght
      //-------------------
      SPI.transfer(0x03); //WU source pour Timeout ou la detection du tag
      SPI.transfer(0xA100); //Enter Control
      SPI.transfer(0xD801); //WU Control
      SPI.transfer(0x1800); //Leave Control
      SPI.transfer(0x01); //WU Period
      SPI.transfer(0x60); //OSC Start
      SPI.transfer(0x60); //DAC Start
      SPI.transfer(0x00); //DacDatal
      SPI.transfer(DH); //DacDataH /*/*/*/*/
      SPI.transfer(0x3F); //Swing Count
      SPI.transfer(0x00); //Max Sleep
      digitalWrite(SPI_SS, HIGH);
}

uint8_t Calibration_TAG(){
  digitalWrite(SPI_SS, LOW);
  uint8_t DacDataL = 0x00;
  uint8_t DacDataH = 0x00;
  uint8_t DacDataRef = 0x00;
  uint8_t Wake_up_event = 0x00;
  //Debut
  //step 0
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  Wake_up_event = SPI.transfer(0x00);
  if(Wake_up_event != 0x000102){
      Serial.print("\nERROR 1");
      return;
    }
  //step 1
  DacDataH = 0xFC;
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  Wake_up_event = SPI.transfer(0x00);
  if(Wake_up_event != 0x000101){
      Serial.print("\nERROR 2");
      return;
    }
  //step 2
  DacDataH = DacDataH;
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  if(Wake_up_event == 0x000101){
      DacDataH=DacDataH-0x80;
    }
  Wake_up_event = SPI.transfer(0x00);
  //step 3
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  if(Wake_up_event == 0x000101){
      DacDataH=DacDataH-0x40;
    }
  else{
      DacDataH=DacDataH+0x40;
  }
  Wake_up_event = SPI.transfer(0x00);
  //step 4
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  if(Wake_up_event == 0x000101){
      DacDataH=DacDataH-0x20;
    }
  else{
      DacDataH=DacDataH+0x20;
  }
  Wake_up_event = SPI.transfer(0x00);
  //step 5
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  if(Wake_up_event == 0x000101){
      DacDataH=DacDataH-0x10;
    }
  else{
      DacDataH=DacDataH+0x10;
  }
  Wake_up_event = SPI.transfer(0x00);
  //step 6
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  if(Wake_up_event == 0x000101){
      DacDataH=DacDataH-0x80;
    }
  else{
      DacDataH=DacDataH+0x80;
  }
  Wake_up_event = SPI.transfer(0x00);
  //step 7
  CR95HFDLL_STCMD(DacDataH);
  Polling_CR95HF();
  if(Wake_up_event == 0x000101){
      DacDataH=DacDataH-0x40;
    }
  else{
      DacDataH=DacDataH+0x40;
  }
  Wake_up_event = SPI.transfer(0x00);
  //Fin
  if(Wake_up_event == 0x000102){
      DacDataRef = DacDataH;
    }
  else if(Wake_up_event == 0x000101){
      DacDataRef = DacDataH - 4;
  }
  digitalWrite(SPI_SS, LOW);
  return DacDataRef;
}

void startupSequence(){
  digitalWrite(SSI_0,HIGH);
  delay(10);
  digitalWrite(IRQ_IN,HIGH);
  delayMicroseconds(10);
  digitalWrite(IRQ_IN,LOW);
  delayMicroseconds(100);
  digitalWrite(IRQ_IN,HIGH);
  delay(1000);
}

void checkCommunicationECHO(){
  //Host to CR95HF
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x00); //control bit for send 
  SPI.transfer(0x55);
  digitalWrite(SPI_SS, HIGH);

  //CR95HFF to host
  uint8_t response = 0;
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x02); //control bit for read
  response = SPI.transfer(0x00);
  digitalWrite(SPI_SS, HIGH);
  if (int(response) == 85) {
    Serial.print("\ngood communication"); 
  }   
  else{
    Serial.print("\nbad communication");    
  }
}

void getInfo_CR95HF(){
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x00); //control bit for send 
  SPI.transfer(0x01); //commande code pour IDN
  SPI.transfer(0x00); // longueur du data 
  digitalWrite(SPI_SS, HIGH);
  Polling_CR95HF();   // wait for response
  Serial.print("\nInformation sur la CR95HF : \n");
  ReadData_CR95HF(); //read data
}

void MonProtocol(int choix){
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x00); //control bit send
  SPI.transfer(0x02); //protocol command select
  SPI.transfer(0x02); // <len> of protocol

	switch(choix){    //<protocol> and <parameters>
    case 0:         //FieldOff
        SPI.transfer(0x01); //<protocol> code for each protocol type
        SPI.transfer(0x01); //<parameters> code for each protocol type 01 02 03 04 05 07 21...
      break;    
    case 1:         //ISO15693
        SPI.transfer(0x01); //<protocol> code for each protocol type
        SPI.transfer(0x01); //<parameters> code for each protocol type 01 02 03 04 05 07 21...
      break;
    case 2:         //ISO14443-A
        SPI.transfer(0x02); 
        SPI.transfer(0x00); //<parameters> code for each protocol type 106 Kbps TR/RC time for this example
      break;
    case 3:         //ISO14443-B
        SPI.transfer(0x03);
        SPI.transfer(0x01); //<parameters> code for each protocol type Type B tag with CRC append
      break;
    case 4:         //ISO18092
        SPI.transfer(0x04);
        SPI.transfer(0x51); //<parameters> code for each protocol for this exampnle we are using 212Kbps TR/RC rates with CRC appended
      break;
    default:        //ISO15693
        SPI.transfer(0x01); //<protocol> code for each protocol type
        SPI.transfer(0x01); //<parameters> code for each protocol type 01 02 03 04 05 07 21...
        Serial.print("le protocol par defaut ISO15693\n");
  digitalWrite(SPI_SS, HIGH);
  }
}

void Polling_CR95HF(){ 
  uint8_t flag = 0x00;
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x03); //control bit for poll 
  while(!((flag & (1<<3))>>3)){
    flag = SPI.transfer(0x03); //polling control byte searching for a flag
    }
  digitalWrite(SPI_SS, HIGH);
}

void ReadData_CR95HF(){
  digitalWrite(SPI_SS, LOW);
  SPI.transfer(0x02); // control byte for READ
  uint8_t RespCode = SPI.transfer(0x00);
  uint8_t len = SPI.transfer(0x00);
  for (int i = 0; i < len-3; i++) {
    uint8_t data = SPI.transfer(0x00);
    Serial.print(char(data));
  }
  digitalWrite(SPI_SS, HIGH);
}

void SendReceive(){
  digitalWrite(SPI_SS,LOW);
  SPI.transfer(0x00);
  SPI.transfer(0x04);
  SPI.transfer(0x07); // <len> of entire data field
  SPI.transfer(0x079370800F8C8E); //notre data
  SPI.transfer(0x28); //transmision flags
	digitalWrite(SPI_SS,HIGH);

	Polling_CR95HF(); // our polling function
	
  digitalWrite(SPI_SS,LOW);
	SPI.transfer(0x02); //read control byte
  uint8_t RespCode = SPI.transfer(0x00);
  uint8_t len = SPI.transfer(0x00);
  Serial.print("\nla valeur de len est :");
  Serial.print(len);
  Serial.print("\n");
  uint8_t data = 0x00;
  for (int i = 0; i < len; i++) {
    data = SPI.transfer(0x00);
    Serial.print(data);
  }
  uint8_t TagDt = SPI.transfer(0x00);
  if ((TagDt & (1<<7))>>7){
    Serial.print("\ncolission detecter");
  }
  digitalWrite(SPI_SS,HIGH);
}

void setup() {

  pinMode(SPI_SS, OUTPUT);
  pinMode(IRQ_IN, OUTPUT);

  Serial.begin(9600);
  digitalWrite(IRQ_IN, HIGH);
  digitalWrite(SPI_SS, HIGH);
  
  startupSequence();
  SPI.begin();
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  
  
  getInfo_CR95HF();
  //MonProtocol(2);
  //SendReceive();
  //checkCommunicationECHO();
  //uint8_t mytag=0x00;
  //mytag = Calibration_TAG();
  //Serial.print("\n");
  //Serial.print(mytag);
  resetCR95HF();
}

void loop() {
}

