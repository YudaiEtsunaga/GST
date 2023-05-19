#include <SPI.h>
//#include <SoftSPI.h>
#include <LoRa.h>
#include <RTClib.h>
#include <Wire.h>
//#include "Adafruit_SHT31.h"

#define FREQ 433.085E6
#define TX_PW 20
#define BW 31.25E3
#define SF 10
#define CR 8

#define PASS 1674736350 //26 Jan 2023: 13:32:30
#define PASS_1  1674559800 //24 Jan 2023: 11:30:00
#define PASS_2  1674561000 //24 Jan 2023: 11:50:00

//#include <OneWire.h>
//#include <DallasTemperature.h>

//#define ONE_WIRE_BUS_1 7 //pin number where the data line of temp sensor is connected


RTC_DS3231 rtc;
/*OneWire oneWire_in(ONE_WIRE_BUS_1);
DallasTemperature sensor_inhouse(&oneWire_in);*/

uint8_t pckt[10];

uint16_t checksum = 0;
uint16_t counter = 0;

float temp_digital = 0;
uint16_t temp;
//uint32_t pckt_address = 0x00000000;  // latest packet address

DateTime now;
uint8_t hr, min, sec;
uint16_t time;

uint8_t LoRainit(){
  if (!LoRa.begin(FREQ)) {
    Serial.println("Starting LoRa failed!");
  while (1);
  }

  LoRa.setTxPower(TX_PW,PA_OUTPUT_PA_BOOST_PIN); 
  LoRa.setSignalBandwidth(BW);
  LoRa.setSpreadingFactor(SF);
  LoRa.setCodingRate4(CR);

  return 1;
}

uint8_t RTCinit(){

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  /*if (!rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(2022, 12, 20, 11, 30, 0));
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }*/

  //rtc.adjust(DateTime(2023, 01, 13, 19, 18, 0));

  return 1;

}

void setup() {
  Serial.begin(57600);

    /*if (!sht31_44.begin(0x44)) {   // Set to 0x44 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
    }*/


  if(RTCinit() && LoRainit()){
    Serial.println("KITSTUNE Sender initialization success");
  }

  pckt[0] = 0x44; 
  pckt[1] = 0x44; 
  pckt[2] = 0x44;
  pckt[3] = 0x01;

}

void loop() {
  createPackt();

  for(int i =0; i<8 ;i++)
  {
    checksum = checksum + pckt[i];
    }

  pckt[8] = (checksum>>8) & 0xff;
  pckt[9] =  checksum & 0xff; 

  checksum = 0;

  if (now.unixtime() >= PASS_1){

    Serial.print("Sending packet ");
    Serial.print(counter);
    Serial.print(":   ");

    for(int i = 0; i<10;i++)
    {
        Serial.print(pckt[i],HEX);
        Serial.print(" ");
    }
    Serial.println("");
    Serial.println("");

    LoRa.beginPacket();
    LoRa.write(pckt,10);
    LoRa.endPacket();

    counter++;

    if (counter == 5){
      sleep();
    }

    delay(9000);

  }


  if (now.unixtime() < PASS_1) {
    Serial.println("Waiting for KITSUNE pass...");
    delay(1000);
  }


}

void createPackt()
{
  now = rtc.now();
  //Serial.println(now.unixtime());

  hr = now.hour();
  min = now.minute();
  sec = now.second();
  /*
  Serial.println("Createpacket>>>");
  Serial.print("Time: ");

  Serial.print(hr);
    Serial.print(":");

  Serial.print(min);
    Serial.print(":");

  Serial.println(sec);
  */
  
  time = (hr << 11) | (min << 5) | (sec >> 1);


  pckt[4] = (time >> 8);
  pckt[5] = time & 0xff;
    

  temp_digital = rtc.getTemperature();
  /*
  Serial.print("Temp: ");
  Serial.print(temp_digital);
  Serial.println(" ℃");
  */
  temp = temp_digital*100;  //Resolusion of temperature sensor on rtc is 0.25℃

  pckt[6] = (temp >> 8) & 0xff;
  pckt[7] = temp & 0xff;

}

void sleep()
{
  for (int l=0; l<10; l++){
    Serial.println("Waiting for the next passage");
    delay(1000);
  }
}
