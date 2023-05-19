#include <SPI.h>
//#include <SoftSPI.h>
#include <LoRa.h>
#include <RTClib.h>
#include <Wire.h>


#define FREQ 433.085E6
#define TX_PW 20
#define BW 31.25E3
#define SF 10
#define CR 8

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_1 7 //pin number where the data line of temp sensor is connected

#define period 9000


RTC_DS3231 rtc;

OneWire oneWire_in(ONE_WIRE_BUS_1);
DallasTemperature temp_sensor(&oneWire_in);

uint8_t pckt[16];

uint16_t checksum = 0;
uint16_t counter = 0;

uint32_t startMillis;  //some global variables available anywhere in the program
uint32_t currentMillis;

uint16_t temp_digital = 0;

DateTime now;
uint8_t hr, min, sec;
uint16_t time;

/*
uint8_t tempinit(){
  Serial.print("Initialiting temperature sensor =====> ");

  temp_sensor.begin();
  
  
  if (!lps35hw.begin_I2C()) {
    Serial.println("Couldn't find LPS35HW chip");
    while (1);
  }

  Serial.println("Success");
  return 1;
}
*/

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
  temp_sensor.begin();

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

  startMillis = millis();
  
}

void loop() {

  currentMillis = millis();
  
  if (currentMillis - startMillis >= period){
    createPackt();
  
    for(int i =0; i<14 ;i++)
    {
      checksum = checksum + pckt[i];
      }

    pckt[14] = (checksum>>8) & 0xff;
    pckt[15] =  checksum & 0xff; 
  
    checksum = 0;


    Serial.print("Sending packet ");
    Serial.print(counter);
    Serial.print(":   ");
    for(int i = 0; i<16;i++)
      {
          Serial.print(pckt[i],HEX);
          Serial.print(" ");
      }
    Serial.println("");
    Serial.println("");
  
    LoRa.beginPacket();
    LoRa.write(pckt,16);
    LoRa.endPacket();
  
    counter++;
    startMillis = currentMillis;
  }
}

void createPackt()
{
  now = rtc.now();
  //Serial.printl(now.unixtime());

  hr = now.hour();
  min = now.minute();
  sec = now.second();

  Serial.print("Time: ");

  Serial.print(hr);
    Serial.print(":");

  Serial.print(min);
    Serial.print(":");

  Serial.println(sec);

  
  time = (hr << 11) | (min << 5) | (sec >> 1);


  pckt[4] = (time >> 8);
  pckt[5] = time & 0xff;
    

  //temp_digital = rtc.getTemperature();
 
  temp_sensor.requestTemperatures();
  temp_digital = 100*(temp_sensor.getTempCByIndex(0));

  Serial.print("Temp: ");
  Serial.print(temp_digital/100.);
  Serial.println(" ℃");

  //temp = temp_digital*100;  //Resolusion of temperature sensor on rtc is 0.25℃

  pckt[6] = (temp_digital >> 8) & 0xff;
  pckt[7] = temp_digital & 0xff;

  pckt[8]= 0;
  pckt[9]= 0;
  pckt[10]= 0;
  pckt[11]= 0;
  pckt[12]= 0;
  pckt[13] =0;
  
 
  

}
