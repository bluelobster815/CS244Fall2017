/**
 * CS244 Assignment 2 - Work with PPG Sensor
 * 
 * Team #7
 *
 */
#include <Wire.h>
#include "MAX30105.h"
#include "Arduino.h"
#include "Esp.h"

#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "Config.h"

#define SAMPLE_BUF_SIZE 200

MAX30105 particleSensor;

long startTime;
long totalSamplesTaken = 0; //Counter for calculating the Hz or read rate
volatile int sampleCounter = 0;

volatile int sampleBufferRed[SAMPLE_BUF_SIZE];
volatile int sampleBufferIR[SAMPLE_BUF_SIZE];

volatile int headIndex = 0;
volatile int tailIndex = 0;

volatile int overflowCounter = 0;

void sensorISR() {
  // Serial.println("interrupt fired");
  particleSensor.check();
  while (particleSensor.available()) {
      totalSamplesTaken++;
      if(sampleCounter < SAMPLE_BUF_SIZE) {
        sampleCounter++;
      } else{
        overflowCounter++;
      }

      sampleBufferRed[tailIndex] = particleSensor.getFIFORed();
      sampleBufferIR[tailIndex] = particleSensor.getFIFOIR();

      tailIndex = ( tailIndex + 1 ) % SAMPLE_BUF_SIZE;
      if( tailIndex == headIndex ) {
        headIndex = ( headIndex + 1 ) % SAMPLE_BUF_SIZE;
      }
  
      // if ( totalSamplesTaken % 50 == 0 ) {
      //   Serial.print("] Hz[");
      //   Serial.print((float)totalSamplesTaken / ((millis() - startTime) / 1000.0), 2);
      //   Serial.println("]");
      // }
  
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
  
  char hostip[] = "169.234.43.64";
  // Attempt to establish a connection to the server.
  // Serial.print("Attempting to connect to ");
  Serial.println(hostip);

  // Arduino library class that provides funcitonality for sending HTTP requests
  HTTPClient http;
  if (http.begin(hostip, port, path)) {
    
    // Serial.println("Successfully connected.");
    // Serial.println("Sending HTTP POST request...");
    
    // get and print HTTP status code
    int httpStatusCode = http.POST(messageBody);
    Serial.print("HTTP status code: ");
    Serial.println(httpStatusCode);

    // if HTTP status code is not negative (meaning no errors):
    // if ( httpStatusCode > 0 ) {
    //   Serial.println("=== Begin Response Payload ===");
    //   String payload = http.getString();
    //   Serial.println(payload);
    //   Serial.println("=== End Response Payload ===");
    // }

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
  // attachInterrupt(15, sensorISR, FALLING);


  // startTime = millis();
  Serial.println("end initialization");
}

void gatherData(int powerLevel){
  unsigned long netTimerStart = 0;
  char entry[15] = "";
  char message[1500] = "";

  byte currentLevel = 0;
  char columnHeader[10] = "";
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

  sendPostRequest("169.234.43.64", 8080, serverPath, columnHeader);

  
  
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

  attachInterrupt(15, sensorISR, FALLING);

  //Configure sensor. Default 6.4mA for LED drive
  particleSensor.setup(currentLevel, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.enableDATARDY();

  delay(100);
  particleSensor.getINT1();

  unsigned long loopStartTime = millis();
  
  while ((millis() - loopStartTime) < 120000) {

    wdt_reset();

    if( sampleCounter >= 100 ) {

      detachInterrupt(15);
      particleSensor.disableDATARDY();
      // sampleCounter = sampleCounter - 50;
      for(int i = 0; i < 100; i++){
        sprintf(entry, "%lu,%lu\n", sampleBufferIR[headIndex], sampleBufferRed[headIndex]);
        strcat(message,entry);
        sampleCounter--;
        headIndex = ( headIndex + 1 ) % SAMPLE_BUF_SIZE;
        if ( headIndex == tailIndex ){
          break;
        }
      }
      attachInterrupt(15, sensorISR, FALLING);
      // particleSensor.getINT1();
      particleSensor.enableDATARDY();

      netTimerStart = millis();
      sendPostRequest("169.234.43.64", 8080, serverPath, message);
      Serial.print("POST took ");
      Serial.print(millis() - netTimerStart);
      Serial.println(" ms");

      memset(message, 0, sizeof(message));
      message[0] = '\0';
    }
  }

  // clear the sensor library buffer
  while (particleSensor.available()) {
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }

  Serial.print("timer has expired.  there are ");
  Serial.print(sampleCounter);
  Serial.println(" samples left in the buffer.");
  Serial.println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
  Serial.print("overflow counter: ");
  Serial.println(overflowCounter);
  
  detachInterrupt(15);
  particleSensor.disableDATARDY();

  while ( sampleCounter > 0 ) {

    for(int i = 0; i < 100; i++){
      sprintf(entry, "%lu,%lu\n", sampleBufferIR[headIndex], sampleBufferRed[headIndex]);
      strcat(message,entry);
      sampleCounter--;
      headIndex = ( headIndex + 1 ) % SAMPLE_BUF_SIZE;
      if ( headIndex == tailIndex ){
        break;
      }
    }

    netTimerStart = millis();
    sendPostRequest("169.234.43.64", 8080, serverPath, message);
    Serial.print("POST took ");
    Serial.print(millis() - netTimerStart);
    Serial.println(" ms");

    memset(message, 0, sizeof(message));
    message[0] = '\0';

  }

}

void loop()
{

  gatherData(1);
  gatherData(2);
  gatherData(3);
  gatherData(4);

  Serial.println("Done.  Wait forever...");
  // Serial.print("average sample retrieval rate: ");
  // Serial.println((float)totalSamplesTaken / ((millis() - startTime) / 1000.0), 2);
  // Serial.print("total samples taken = ");
  // Serial.println(totalSamplesTaken);

  // wait forever
  while(1){
    delay(500);
    wdt_reset();
  }

}