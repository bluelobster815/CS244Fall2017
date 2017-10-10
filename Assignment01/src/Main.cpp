/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "Config.h"

// Set LED_BUILTIN if it is not defined by Arduino framework
// #define LED_BUILTIN 13

String getMacAddress() {
  // MAC addresses are 48 bits, i.e. 6 bytes.
  // We **could** read the byte representation of the MAC address into a a byte array and then convert to the string representation as in Deep's example.
  // However, the library provides a function that returns the string representation of the MAC address, so there is no need to parse it ourselves.
  return WiFi.macAddress();
}

void connectWiFi() {
  Serial.print("Connecting to WiFi network");
  // Initialize WiFi radio and attempt to connect to specified SSID.
  WiFi.begin(WiFiSSID); // Add password as second parameter when on a password-protected WiFi.
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

void setup()
{
  // ???
  // TODO how to view printouts from setup() function?
  // Serial Monitor is not open when the program is launched.
  // Has to be opened manually AFTER upload... (as it cannot run in parallel with the upload task)?
  // Alternatively one could add a delay at the start of setup()
  // ???
  delay(8000);
  
  // Set the data rate in bits per second (baud) for serial data transmission.
  //Serial.begin(921600);
  Serial.begin(9600);
  Serial.println("===   Booting...  ===");
  
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Print MAC Address.
  Serial.print("MAC Address: ");
  Serial.println(getMacAddress());
  
  // Connect to WiFi
  connectWiFi();

  Serial.println("=== Boot completed ===");
}

void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);

  // wait for a second
  delay(1000);

  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);

   // wait for a second
  delay(1000);

  // Serial.print("MAC address: ");
  // Serial.println(getMacAddress());
}