#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"
#include "Config.h"

#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "stdio.h"

#define BUF_SIZE 200

volatile int16_t sampleBufferX[BUF_SIZE];
volatile int16_t sampleBufferY[BUF_SIZE];
volatile int16_t sampleBufferZ[BUF_SIZE];

volatile int headIndex = 0;
volatile int tailIndex = 0;
volatile int bufferCount = 0;

volatile uint32_t sampleNumber = 0; //Used to make CSV output row numbers

volatile bool samplesReady = false;

volatile int overflowCount = 0;




LIS3DH myIMU(SPI_MODE, 16); //Constructing with SPI interface information
//LIS3DH myIMU(I2C_MODE, 0x19); //Alternate constructor for I2C

void uhoh(char* message) {
   Serial.println(message);
   Serial.print("sample number = ");
   Serial.println(sampleNumber);
   while(1){      
      wdt_reset();
   }
}

// void incrementHead() {
//    if( bufferCount <= 0 ) {
//       uhoh();
//    }
//    headIndex = ( headIndex + 1 ) % BUF_SIZE;
// }
// void incrementTail() {
//    if ( ( tailIndex == headIndex ) && ( bufferCount == BUF_SIZE ) ) {
//       incrementHead();
//    }
//    tailIndex = ( tailIndex + 1 ) % BUF_SIZE;
// }

// void enq(int16_t buffer[], int16_t value) {

//    if( bufferCount == BUF_SIZE ) {
//       buffer[tailIndex] = value;
//       tailIndex = ( tailIndex + 1 ) % BUF_SIZE;
//       headIndex = ( headIndex + 1 ) % BUF_SIZE;
//       overflowCount++;
//    }
//    else if ( bufferCount < BUF_SIZE ) {
//       buffer[tailIndex] = value;
//       tailIndex = ( tailIndex + 1 ) % BUF_SIZE;
//       bufferCount++;
//    }
//    else if ( bufferCount > BUF_SIZE ) {
//       uhoh();
//    }

// }

void enq(volatile int16_t buffer[], int16_t value) {

   if ( bufferCount > BUF_SIZE )
      uhoh("bufferCount > BUF_SIZE");
   
   buffer[tailIndex] = value;
   tailIndex = ( tailIndex + 1 ) % BUF_SIZE;

   if( bufferCount == BUF_SIZE ) {
      headIndex = ( headIndex + 1 ) % BUF_SIZE;
      overflowCount++;
   }
   else if ( bufferCount < BUF_SIZE ) {
      bufferCount++;
   }

}

int16_t deq(volatile int16_t buffer[]) {

   int16_t bufferValue;
   
   if ( bufferCount <= 0 ) {
      uhoh("bufferCount <= 0");
   }
   else if ( bufferCount > 0 ) {
      bufferValue = buffer[headIndex];
      headIndex = ( headIndex + 1 ) % BUF_SIZE;
      bufferCount--;
   }

   return bufferValue;
}

void sensorISR(){
   samplesReady = true;
   // Serial.println("interrup fired!");
   while(( myIMU.fifoGetStatus() & 0x20 ) == 0) //This checks for the 'empty' flag
   {
      // sampleBufferX[tailIndex] = myIMU.readRawAccelX();
      // sampleBufferY[tailIndex] = myIMU.readRawAccelY();
      // sampleBufferZ[tailIndex] = myIMU.readRawAccelZ();
      enq(sampleBufferX, myIMU.readRawAccelX());
      enq(sampleBufferY, myIMU.readRawAccelY());
      enq(sampleBufferZ, myIMU.readRawAccelZ());

      sampleNumber++;

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
   //   if ( httpStatusCode > 0 ) {
   //     Serial.println("=== Begin Response Payload ===");
   //     String payload = http.getString();
   //     Serial.println(payload);
   //     Serial.println("=== End Response Payload ===");
   //   }
 
     // close HTTP connection
     http.end();
 
   }
   else {
     Serial.println("Connection attempt failed.");
   }
   
 }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000); //relax...
  Serial.println("Processor came out of reset.\n");

  connectWiFi();
  
  myIMU.settings.adcEnabled = 0;
  //Note:  By also setting tempEnabled = 1, temperature data is available
  //instead of ADC3 in.  Temperature *differences* can be read at a rate of
  //1 degree C per unit of ADC3 data.
  myIMU.settings.tempEnabled = 0;
  myIMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 2;      //Max G force readable.  Can be: 2, 4, 8, 16
  myIMU.settings.xAccelEnabled = 1;
  myIMU.settings.yAccelEnabled = 1;
  myIMU.settings.zAccelEnabled = 1;

  //FIFO control settings
  myIMU.settings.fifoEnabled = 1;
  myIMU.settings.fifoThreshold = 20;  //Can be 0 to 32
  myIMU.settings.fifoMode = 1;  //FIFO mode.
  //fifoMode can be:
  //  0 (Bypass mode, FIFO off)
  //  1 (FIFO mode)
  //  3 (FIFO until full)
  //  4 (FIFO when trigger)
  
  //Call .begin() to configure the IMU (except for the fifo)
  myIMU.begin();

  Serial.print("Configuring FIFO with no error checking...");
  myIMU.fifoBegin(); //Configure fifo
  Serial.print("Done!\n");
  
  Serial.print("Clearing out the FIFO...");
  myIMU.fifoClear();
  Serial.print("Done!\n");
  myIMU.fifoStartRec(); //cause fifo to start taking data (re-applies mode bits)

  // configure LS3IDH to generate watermark interrupt
  uint8_t registerData = 0;
  myIMU.readRegister(&registerData, LIS3DH_CTRL_REG3);
//   Serial.println(registerData, BIN);
  registerData |= 0x04;
//   Serial.println(registerData, BIN);
  myIMU.writeRegister(LIS3DH_CTRL_REG3, registerData);

  // configure ESP8266 external interrupt
  pinMode(4, INPUT);
  attachInterrupt(digitalPinToInterrupt(4), sensorISR, HIGH);
  
}

void loop()
{
  //float temp;  //This is to hold read data
  //uint16_t tempUnsigned;
  //
//   while(( myIMU.fifoGetStatus() & 0x80 ) == 0) {wdt_reset();};  //Wait for watermark
  
  //Now loop until FIFO is empty.
  //If having problems with the fifo not restarting after reading data, use the watermark
  //bits (b5 to b0) instead.
  //while(( myIMU.fifoGetStatus() & 0x1F ) > 2) //This checks that there is only a couple entries left
//   if (samplesReady){
  
//       while(( myIMU.fifoGetStatus() & 0x20 ) == 0) //This checks for the 'empty' flag
//       {
//          Serial.print(sampleNumber);
//          Serial.print(",");
//          Serial.print(myIMU.readRawAccelX());
//          Serial.print(",");
//          Serial.print(myIMU.readRawAccelY());
//          Serial.print(",");
//          Serial.print(myIMU.readRawAccelZ());
//          Serial.println();
//          // myIMU.readRawAccelX();
//          // myIMU.readRawAccelY();
//          // myIMU.readRawAccelZ();
//          sampleNumber++;
//          wdt_reset();
//       }
//       samplesReady = false;
//       Serial.print("sampleNumber = ");
//       Serial.println(sampleNumber);
//    }

   char entry[18] = "";
   char message[1800] = "";

   unsigned long loopStartTime = millis();
   
   while ((millis() - loopStartTime) < 30000) {

      wdt_reset();

      if( bufferCount >= 100 ) {

         Serial.println("Converting samples to CSV data....");
         Serial.print("bufferCount = ");
         Serial.println(bufferCount);

         detachInterrupt(4);
         for( int i = 0; i < 100; i++ ) {
            sprintf(entry, "%lu,%lu,%lu\n", deq(sampleBufferX), deq(sampleBufferY), deq(sampleBufferZ));
            strcat(message,entry);
         }
         attachInterrupt(digitalPinToInterrupt(4), sensorISR, HIGH);

         sendPostRequest(hostip, 8080, serverPath, message);

         memset(message, 0, sizeof(message));
         message[0] = '\0';

      }

   }

   detachInterrupt(4);
   while ( bufferCount > 0 ) {

      for( int i = 0; i < 100; i++ ) {
         sprintf(entry, "%lu,%lu,%lu\n", deq(sampleBufferX), deq(sampleBufferY), deq(sampleBufferZ));
         strcat(message,entry);
         if(bufferCount == 0)
            break;
      }
      sendPostRequest(hostip, 8080, serverPath, message);

      memset(message, 0, sizeof(message));
      message[0] = '\0';

   }

   while(1){
      wdt_reset();
   }

}