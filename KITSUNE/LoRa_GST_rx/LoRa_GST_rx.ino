#include <SPI.h>
#include <LoRa.h>

#define FREQ 31.25E3
#define SF 10
#define CR 8

uint8_t pkt[10];
uint8_t rx_valid = 0;
uint16_t cnt = 1;
uint32_t tsincelastpkt = 0;

uint8_t pkt_len;
uint8_t station;
uint8_t sender;
uint8_t data_type;

uint8_t time[3];
float tx_temp;

int packetSize = 0;
uint8_t i = 0;


void LoRaInit(){
  if (!LoRa.begin(433.085E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSignalBandwidth(FREQ);
  LoRa.setSpreadingFactor(SF);
  LoRa.setCodingRate4(CR);
}

void parseTime(uint8_t time_1, uint8_t time_2, uint8_t *t){
  uint16_t timebin = (time_1 << 8) | time_2;

  t[0] = timebin >> 11;
  t[1] = ((timebin >> 5) << 10) >> 10 & 0xff;
  t[2] = (timebin << 11) >> 10 & 0xff;
}

float parseTemp(uint8_t tempb_1, uint8_t tempb_2){
  uint16_t tmp = (tempb_1 << 8) | tempb_2;
  //Serial.println(tmp);
  float temp = tmp/100.;

  return temp;
}

void LoRaGet(){
  i = 0;
  Serial.print("Received pkt nbr ");
  Serial.print(cnt);
  Serial.print(": ");

  while(LoRa.available()){
    pkt[i] = LoRa.read();
    Serial.print(pkt[i], HEX);  
    Serial.print(" ");
    i++;
    if (i == 10) rx_valid = 1;
  }
  Serial.println(" ");
  
  if(rx_valid){
    pkt_len = pkt[0];
    station = pkt[1];
    sender = pkt[2];
    data_type = pkt[3];
    parseTime(pkt[4], pkt[5], time);
    tx_temp = parseTemp(pkt[6], pkt[7]);
  }
}

void printData(){
  Serial.print(pkt_len, HEX);
  Serial.print(',');
  Serial.print(station, HEX);
  Serial.print(',');
  Serial.print(sender, HEX);
  Serial.print(',');
  Serial.println(data_type, HEX);

  Serial.print("Time: ");
  Serial.print(time[0]);
  Serial.print(':');
  Serial.print(time[1]);
  Serial.print(':');
  Serial.println(time[2]);
  
  Serial.print("Temperature: ");
  Serial.println(tx_temp);
  Serial.println(" ");

  rx_valid = 0;
}


void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Starting LoRa rx");

  LoRaInit();
}

void loop() {
  // try to parse packet
  packetSize = LoRa.parsePacket();
  if (packetSize){
    LoRaGet();
    if (rx_valid){
      cnt++;
      //Serial.print("Received pkt nbr ");
      //Serial.print(cnt);

      Serial.print("RSSI: ");
      Serial.println(LoRa.packetRssi());
      Serial.print("Data: ");
      printData();

      
      

    // received a packet
    //Serial.print("Received bytes: ");
    // read packet
    /*while (LoRa.available()) {
      //Serial.print(LoRa.available());
      Serial.print("pkt: ");
      Serial.print(LoRa.read(), HEX);
    }*/

    // print RSSI of packet
      /*Serial.print(" with RSSI ");
      Serial.println(LoRa.packetRssi());*/
    }
  }
}
