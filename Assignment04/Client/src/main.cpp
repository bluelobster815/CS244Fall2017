#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

LIS3DH myIMU(SPI_MODE, 16); //Constructing with SPI interface information
//LIS3DH myIMU(I2C_MODE, 0x19); //Alternate constructor for I2C

uint32_t sampleNumber = 0; //Used to make CSV output row numbers

volatile bool samplesReady = false;

void sensorISR(){
   samplesReady = true;
   // Serial.println("interrup fired!");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000); //relax...
  Serial.println("Processor came out of reset.\n");
  
  myIMU.settings.adcEnabled = 0;
  //Note:  By also setting tempEnabled = 1, temperature data is available
  //instead of ADC3 in.  Temperature *differences* can be read at a rate of
  //1 degree C per unit of ADC3 data.
  myIMU.settings.tempEnabled = 0;
  myIMU.settings.accelSampleRate = 10;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
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

  uint8_t registerData = 0;
  myIMU.readRegister(&registerData, LIS3DH_CTRL_REG3);
  Serial.println(registerData, BIN);
  registerData |= 0x04;
  Serial.println(registerData, BIN);
  myIMU.writeRegister(LIS3DH_CTRL_REG3, registerData);

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
  if (samplesReady){
  
      while(( myIMU.fifoGetStatus() & 0x20 ) == 0) //This checks for the 'empty' flag
      {
         Serial.print(sampleNumber);
         Serial.print(",");
         Serial.print(myIMU.readRawAccelX());
         Serial.print(",");
         Serial.print(myIMU.readRawAccelY());
         Serial.print(",");
         Serial.print(myIMU.readRawAccelZ());
         Serial.println();
         // myIMU.readRawAccelX();
         // myIMU.readRawAccelY();
         // myIMU.readRawAccelZ();
         sampleNumber++;
         wdt_reset();
      }
      samplesReady = false;
      Serial.print("sampleNumber = ");
      Serial.println(sampleNumber);
   }

}