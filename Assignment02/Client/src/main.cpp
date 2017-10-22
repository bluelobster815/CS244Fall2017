/*
  MAX30105 Breakout: Take readings from the FIFO
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout
  Outputs all Red/IR/Green values at 25Hz by polling the FIFO
  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected
 
  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
  This code is released under the [MIT License](http://opensource.org/licenses/MIT).
*/

#include <Wire.h>
#include "MAX30105.h"
#include "Arduino.h"
#include "Esp.h"

#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "Config.h"

MAX30105 particleSensor;

long startTime;
long totalSamplesTaken = 0; //Counter for calculating the Hz or read rate
int sampleCounter = 0;

volatile int sampleBufferRed[150];
volatile int sampleBufferIR[150];
volatile int headPtr = 0;
volatile int tailPtr = 0;
// volatile int bufferCounter = 0;

void sensorISR() {
    // Serial.println("interrupt fired");
    particleSensor.check();
    while (particleSensor.available()) {
        totalSamplesTaken++;
        sampleCounter++;

        sampleBufferRed[tailPtr] = particleSensor.getFIFORed();
        sampleBufferIR[tailPtr] = particleSensor.getFIFOIR();

        tailPtr = ( tailPtr + 1 ) % 150;
        if( tailPtr == headPtr ) {
          headPtr = ( headPtr + 1 ) % 150;
        }
    
        if ( totalSamplesTaken % 50 == 0 ) {
          Serial.print("] Hz[");
          Serial.print((float)totalSamplesTaken / ((millis() - startTime) / 1000.0), 2);
          Serial.println("]");
        }
    
        particleSensor.nextSample(); //We're finished with this sample so move to next sample
    }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi network");

  // Sometimes a previous connection (that was not properly disconnected) can prevent the device from connecting to WiFi.
  // Performing a disconnect here seems to fix the issue.
  WiFi.disconnect();

  // Initialize WiFi radio and attempt to connect to specified SSID.
  WiFi.begin(WiFiSSID, WiFiPassword);

  // Keep waiting/retrying till the WiFi status changes to connected.
  // Note that we will wait forever if we reach the maximum number of failed connection attemps.
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Successfully connected to WiFi network.");
  Serial.print("Assigned IP address: ");
  Serial.println(WiFi.localIP());
}

// Send an HTTP POST request
void sendPostRequest(char *host, uint16_t port, char *path, char *messageBody) {
  
  // Attempt to establish a connection to the server.
  Serial.print("Attempting to connect to ");
  Serial.println(host);

  // Arduino library class that provides funcitonality for sending HTTP requests
  HTTPClient http;
  if (http.begin(host, port, path)) {
    
    Serial.println("Successfully connected.");
    Serial.println("Sending HTTP POST request...");
    
    // get and print HTTP status code
    int httpStatusCode = http.POST(messageBody);
    Serial.print("HTTP status code: ");
    Serial.println(httpStatusCode);

    // if HTTP status code is not negative (meaning no errors):
    if ( httpStatusCode > 0 ) {
      Serial.println("=== Begin Response Payload ===");
      String payload = http.getString();
      Serial.println(payload);
      Serial.println("=== End Response Payload ===");
    }

    // close HTTP connection
    http.end();

  } else {
    Serial.println("Connection attempt failed.");
  }
  
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing...");

  connectWiFi();

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  Serial.println("configure Arduino interrupts");
  pinMode(15, INPUT_PULLUP);
  attachInterrupt(15, sensorISR, FALLING);


  startTime = millis();
  Serial.println("end initialization");
}

void gatherData(int powerLevel){
  unsigned long netTimerStart = 0;
  char entry[15] = "";
  char message[750] = "";

  byte currentLevel = 0;
  char columnHeader[9] = "";
  char serverPath[20] = "";
  
  switch ( powerLevel ) {
    case 1:
      currentLevel = 0x02;
      sprintf(columnHeader, "IR1,RED1\n");
      sprintf(serverPath, "/cs244/data/400uA");
      break;
    case 2:
      currentLevel = 0x1F;
      sprintf(columnHeader, "IR2,RED2\n");
      sprintf(serverPath, "/cs244/data/6400uA");
      break;
    case 3:
      currentLevel = 0x7F;
      sprintf(columnHeader, "IR3,RED3\n");
      sprintf(serverPath, "/cs244/data/25400uA");
      break;
    case 4:
      currentLevel = 0xFF;
      sprintf(columnHeader, "IR4,RED4\n");
      sprintf(serverPath, "/cs244/data/50000uA");
      break;
  }

  sendPostRequest("192.168.0.102", 8080, serverPath, columnHeader);

  
  
  // Serial.println("configure MAX30105 interrupts");
  //LED Pulse Amplitude Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //Default is 0x1F which gets us 6.4mA
  //powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
  //powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
  //powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
  //powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

  // byte powerLevel = 0x7F; // default = 0x1F
  byte sampleAverage = 1; // default
  byte ledMode = 2;       // default
  int sampleRate = 50;   // default = 400
  int pulseWidth = 411;   // default
  int adcRange = 4096;    // default

  //Configure sensor. Default 6.4mA for LED drive
  particleSensor.setup(currentLevel, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.enableDATARDY();

  delay(100);
  particleSensor.getINT1();

  // char columnHeader[]="RED1,IR1\n";
  unsigned long loopStartTime = millis();
  
  while ((millis() - loopStartTime) < 30000) {

    wdt_reset();

    if( sampleCounter >= 50 ) {

      // detachInterrupt(15);
      particleSensor.disableDATARDY();
      sampleCounter = 0;
      for(int i = 0; i < 50; i++){
        sprintf(entry, "%lu,%lu\n", sampleBufferIR[headPtr], sampleBufferRed[headPtr]);
        strcat(message,entry);
        headPtr = ( headPtr + 1 ) % 150;
        if ( headPtr == tailPtr ){
          break;
        }
      }
      // attachInterrupt(15, sensorISR, FALLING);
      // particleSensor.getINT1();
      particleSensor.enableDATARDY();

      netTimerStart = millis();
      sendPostRequest("192.168.0.102", 8080, serverPath, message);
      Serial.print("POST took ");
      Serial.print(millis() - netTimerStart);
      Serial.println(" ms");

      memset(message, 0, sizeof(message));
      message[0] = '\0';
    }
  }

  Serial.print("timer has expired.  there are ");
  Serial.print(sampleCounter);
  Serial.println(" samples left in the buffer.");
  
  particleSensor.disableDATARDY();

  for(int i = 0; i < 50; i++){
    // sprintf(entry, "%lu,%lu\n", sampleBufferRed[headPtr], sampleBufferIR[headPtr]);
    sprintf(entry, "%lu,%lu\n", sampleBufferIR[headPtr], sampleBufferRed[headPtr]);
    strcat(message,entry);
    headPtr = ( headPtr + 1 ) % 150;
    if ( headPtr == tailPtr ){
      break;
    }
  }

  netTimerStart = millis();
  sendPostRequest("192.168.0.102", 8080, serverPath, message);
  Serial.print("final POST took ");
  Serial.print(millis() - netTimerStart);
  Serial.println(" ms");




}

void loop()
{

  // gatherData(1);
  // gatherData(2);
  gatherData(3);
  gatherData(4);

  Serial.println("Done.  Wait forever...");
  Serial.print("average sample retrieval rate: ");
  Serial.println((float)totalSamplesTaken / ((millis() - startTime) / 1000.0), 2);
  Serial.print("total samples taken = ");
  Serial.println(totalSamplesTaken);

  // blink LED while waiting forever
  while(1){
  //   digitalWrite(LED_BUILTIN, HIGH); // LED off
  //   delay(500);
  //   digitalWrite(LED_BUILTIN, LOW); // LED on
    delay(500);
    wdt_reset();
  }

}