/**
 * CS244 Assignment 4 - Work with 3D Accelerometer
 * 
 * Team #7 
 *
 */
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"
#include "Config.h"

#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "stdio.h"

// #include "ets_sys.h"
// #include "os_type.h"

// extern "C" {
// #include "user_interface.h"
// }
// uint32_t freeHeapSize = system_get_free_heap_size();

#define BUF_SIZE 100
#define MESSAGE_SIZE 50
#define SAMPLE_WINDOW_MS 1000 * 120

volatile int16_t sampleBufferX[BUF_SIZE];
volatile int16_t sampleBufferY[BUF_SIZE];
volatile int16_t sampleBufferZ[BUF_SIZE];

volatile int8_t headIndex = 0;
volatile int8_t tailIndex = 0;
volatile int8_t bufferCount = 0;

// used for debugging
volatile uint32_t sampleNumber = 0;
volatile int overflowCount = 0;
bool debugMode = false;

LIS3DH accelerometer(SPI_MODE, 16); //Constructing with SPI interface information

void enq(int16_t valueX, int16_t valueY, int16_t valueZ);
void deq(int16_t *valueX, int16_t *valueY, int16_t *valueZ);

void sensorISR(){

   // uint8_t fifoUnreadSamples = ( accelerometer.fifoGetStatus() & 0x1F );
   // uint8_t samplesToGet = fifoUnreadSamples < 20 ? fifoUnreadSamples : 20;
   
   // while the device FIFO empty flag is not set
   while(( accelerometer.fifoGetStatus() & 0x20 ) == 0)
   {
      int16_t xsample = accelerometer.readRawAccelX();
      int16_t ysample = accelerometer.readRawAccelY();
      int16_t zsample = accelerometer.readRawAccelZ();
      enq(xsample,ysample,zsample);
      sampleNumber++;
      
      // this reset may not be necessary
      wdt_reset();

      // for debugging
      if(debugMode) {
         Serial.print("sample count since last parse: ");
         Serial.println(sampleNumber);
      }

   }

}

// panic message for debugging
void uhoh(char* message) {
   Serial.println("UH OH SOMETHING REALLY BAD HAPPENED");
   Serial.println(message);
   Serial.print("sample number = ");
   Serial.println(sampleNumber);
   while(1){      
      wdt_reset();
   }
}

void enq(int16_t valueX, int16_t valueY, int16_t valueZ) {

   if ( bufferCount > BUF_SIZE )
      uhoh("bufferCount > BUF_SIZE");

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

void deq(int16_t *valueX, int16_t *valueY, int16_t *valueZ) {
   
   if ( bufferCount <= 0 ) {
      uhoh("bufferCount <= 0");
   }
   else if ( bufferCount > 0 ) {
      *valueX = sampleBufferX[headIndex];
      *valueY = sampleBufferY[headIndex];
      *valueZ = sampleBufferZ[headIndex];

      headIndex = ( headIndex + 1 ) % BUF_SIZE;
      bufferCount--;
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
      // if ( httpStatusCode > 0 ) {
      //    Serial.println("=== Begin Response Payload ===");
      //    String payload = http.getString();
      //    Serial.println(payload);
      //    Serial.println("=== End Response Payload ===");
      // }

      // close HTTP connection
      http.end();

   }
   else {
      Serial.println("Connection attempt failed.");
   }

}

void configureSensor(){

   accelerometer.settings.adcEnabled = 0;
   //Note:  By also setting tempEnabled = 1, temperature data is available
   //instead of ADC3 in.  Temperature *differences* can be read at a rate of
   //1 degree C per unit of ADC3 data.
   accelerometer.settings.tempEnabled = 0;
   accelerometer.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
   accelerometer.settings.accelRange = 2;      //Max G force readable.  Can be: 2, 4, 8, 16
   accelerometer.settings.xAccelEnabled = 1;
   accelerometer.settings.yAccelEnabled = 1;
   accelerometer.settings.zAccelEnabled = 1;

   //FIFO control settings
   accelerometer.settings.fifoEnabled = 1;
   accelerometer.settings.fifoThreshold = 10;  //Can be 0 to 32
   accelerometer.settings.fifoMode = 2;
   //fifoMode can be:
   //  0 (Bypass mode, FIFO off)
   //  1 (FIFO mode)
   //  2 (Stream mode)
   //  3 (Stream to FIFO mode)
  
   //Call .begin() to configure the IMU (except for the fifo)
   accelerometer.begin();

   Serial.print("Configuring FIFO with no error checking...");
   accelerometer.fifoBegin(); //Configure fifo
   Serial.print("Done!\n");

   Serial.print("Clearing out the FIFO...");
   accelerometer.fifoClear();
   Serial.print("Done!\n");
   accelerometer.fifoStartRec(); //cause fifo to start taking data (re-applies mode bits)

   // configure LS3IDH to generate watermark interrupt
   uint8_t registerData = 0;
   accelerometer.readRegister(&registerData, LIS3DH_CTRL_REG3);
   // Serial.println(registerData, BIN);
   registerData |= 0x04;
   // Serial.println(registerData, BIN);
   accelerometer.writeRegister(LIS3DH_CTRL_REG3, registerData);

}

void setup() {

   delay(3000);

   Serial.begin(9600);   
   Serial.println("===   Booting...  ===");

   // Serial.print("free heap size = ");
   // Serial.println(freeHeapSize);

   connectWiFi();
   configureSensor();   

   // configure ESP8266 external interrupt
   pinMode(4, INPUT);
   attachInterrupt(digitalPinToInterrupt(4), sensorISR, RISING);

   Serial.println("=== Boot completed ===");
  
}

void loop()
{

   char entry[18] = "";
   char message[18*MESSAGE_SIZE] = "";
   char serverPath[] = "/cs244/webapi/data/acceleration";

   unsigned long loopStartTime = millis();
   unsigned long loopWatchdog = millis();

   int16_t x_sample = 0;
   int16_t y_sample = 0;
   int16_t z_sample = 0;
   
   while ((millis() - loopStartTime) < SAMPLE_WINDOW_MS) {

      wdt_reset();

      // for debugging
      if((millis() - loopWatchdog) > 10000)
         debugMode = true;

      // while(( accelerometer.fifoGetStatus() & 0x80 ) == 0) {wdt_reset();};  //Wait for watermark
      // Serial.print("bufferCount before = ");
      // Serial.println(bufferCount);
      // Serial.println("getting samples");
      // sensorISR();
      // Serial.print("bufferCount after = ");
      // Serial.println(bufferCount);

      if( bufferCount >= MESSAGE_SIZE ) {

         Serial.println("Converting samples to CSV data....");
         Serial.print("bufferCount = ");
         Serial.println(bufferCount);

         // disable external interrupt for critical section in loop so that
         // it does not try to read from buffers while ISR is writing to them
         detachInterrupt(4);
         for( int i = 0; i < MESSAGE_SIZE; i++ ) {
            // retrieve samples from buffer and convert to CSV formatted string
            deq(&x_sample, &y_sample, &z_sample);
            sprintf(entry, "%d,%d,%d\n", x_sample, y_sample, z_sample);
            strcat(message,entry);
         }
         attachInterrupt(digitalPinToInterrupt(4), sensorISR, RISING);

         // send CSV formatted string to server in HTTP POST request
         sendPostRequest(hostip, 8080, serverPath, message);

         // re-initialize message buffer
         memset(message, 0, sizeof(message));
         message[0] = '\0';

         // not sure if this is necessary
         sensorISR();

         // for debugging
         sampleNumber = 0;
         loopWatchdog = millis();         

      }

   }

   Serial.println("loop timer expired.");
   Serial.print("There are ");
   Serial.print(bufferCount);
   Serial.println(" samples left in the buffer.");

   detachInterrupt(4);

   // convert any remaining samples in buffer to CSV formatted string and send to server
   while ( bufferCount > 0 ) {

      for( int i = 0; i < MESSAGE_SIZE; i++ ) {
         deq(&x_sample, &y_sample, &z_sample);
         sprintf(entry, "%d,%d,%d\n", x_sample, y_sample, z_sample);
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