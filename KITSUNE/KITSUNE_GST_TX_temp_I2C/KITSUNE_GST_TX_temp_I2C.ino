#include <SPI.h>
#include <LoRa.h>
#include <RTClib.h>
#include <Wire.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include <Adafruit_LPS35HW.h>
//#include <LowPower.h>

#define FREQ 433.085E6
#define TX_PW 20
#define BW 31.25E3
#define SF 10
#define CR 8

#define period 9000

//#define ONE_WIRE_BUS_1 7 

RTC_PCF8523 rtc;

//OneWire oneWire_in(ONE_WIRE_BUS_1);
//DallasTemperature temp_sensor(&oneWire_in);

Adafruit_LPS35HW lps35hw = Adafruit_LPS35HW();

uint8_t pckt[16];

uint16_t checksum = 0;
uint16_t counter = 0;

uint32_t startMillis;  //some global variables available anywhere in the program
uint32_t currentMillis;

uint16_t temp_digital = 0;
uint32_t pressure_d = 0;

//uint32_t pckt_address = 0x00000000;  // latest packet address

DateTime now;
uint8_t hr, min, sec;
uint16_t time;

uint8_t tempinit(){
  Serial.print("Initialiting temperature sensor =====> ");
  
  //temp_sensor.begin();

  if (!lps35hw.begin_I2C()) {
    Serial.println("Couldn't find LPS35HW chip");
    while (1);
  }

  Serial.println("Success");
  return 1;
}

uint8_t LoRainit(){
  Serial.print("Initialiting LoRa =====> ");

  if (!LoRa.begin(FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.print("Setting LoRa parameters =====> ");
  LoRa.setTxPower(TX_PW,PA_OUTPUT_PA_BOOST_PIN); // transmitting with 0dB power 
  LoRa.setSignalBandwidth(BW);
  LoRa.setSpreadingFactor(SF);
  LoRa.setCodingRate4(CR);

  Serial.println("Success");

  return 1;
}

uint8_t RTCinit(){

  Serial.print("Initialiting RTC =====> ");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.initialized()) {
    //Serial.println("RTC is NOT running!");
  }
 /* 
  Serial.print("Setting time =====> ");    
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(2023, 1, 30, 16, 27, 0));
  Serial.println("Success");
  */
  return 1;
}

void setup() {
  Serial.begin(57600);
  //while (!Serial);

  if(tempinit() && RTCinit() && LoRainit() ){
    Serial.println("KITSTUNE GST initialization: success");
  }
  else{
    Serial.println("KITSTUNE GST initialization: failed");
    while(1);
  }

  pckt[0] = 0x44; 
  pckt[1] = 0x44; 
  pckt[2] = 0x44;
  pckt[3] = 0x22;

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
    //LoRa.sleep();

    counter++;
    startMillis = currentMillis;
  }
  //delay(9000);
  //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //LoRa.idle();

}

void createPackt()
{
  now = rtc.now();
  //Serial.println(now.unixtime());

  hr = now.hour();
  min = now.minute();
  sec = now.second();

  Serial.print("Time: ");

  Serial.print(hr);
    Serial.print(':');

  Serial.print(min);
    Serial.print(':');

  Serial.println(sec);

  
  time = (hr << 11) | (min << 5) | (sec >> 1);

  pckt[4] = (time >> 8);
  pckt[5] = time & 0xff;

  
  temp_digital = 100*lps35hw.readTemperature();
  pressure_d = 100*lps35hw.readPressure();

  //temp_sensor.requestTemperatures();
  //temp_digital = 100*(temp_sensor.getTempCByIndex(0));

  Serial.print("Temperature: ");
  Serial.println(temp_digital);

  Serial.print("Pressure: ");
  Serial.println(pressure_d);
  /*Serial.print(" || Temperature RTC: ");
  Serial.println(rtc.getTemperature());*/

  pckt[6] = (temp_digital >> 8) & 0xff;
  pckt[7] = temp_digital&0xff;

  pckt[8] = (pressure_d >> 24) & 0xff;
  pckt[9] = (pressure_d >> 16) & 0xff;
  pckt[10] = (pressure_d >> 8) & 0xff;
  pckt[11] = pressure_d & 0xff;

  pckt[12] = 0;
  pckt[13] = 0;

}
