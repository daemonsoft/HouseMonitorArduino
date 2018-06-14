#include "EmonLib.h"  //Usada para el sensor de energia

// Crear una instancia EnergyMonitor
EnergyMonitor energyMonitor;

// Voltaje de nuestra red eléctrica
float voltajeRed = 110.0;
String r = " ";
char character;
const int waterFlowSensorPin = 2;
const int measureInterval = 2500;
volatile int pulseConter;
const int lightPin = 4;
const int doorPin = 3;
int doorState = LOW;
int lightState = LOW;
unsigned long previousMillis = 0;
const long interval = 3000;
// YF-S201
const float factorK = 7.5;


void ISRCountPulse(){
   pulseConter++;
}


float GetFrequency(){
   pulseConter = 0;
   interrupts();
   delay(measureInterval);
   noInterrupts();
   return (float)pulseConter * 1000 / measureInterval;
}


void openDoor(unsigned long currentMillis,unsigned long previousMillis,String r){
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    if (doorState == LOW) {
      if(r == "D1"){
        doorState = HIGH;
      }
    } else {
      doorState = LOW;
    }
    digitalWrite(doorPin, doorState);
  }
}

void switchLight(String r){
  if(r == "L11"){
    if(lightState == LOW){
      digitalWrite(lightPin, HIGH);
      lightState = HIGH;
    }else{
      digitalWrite(lightPin, LOW);
      lightState = LOW;
    }
  }
}

void getString(){
  while(Serial.available()){
    character = Serial.read();
    r.concat(character);
    Serial.print("aasl");
  }
}


//--------------SETUP-------------------

void setup(){
   // Iniciamos la clase indicando
   // Número de pin: donde tenemos conectado el SCT-013
   // Valor de calibración: valor obtenido de la calibración teórica
   energyMonitor.current(0, 1.6);
   pinMode(lightPin, OUTPUT);
   pinMode(doorPin, OUTPUT);
   Serial.begin(9600);
   attachInterrupt(digitalPinToInterrupt(waterFlowSensorPin), ISRCountPulse, RISING);
}


//--------------LOOP-------------------

void loop(){
  unsigned long currentMillis = millis();
  //-- CALCULO DEL CAUDALIMETRO --
  // obtener frecuencia en Hz
  float frequency = GetFrequency();
  // calcular caudal L/min
  float flow_Lmin = frequency * factorK;
  //--

  //-- CALCULO DEL SENSOR DE ENERGIA --
  double Irms = energyMonitor.calcIrms(1484);
  // Calculamos la potencia aparente
  double potencia =  Irms * voltajeRed;
  // Mostramos la información por el monitor serie
  //--

  //-- PUERTA Y BOMBILLOS --
  getString();
  openDoor(currentMillis, previousMillis, r);
  switchLight(r);
  r = "";
  //--
}
----------------

/***************************************************

 ****************************************************/

#include <SPI.h>
#include <Keypad.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.

#define PN532_IRQ   (6)
#define PN532_RESET (7)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
uint8_t success;
uint8_t uid[] = {0,0,0,0,0,0,0};
uint8_t uidLength;
int test = 0;
void setup(void) {
	Serial.begin(115200);

	nfc.begin();

	uint32_t versiondata = nfc.getFirmwareVersion();
	if (!versiondata) {
		Serial.println("Didn't find PN53x board");
		//while (1); // halt
	}

	// Got ok data, print it out!
	Serial.print("Found chip PN5");
	Serial.println((versiondata >> 24) & 0xFF, HEX);
	Serial.print("Firmware ver. ");
	Serial.print((versiondata >> 16) & 0xFF, DEC);
	Serial.print('.');
	Serial.println((versiondata >> 8) & 0xFF, DEC);

	// Set the max number of retry attempts to read from a card
	// This prevents us from waiting forever for a card, which is
	// the default behaviour of the PN532.
	nfc.setPassiveActivationRetries(0x01);

	// configure board to read RFID tags
	nfc.SAMConfig();

	Serial.print("Initializing...");


	Serial.print(" done.\n");
	//delay(1000);
}

void loop() {

	// Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
	// 'uid' will be populated with the UID, and uidLength will indicate
	// if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
	success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

	if (success) {
		// Display some basic information about the card
		Serial.print("Found an ISO14443A card ");

	}

	delay(2000);
	Serial.print(";bitch;");
	Serial.print(test);
	Serial.print(";");
	test = test + 1;
}
