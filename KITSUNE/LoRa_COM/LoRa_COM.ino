#include <SPI.h>
#include <SoftSPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <RTClib.h>
//#include "DigitalIO.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_1 7 //pin number where the data line of temp sensor is connected
//#define ONE_WIRE_BUS_2 4

OneWire oneWire_in(ONE_WIRE_BUS_1);
DallasTemperature sensor_inhouse(&oneWire_in);

uint16_t temp_digital = 0;

//Creating a new SPI Port with
//Pin 25 = D4 = MOSI; match with arduino micro pin out
//Pin 27 = D6 = MISO;
//Pin 26 = D12 =SCK;
SoftSPI spi(4, 6, 12);
int CS_FM = 11; 


#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

RTC_PCF8523 rtc;
void createPackt();
uint8_t pckt[16] ;
uint32_t timenow = 0;
int counter = 0;
int timer = 0;
int timer_flash_updte = 0;
int batVol = 0;
int spVol = 0;
uint32_t pckt_address = 0x00000000;  // latest packet address
uint32_t data_address[4] ;
uint32_t data_address2[4] ;
uint16_t checksum = 0;
uint16_t SC = 0;
uint16_t RC = 0;


void setup () {

sensor_inhouse.begin();
//pckt_address = 0x00010000;
//pckt[0] = 0x01; // GST ID - kyutech
//pckt[1] = 0x01; // GST ID - GST no 1
//pckt[2] = 0x01; // Data type - temp sensor


spi.begin();
pinMode(CS_FM,OUTPUT);
//SECTOR_ERASE(0x07ff0000);
//SECTOR_ERASE(0x00000000);
//SECTOR_ERASE(0x00010000);  


if(BYTE_READ(0x07ff0004)!= 0x69) /// undersand here
{
  SECTOR_ERASE(0x00000000);      
}

if(BYTE_READ(0x07ff0004)== 0x69)  /// undersand here,
{
  data_address[0] = BYTE_READ(0x07ff0000);
  delay(1);
  data_address[1] = BYTE_READ(0x07ff0001);
  delay(1);
  data_address[2] = BYTE_READ(0x07ff0002);
  delay(1);
  data_address[3] = BYTE_READ(0x07ff0003);
  delay(1);

  pckt_address = (data_address[0]<<24)|(data_address[1]<<16)|(data_address[2]<<8)|(data_address[3]);
  //pckt_address = 0x00010000;
}


#ifndef ESP8266
 //while (!Serial); 
#endif

  Serial.begin(57600);
  Serial1.begin(9600);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(2021, 1, 16, 0, 0, 0));
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

  Serial.println("LoRa Sender");

  if (!LoRa.begin(405.015E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

   LoRa.setTxPower(0,PA_OUTPUT_PA_BOOST_PIN); // transmitting with 0dB power 
   LoRa.setSignalBandwidth(41.7E3);
   LoRa.setSpreadingFactor(8);
   LoRa.setCodingRate4(8);
}

void loop() 
{ 

    createPackt();
    createpktwRTC();

    for(int i =0; i<14 ;i++)
    {
      checksum = checksum + pckt[i];
     }

    pckt[14] = (checksum>>8) & 0xff;
    pckt[15] =  checksum & 0xff; 

    checksum = 0;

   
    //if (batVol >= 0x80)
    //{
    Serial.print("Sending packet: ");
    Serial.print(counter);
    Serial.print(":    ");
    
    for(int i = 0; i<16;i++)
    {
        Serial.print(pckt[i],HEX);
        Serial.print(" ");
    }
    Serial.println("");
    LoRa.beginPacket();
    LoRa.write(pckt,16);
    LoRa.endPacket();
   
    counter++;
    timer++;
    //SECTOR_ERASE(0x07ff0000);

    if(timer == 6)
    {
      timer_flash_updte++;
      timer = 0;
     
      for(int i = 0; i<16 ;i++)
      {
        BYTE_WRITE(pckt_address + i ,pckt[i]);
        delay(1);
      }
      
      Serial.print("written packet in address ");
      Serial.print(pckt_address,HEX);
      Serial.print(" :>> ");

      
      for(int i =0; i<16 ;i++)
      {
        Serial.print(BYTE_READ(pckt_address+i),HEX);
        Serial.print(" ");
      
      }
      Serial.println("");
      
      
      if(timer_flash_updte == 6)
      {
        timer_flash_updte = 0;
        SECTOR_ERASE(0x07ff0000);

        uint8_t d1 = pckt_address >> 24 & 0xFF;
        uint8_t d2 = pckt_address >> 16 & 0xFF;
        uint8_t d3 = pckt_address >> 8 & 0xFF;
        uint8_t d4 = pckt_address >> 0 & 0xFF;
             
        BYTE_WRITE(0x07ff0000,d1);
        delay(1);
        BYTE_WRITE(0x07ff0001,d2);
        delay(1);
        BYTE_WRITE(0x07ff0002,d3);
        delay(1);
        BYTE_WRITE(0x07ff0003,d4);
        delay(1);
        
        BYTE_WRITE(0x07ff0004,0x69); // this is a flag
        delay(1);

        Serial.print("address data");
        Serial.print(0x07ff0000,HEX);
        Serial.print(" :>> ");
      
        for(int i =0; i<5 ;i++)
        {
          Serial.print(BYTE_READ(0x07ff0000+i),HEX);
          Serial.print(" ");
          delay(1);
         }
       }

        pckt_address = pckt_address+16;
     }

              
   delay(9000); //delay 9s
   
   //checksum = 0;
  //}
}
void createpktwRTC()
{
    DateTime now = rtc.now();
    //Serial.print(now.unixtime());
    //Serial.print("s");
    //Serial.println();
    
    timenow = now.unixtime()-1609459200; //total number of seconds from midnight Jan 1,2021
    //now.unixtime()- 1577836800;
    pckt[3] = (timenow>>24) & 0xff;
    pckt[4] = (timenow>>16) & 0xff;
    pckt[5] = (timenow>>8) & 0xff;
    pckt[6] =  timenow & 0xff;
    
    pckt[10] = (pckt_address>>24) & 0xff;
    pckt[11] = (pckt_address>>16) & 0xff;
    pckt[12] = (pckt_address>>8) & 0xff;
    pckt[13] =  pckt_address & 0xff;

    //checksum = pckt[0]+pckt[1]+pckt[2]+pckt[3]+pckt[4]+pckt[5]+pckt[6]+pckt[7]+pckt[8]+pckt[9]+pckt[10]+pckt[11]+pckt[12]+pckt[13];
//    for(int i =0; i<14 ;i++)
//    {
//      checksum = checksum + pckt[i];
//     }
//    
//    pckt[14] = (checksum>>8) & 0xff;
//    pckt[15] =  checksum & 0xff; 
//
//    checksum = 0;
}

void createPackt()
{
    //Serial.print(pckt_address,HEX);
    SC = analogRead(A5); //solar panel current read
    RC = analogRead(A4); //raw current read    
    batVol = analogRead(A3); //battery voltage read    
    spVol = analogRead(A2); // solar panel voltage    
    
    pckt[0] =  SC; //SC/4; //solar panel current 8 bits
    pckt[1] = RC; //RC/4; //raw panel current 8 bits
    pckt[2] = spVol/4;
    pckt[7] = batVol/4; //battery voltage 8 bits 

    sensor_inhouse.requestTemperatures(); 
    //Serial.print("Inhouse: ");
    temp_digital = 1092*(sensor_inhouse.getTempCByIndex(0)+10);

    pckt[8] = (temp_digital>>8)&0xff;
    pckt[9] = temp_digital&0xff;
}

uint8_t BYTE_READ(uint32_t byte_address)
{

  byte address[4];
  address [0] = byte_address >> 24 & 0xFF;
  address [1] = byte_address >> 16 & 0xFF;
  address [2] = byte_address >>  8 & 0xFF;
  address [3] = byte_address >>  0 & 0xFF;
   
   digitalWrite(CS_FM, LOW);           //lower the CS PIN
   delay(1);
    //////////////////////////////////////////////////////////////////
   uint8_t data;
   spi.transfer(0X13);  //READ DATA COMAND   (0x13)
   spi.transfer(address [0]);    
   spi.transfer(address [1]);    
   spi.transfer(address [2]);    
   spi.transfer(address [3]);
   
   data = spi.transfer(0x00);
   delay(1);
   digitalWrite(CS_FM, HIGH);               //take CS PIN higher back
   
   return data;
}


uint8_t read_id()
{
  digitalWrite(CS_FM,LOW);
  spi.transfer(0X9F);
  uint8_t in = spi.transfer(0X00);
  digitalWrite(CS_FM,HIGH);
  return in;
}

void write_enable(void) {
  
  digitalWrite(CS_FM, LOW);
  spi.transfer(0x06);
  digitalWrite(CS_FM, HIGH);
}

void BYTE_WRITE(uint32_t byte_address,uint8_t data)
{
  byte address[4];
  
  address [0] = byte_address >> 24 & 0xFF;
  address [1] = byte_address >> 16 & 0xFF;
  address [2] = byte_address >>  8 & 0xFF;
  address [3] = byte_address >>  0 & 0xFF;

   
   write_enable();
   
   digitalWrite(CS_FM, LOW);           //lower the CS PIN
   delay(1);
   
   ////////////////////////////////////////////////////////////////
   spi.transfer(0x12); //Byte WRITE COMAND  (0x12)
   spi.transfer(address [0]);    
   spi.transfer(address [1]);    
   spi.transfer(address [2]);    
   spi.transfer(address [3]);
   
   spi.transfer(data);
   delay(1);   
   digitalWrite(CS_FM, HIGH);         //take CS PIN higher back

   return;
}



void SECTOR_ERASE(uint32_t sector_address)
{
   byte address[4];
   
   address[0]  = uint8_t((sector_address>>24) & 0xFF);   // 0x _ _ 00 00 00
   address[1]  = uint8_t((sector_address>>16) & 0xFF);   // 0x 00 _ _ 00 00
   address[2]  = uint8_t((sector_address>>8) & 0xFF);    // 0x 00 00 _ _ 00
   address[3]  = uint8_t((sector_address) & 0xFF);       // 0x 00 00 00 _ _
   
   write_enable();
   
    digitalWrite(CS_FM, LOW);           //lower the CS PIN
    delay(1);
   ///////////////////////////////////////////////////////////////////
   spi.transfer(0XDC);  //SECTOR ERASE COMAND   (0xDC)
   spi.transfer(address [0]);    
   spi.transfer(address [1]);    
   spi.transfer(address [2]);    
   spi.transfer(address [3]);
   //////////////////////////////////////////////////////////////////
   delay(1);
   digitalWrite(CS_FM, HIGH);               //take CS PIN higher back
   delay(1000);  
   return;
}
