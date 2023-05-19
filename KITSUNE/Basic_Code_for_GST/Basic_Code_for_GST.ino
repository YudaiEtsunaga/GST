#include <SPI.h>
#include <LoRa.h>
uint8_t pckt[10] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55}; // put your message or data here or you can create it in the loop also
int counter = 0;

//in case you dont use the default SPI pins of arduino, you can create your SPI port like this but use this library #include <SoftSPI.h>
//Creating a new SPI Port with
//Pin 25 = D4 = MOSI; match with arduino micro pin out
//Pin 27 = D6 = MISO;
//Pin 26 = D12 =SCK;
//SoftSPI spi(4, 6, 12);
 

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) { //this one sets the frequency module - like 433,868,915MHz
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setFrequency(433.067E6); //put your transmission frequency here
  LoRa.setTxPower(17,PA_OUTPUT_PA_BOOST_PIN); // transmitting with 17dB power 
  LoRa.setSignalBandwidth(31.25E3); // put your bandwidth here, Lora has fixed bandwidths (7.8,10.4,15.6,20.8,31.25,41.7,62.5,125,250,500kHz)
  LoRa.setSpreadingFactor(8); //put your spreading factor here, choices are 6,7,8,9,10,11,12
  LoRa.setCodingRate4(8); //put your coding rate here, choices are 5,6,7,8
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  Serial.print(":    ");
    
    for(int i = 0; i<10;i++)
    {
        Serial.print(pckt[i],HEX);
        Serial.print(" ");
    }
    Serial.println("");
    LoRa.beginPacket();
    LoRa.write(pckt,10);
    LoRa.endPacket();
   
    counter++;

  delay(5000); // choose how often you want to transmit (this one is delay of 5 secs)
}
