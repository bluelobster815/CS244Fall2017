#include "Wire.h"
#include "MAX30105.h"
#include "Arduino.h"
#include "SPI.h"
#include "SparkFunLIS3DH.h"

#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"

#include "Config.h"

#define BUF_SIZE 100
#define MESSAGE_SIZE 50
#define SAMPLE_WINDOW_MS 1000 * 600

// IR reading is 18 bits unsigned = 6 chars
// Red reading is also 18 bits unsigned
// accelerometer reading is 5 chars + sign char = 6 chars
// 6*2 + 6*3 + 4 commas + newline + \0 = 
#define CHARS_PER_ENTRY 36

MAX30105 particleSensor;
LIS3DH accelerometer(SPI_MODE, 16);

volatile uint16_t samples = 0;
volatile int interrupts = 0;

byte interruptPin = 15;

volatile int16_t sampleBufferX[BUF_SIZE];
volatile int16_t sampleBufferY[BUF_SIZE];
volatile int16_t sampleBufferZ[BUF_SIZE];

volatile int sampleBufferIR[BUF_SIZE];
volatile int sampleBufferRed[BUF_SIZE];

volatile int8_t headIndex = 0;
volatile int8_t tailIndex = 0;
volatile int8_t bufferCount = 0;

volatile int overflowCount = 0;

// panic message for debugging
void uhoh(char* message) {
   Serial.println("UH OH SOMETHING REALLY BAD HAPPENED");
   Serial.println(message);
   Serial.print("sample number = ");
   // Serial.println(sampleNumber);
   while(1){      
      wdt_reset();
   }
}

void enq(int valueIR, int valueRed, int16_t valueX, int16_t valueY, int16_t valueZ) {

   if ( bufferCount > BUF_SIZE )
      uhoh("bufferCount > BUF_SIZE");

   sampleBufferIR[tailIndex] = valueIR;
   sampleBufferRed[tailIndex] = valueRed;
   sampleBufferX[tailIndex] = valueX;
   sampleBufferY[tailIndex] = valueY;
   sampleBufferZ[tailIndex] = valueZ;

   tailIndex = ( tailIndex + 1 ) % BUF_SIZE;

   if( bufferCount == BUF_SIZE ) {
      headIndex = ( headIndex + 1 ) % BUF_SIZE;
      overflowCount++;
   }
   else if ( bufferCount < BUF_SIZE ) {
      bufferCount++;
   }

}

void deq(int *valueIR, int *valueRed, int16_t *valueX, int16_t *valueY, int16_t *valueZ) {
   
   if ( bufferCount <= 0 ) {
      uhoh("bufferCount <= 0");
   }
   else if ( bufferCount > 0 ) {
      *valueIR = sampleBufferIR[headIndex];
      *valueRed = sampleBufferRed[headIndex];
      *valueX = sampleBufferX[headIndex];
      *valueY = sampleBufferY[headIndex];
      *valueZ = sampleBufferZ[headIndex];

      headIndex = ( headIndex + 1 ) % BUF_SIZE;
      bufferCount--;
   }

}

void sensorISR()
{
   // Serial.print("[R=");
   // Serial.print(particleSensor.getFIFORed());
   // Serial.print("][IR=");
   // Serial.print(particleSensor.getFIFOIR());
   // Serial.print("][X=");
   // Serial.print(accelerometer.readFloatAccelX(), 4);
   // Serial.print("][Y=");
   // Serial.print(accelerometer.readFloatAccelY(), 4);
   // Serial.print("][Z=");
   // Serial.print(accelerometer.readFloatAccelZ(), 4);
   // Serial.println("]");
   // samples = samples + particleSensor.check();
   // interrupts++;

   int irSample = 0;
   int redSample = 0;

   int16_t xsample = 0;
   int16_t ysample = 0;
   int16_t zsample = 0;
   
   uint8_t numSamples = particleSensor.check();

   while (particleSensor.available()) {
   // if ( numSamples > 0 ) {

      irSample = particleSensor.getFIFOIR();
      redSample = particleSensor.getFIFORed();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      xsample = accelerometer.readRawAccelX();
      ysample = accelerometer.readRawAccelY();
      zsample = accelerometer.readRawAccelZ();

      enq(irSample, redSample, xsample, ysample, zsample);
   }

}

void configureParticleSensor()
{

   // Initialize sensor
   if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
   {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
      while (1);
   }

   pinMode(interruptPin, INPUT_PULLUP);
   attachInterrupt(interruptPin, sensorISR, FALLING);

   byte powerLevel = 60; // from spo2 example
   byte sampleAverage = 1; // default
   byte ledMode = 2;       // default
   int sampleRate = 50;   // default = 400
   int pulseWidth = 411;   // default
   int adcRange = 4096;    // default  

   particleSensor.setup(powerLevel, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
   
   particleSensor.enableDATARDY();

   // needed for interrupt to start triggering
   delay(100);
   particleSensor.getINT1();

}

void configureAccelerometer(){

   accelerometer.settings.adcEnabled = 0;
   accelerometer.settings.tempEnabled = 0;
   accelerometer.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
   accelerometer.settings.accelRange = 2;      //Max G force readable.  Can be: 2, 4, 8, 16
   accelerometer.settings.xAccelEnabled = 1;
   accelerometer.settings.yAccelEnabled = 1;
   accelerometer.settings.zAccelEnabled = 1;

   accelerometer.begin();

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

   }
   else {
      Serial.println("Connection attempt failed.");
   }

}

void setup()
{
   Serial.begin(9600);
   Serial.println("Initializing...");

   connectWiFi();

   Serial.println("Configure accelerometer...");
   configureAccelerometer();  

   Serial.println("Configure particle sensor...");
   configureParticleSensor();

   // startTime = millis();
   // Serial.println("end initialization");
}

// void loop()
// {
//    while(interrupts<50){
//       wdt_reset();
//       // Serial.print("interrupts = ");
//       // Serial.println(interrupts);
//    }
//    float samplesPerInterrupt = float(samples)/float(interrupts);
//    Serial.print("samples per interrupt = ");
//    Serial.println(samplesPerInterrupt, 8);
//    samples = 0;
//    interrupts = 0;
// }

void loop()
{

   char entry[CHARS_PER_ENTRY] = "";
   char message[CHARS_PER_ENTRY*MESSAGE_SIZE] = "";
   char serverPath[] = "/cs244/webapi/data/acceleration";

   unsigned long loopStartTime = millis();
   unsigned long loopWatchdog = millis();

   int IR_sample = 0;
   int Red_sample = 0;
   int16_t x_sample = 0;
   int16_t y_sample = 0;
   int16_t z_sample = 0;
   
   while ((millis() - loopStartTime) < SAMPLE_WINDOW_MS) {

      wdt_reset();

      // for debugging
      // if((millis() - loopWatchdog) > 10000)
      //    debugMode = true;

      if( bufferCount >= MESSAGE_SIZE ) {

         Serial.println("Converting samples to CSV data....");
         // Serial.print("bufferCount = ");
         // Serial.println(bufferCount);

         // disable external interrupt for critical section in loop so that
         // it does not try to read from buffers while ISR is writing to them
         detachInterrupt(interruptPin);
         // particleSensor.disableDATARDY();
         for( int i = 0; i < MESSAGE_SIZE; i++ ) {
            // retrieve samples from buffer and convert to CSV formatted string
            deq(&IR_sample, &Red_sample, &x_sample, &y_sample, &z_sample);
            sprintf(entry, "%lu,%lu,%d,%d,%d\n", IR_sample, Red_sample, x_sample, y_sample, z_sample);
            strcat(message,entry);
         }
         attachInterrupt(interruptPin, sensorISR, FALLING);
         // particleSensor.enableDATARDY();

         // call to read samples that may have been gathered during string conversion
         // and clear the interrupt
         sensorISR();

         // send CSV formatted string to server in HTTP POST request
         sendPostRequest(hostip, 8080, serverPath, message);

         // re-initialize message buffer
         memset(message, 0, sizeof(message));
         message[0] = '\0';

         // for debugging
         // sampleNumber = 0;
         loopWatchdog = millis();         

      }

   }

   Serial.println("loop timer expired.");
   Serial.print("There are ");
   Serial.print(bufferCount);
   Serial.println(" samples left in the buffer.");

   // detachInterrupt(4);
   detachInterrupt(interruptPin);

   // convert any remaining samples in buffer to CSV formatted string and send to server
   while ( bufferCount > 0 ) {

      for( int i = 0; i < MESSAGE_SIZE; i++ ) {
         deq(&IR_sample, &Red_sample, &x_sample, &y_sample, &z_sample);
         sprintf(entry, "%lu,%lu,%d,%d,%d\n", IR_sample, Red_sample, x_sample, y_sample, z_sample);
         strcat(message,entry);
         if(bufferCount == 0)
            break;
      }
      sendPostRequest(hostip, 8080, serverPath, message);

      memset(message, 0, sizeof(message));
      message[0] = '\0';

   }

   Serial.println("Done!! Wait forever...............................");

   while(1){
      wdt_reset(); // needed to prevent watchdog timers from resetting the board
   }

}