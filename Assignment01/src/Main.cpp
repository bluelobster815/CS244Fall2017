/**
 * CS244 Assignment 1 - Sensor Node to Cloud Data Transfer
 * 
 * Team #7
 *
 */

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
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
void sendPostRequest(char *host, uint16_t port, char *path, char* messageBody) {
  
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
  
  // Get Wifi received signal strength indicator to send as message to server
  int rssi = WiFi.RSSI();
  char message[20];
  sprintf(message, "RSSI = %d", rssi);

  Serial.println();
  Serial.print("message to send: ");
  Serial.println(message);

  sendPostRequest("172.16.0.112", 8080, "/cs244/webapi/data/upload/", message);
  
  // Perform a request every 30 seconds.
  delay(30000);

}