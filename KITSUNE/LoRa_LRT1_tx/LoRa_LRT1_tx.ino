#include <SPI.h>
#include <LoRa.h>

//Vin - 5V
//GND
//SCK - 13
//MISO - 12
//MOSI - 11
//CS - 10


uint8_t pkt[10] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");
  
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setTxPower(17,PA_OUTPUT_PA_BOOST_PIN); // transmitting with 17dBm(50mW) power 
  LoRa.setFrequency(433E6); //put your transmission frequency here
  LoRa.setSignalBandwidth(250E3); // put your bandwidth here, Lora has fixed bandwidths (7.8,10.4,15.6,20.8,31.25,41.7,62.5,125,250,500kHz)
  LoRa.setSpreadingFactor(7); //put your spreading factor here, choices are 6,7,8,9,10,11,12
  LoRa.setCodingRate4(5); //put your coding rate here, choices are 5,6,7,8
  
  Serial.println("LoRa setup succeeded.");
  
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  Serial.print(":    ");

    for(int i = 0; i<10;i++)
    {
        Serial.print(pkt[i],HEX); 
    }
    Serial.println(" ");
    
    //send packet 
    LoRa.beginPacket();
        LoRa.print(counter);
        LoRa.print(": ");
        for(int i = 0; i<10;i++)
        {
            LoRa.print(pkt[i],HEX); 
        }
    LoRa.endPacket();

    counter++;

  delay(2000);
}
