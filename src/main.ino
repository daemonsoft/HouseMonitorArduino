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
const int light1Pin = 9;
const int doorPin = 10;
const int light2Pin = 11;
int doorState = LOW;
int light1State = LOW;
int light2State = LOW;
unsigned long previousMillis = 0;
const long interval = 5000;
float lastFlow = 0;
double lastPotency = 0;
unsigned long currentMillisPotency = 0;
unsigned long previousMillisPotency = 0;
unsigned long currentMillisFlow = 0;
unsigned long previousMillisFlow = 0;
float flowConsumption = 0;
float potencyConsumption = 0;
String stringPotencyConsumptionCommand = "";
String stringFlowConsumptionCommand = "";

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


void commandsReceive(String r){
  String command = "";
  for(int i = 0; i < r.length(); i++){
    if(r.charAt(i) == '{'){
      int j = i + 1;
      while(r.charAt(j) != '}'){
        command = command + r.charAt(j);
        j++;
        i = j;
      }
      openDoor(command);
      switchLight(command);
      command = "";
    }
  }
}

void openDoor(String command){
//  Serial.print("R3 = ");
//  Serial.println(r);
  if(command.toInt() == 11){
    doorState = HIGH;
  }else if(command.toInt() == 10){
    doorState = LOW;
  }
  digitalWrite(doorPin, doorState);
}

void switchLight(String command){
//  Serial.print("R2 = ");
//  Serial.println(r);
  if(command.toInt() == 211){
      digitalWrite(light1Pin, HIGH);
      light1State = HIGH;
  }else if(command.toInt() == 210){
      digitalWrite(light1Pin, LOW);
      light1State = LOW;
  }else if(command.toInt() == 221){
      digitalWrite(light2Pin, HIGH);
      light2State = HIGH;
  }else if(command.toInt() == 220){
      digitalWrite(light2Pin, LOW);
      light2State = LOW;
  }
}

void getString(){
  while(Serial.available()){
    character = Serial.read();
    r.concat(character);
  }
}


//--------------SETUP-------------------

void setup(){
   // Iniciamos la clase indicando
   // Número de pin: donde tenemos conectado el SCT-013
   // Valor de calibración: valor obtenido de la calibración teórica
   energyMonitor.current(0, 1.6);
   pinMode(light1Pin, OUTPUT);
   pinMode(light2Pin, OUTPUT);
   pinMode(doorPin, OUTPUT);
   Serial.begin(57600);
   Serial.setTimeout(300);
   attachInterrupt(digitalPinToInterrupt(waterFlowSensorPin), ISRCountPulse, RISING);
}


//--------------LOOP-------------------

void loop(){
  unsigned long currentMillis = millis();
  unsigned long currentMillisFlow = millis();
  unsigned long currentMillisPotency = millis();
  //-- CALCULO DEL CAUDALIMETRO --
  // obtener frecuencia en Hz
  float frequency = GetFrequency();
  // calcular caudal L/min
  float currentFlow_Lmin = frequency * factorK;
  if(lastFlow != currentFlow_Lmin){
    flowConsumption = currentMillisFlow - previousMillisFlow;
    flowConsumption = (flowConsumption/60000) * lastFlow;
    previousMillisFlow = currentMillisFlow;
    lastFlow = currentFlow_Lmin;
    stringFlowConsumptionCommand = "{F";
    stringFlowConsumptionCommand = stringFlowConsumptionCommand + flowConsumption + "}";
    Serial.println(stringFlowConsumptionCommand);
    flowConsumption = 0;
    stringFlowConsumptionCommand = "";
  }


  //--

  //-- CALCULO DEL SENSOR DE ENERGIA --
  double Irms = energyMonitor.calcIrms(1484);
  // Calculamos la potencia aparente
  double currentPotency =  Irms * voltajeRed;
  // Mostramos la información por el monitor serie
    //
  if(lastPotency != currentPotency){
    potencyConsumption = currentMillisPotency - previousMillisPotency;
    potencyConsumption = (potencyConsumption/60000) * lastPotency;
    previousMillisPotency = currentMillisPotency;
    lastPotency = currentPotency;
    stringPotencyConsumptionCommand = "{P";
    stringPotencyConsumptionCommand = stringPotencyConsumptionCommand + potencyConsumption + "}";
    Serial.println(stringPotencyConsumptionCommand);
    potencyConsumption = 0;
    stringPotencyConsumptionCommand = "";
  }


  //

  //--

  //-- PUERTA Y BOMBILLOS --
  getString();
//  Serial.print("R1 = ");
//  Serial.println(r);
  commandsReceive(r);
  r = "";
  //--
}
