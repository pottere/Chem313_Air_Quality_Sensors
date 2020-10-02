/*
PM 2.5 Sensor + Arduino Uno + MicroSD
Reed College Department of Chemistry
Last modified 31 AUG 2020
by Teddie STewart
*/
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
struct pms5003data {
uint16_t framelen;
uint16_t pm10_standard, pm25_standard, pm100_standard;
uint16_t pm10_env, pm25_env, pm100_env;
uint16_t particles_03um, particles_05um, particles_10um, particles_25um,
particles_50um, particles_100um;
uint16_t unused;
uint16_t checksum;
};
struct pms5003data data;
SoftwareSerial pmsSerial(2, 3);
const int chipSelect = 10;
unsigned long time;
const int buttonPin = 4;
int state = 0;
int lastMillis = 0;
const int timeoutMils = 25;
void setup() {
pinMode(buttonPin, INPUT);
// Open serial communications and wait for port to open:
Serial.begin(115200);
pmsSerial.begin(9600);
while (!Serial) {
; // wait for serial port to connect. Needed for native USB port only
}
Serial.print("Initializing SD card...");
// see if the card is present and can be initialized:
if (!SD.begin(chipSelect)) {
Serial.println("Card failed, or not present");
// don't do anything more:
while (1);
}
Serial.println("card initialized.");
}
boolean readPMSdata(Stream *s){
if (! s->available()) {
return false;
}
// Read a byte at a time until we get to the special '0x42' start-byte
if (s->peek() != 0x42) {
s->read();
return false;
}
// Now read all 32 bytes
if (s->available() < 32) {
return false;
}
uint8_t buffer[32];
uint16_t sum = 0;
s->readBytes(buffer, 32);
// get checksum ready
for (uint8_t i = 0; i < 30; i++) {
sum += buffer[i];
}
/* debugging
for (uint8_t i=2; i<32; i++) {
Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
}
Serial.println();
*/
// The data comes in endian'd, this solves it so it works on all platforms
uint16_t buffer_u16[15];
for (uint8_t i = 0; i < 15; i++) {
buffer_u16[i] = buffer[2 + i * 2 + 1];
buffer_u16[i] += (buffer[2 + i * 2] << 8);
}
// put it into a nice struct :)
memcpy((void *)&data, (void *)buffer_u16, 30);
if (sum != data.checksum) {
Serial.println("Checksum failure");
return false;
}
return true;
}
bool isButtonPressed(){
if(!digitalRead(buttonPin)){
if(millis() - lastMillis > timeoutMils){
lastMillis = millis();
return true;
}
else{
return false;
}
}
return false;
}
void loop() {
if(state == 0){
if(isButtonPressed()){
state = 1;
return;
}
return;
}
while(!readPMSdata(&pmsSerial)){
if(isButtonPressed()){
state = 0;
return;
}
}
Serial.println("Sensor read successful");
File myFile = SD.open("DATA_MINIMALIST.TXT", FILE_WRITE); //Text file name cannot be
if(myFile){ {
time= millis(); }
myFile.println();
myFile.print(time); myFile.print (",PM1:");
myFile.print(data.pm10_env); myFile.print (", PM2.5:");
myFile.print(data.pm25_env); myFile.print (", PM10:");
myFile.print(data.pm100_env);
myFile.close();
} else{
Serial.println("Data read, file not created :(");
}
}
