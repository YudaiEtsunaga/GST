#include <SPI.h>
#include <LoRa.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_1 7 //pin number where the data line of temp sensor is connected
//Vin - 5V
//GND
//SCK - 13
//MISO - 12
//MOSI - 11
//CS - 10

RTC_DS3231 rtc;
OneWire oneWire_in(ONE_WIRE_BUS_1);
DallasTemperature sensor_inhouse(&oneWire_in);

void createPackt();

uint8_t pckt[16];

int counter = 0;
uint16_t checksum = 0;

uint16_t temp_digital = 0;
uint32_t pckt_address = 0x00000000;  // latest packet address

DateTime now;
uint8_t hr, mn, sec;
uint16_t time;


void setup() {
  Serial.begin(57600);
  while (!Serial);
   
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  //if (! rtc.initialized()) {
    //Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(22, 12, 14, 10, 48, 0));  //sets the RTC with an explicit date and time
    //}

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433.067E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

   LoRa.setTxPower(0,PA_OUTPUT_PA_BOOST_PIN); // transmitting with 0dB power 
   LoRa.setSignalBandwidth(31.25E3);
   LoRa.setSpreadingFactor(10);
   LoRa.setCodingRate4(5);

  pckt[0] = 0xDD; 
  pckt[1] = 0x11; 
  pckt[2] = 0x55;
  pckt[3] = 0x01;


}

void loop() {
  createPackt();

  for(int i =0; i<14 ;i++)
  {
    checksum = checksum + pckt[i];  //checksum は16bit
                                    //16bitの最大値65535(十進法)
                                    //8bitの最大値255(十進法)
                                    //255*14=3570<65535
                                    //checksumは16bitで十分
                                    //checksumはパケット13までの総合計(二進法)
    }

  pckt[14] = (checksum>>8) & 0xff;
  pckt[15] =  checksum & 0xff; 

  checksum = 0;

  //if (batVol >= 0x80)
  //{
  /*Serial.print("Sending packet: ");
  Serial.print(counter);
  Serial.print(":    ");*/
  
  for(int i = 0; i<16;i++)
  {
      Serial.print(pckt[i],HEX);
      Serial.print(" ");
  }

  Serial.println("");
  LoRa.beginPacket();
  LoRa.write(pckt,16);
  LoRa.endPacket();

  delay(3000);

}

void createPackt()
{
  now = rtc.now();
  
  Serial.print(now.unixtime());
  Serial.print("s");
  Serial.println();
  
  hr = now.hour();
  mn = now.minute();
  sec = now.second();
  
  time = (hr << 11) | (mn << 5) | (sec >> 1); //8ビットのhr, mn, secを16ビットに結合
                                              //必要なところだけ残す-> hr, mn, secの最大値はそれぞれ23, 59, 59
                                              //23は二進数で10111(5桁), 59は二進数で111011(6桁)
                                              //8ビットのhrを11左シフトするとtimeは16ビットなので先頭の3ビットは（0成分）は消去されることになる
                                              //minについてもhrと同じ要領
                                              //secは右に1つだけシフト。衛星側はシフトされる前に何があったのかを知ることはできない。よって0か1の2通りを5ビットに結合する必要がある。従って1秒の誤差が生じる
                                              //OR operation:入力の二つのビットがどちらかでも1であれば、出力ビットは1

  //timenow = now.unixtime()-1609459200; //total number of seconds from midnight Jan 1,2021
  //Serial.println(now.unixtime());
  //now.unixtime()- 1577836800;

  pckt[4] = (time >> 8) & 0xff; //& operation: ビット列の一部を取り出す。取り出したい箇所だけ1にする。1－1の時だけ1
                                //HEX:ff = BIN:11111111
                                //& 0xff operationは16ビットのtimeを8ビットにしなければいけないため
                                //右にシフトしたのはhr, mn, secの順にパケットを作りたいため
  pckt[5] = time & 0xff;
    
  //pckt[7] = 4; //battery voltage 8 bits 

  /*sensor_inhouse.requestTemperatures(); 
  temp_digital = 1092*(sensor_inhouse.getTempCByIndex(0)+10);

  Serial.println(temp_digital);

  pckt[6] = (temp_digital>>8)&0xff;
  pckt[7] = temp_digital&0xff;*/
}
