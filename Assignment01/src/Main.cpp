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

// Library class that provides functionality for sending and recieving HTTP(S) requests and responses, respectively.
WiFiClientSecure webClient;

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

// Send an HTTP POST request
void sendPostRequest(char *host, uint16_t port, char *path, const char* hostFingerprint, char* messageBody) {
  // Attempt to establish a connection to the server.
  Serial.print("Attempting to connect to ");
  Serial.println(host);

  if (webClient.connect(host, port)) {
    // Verify website's fingerprint if we are using SSL (hostFingerprint should be left NULL/empty to indicate that we are to use regular HTTP).
    if (hostFingerprint != NULL && hostFingerprint[0] != '\0') {
      Serial.println("TLS request; verifying host's fingerprint...");
      if (webClient.verify(hostFingerprint, host)) {
        Serial.println("Fingerprint was verified; commencing request.");
      } else {
        Serial.println("Fingerprint does not match; abandoning request.");
        webClient.stop();
        return;
      }
    }

    Serial.println("Successfully connected.");
    Serial.println("Sending HTTP POST request...");
    
    Serial.println("============================");
    // Char buffer for formatting the HTTP request.
    // Note that in the code below, client is responsible for actually performing the HTTP request, but we also print to Serial for debugging purposes.
    char buffer[128];
    sprintf(buffer, "POST %s HTTP/1.1", path);
    Serial.println(buffer);
    webClient.println(buffer);
    sprintf(buffer, "Host: %s", host);
    Serial.println(buffer);
    webClient.println(buffer);
    sprintf(buffer, "Accept-Language: en");
    Serial.println(buffer);
    webClient.println(buffer);
    sprintf(buffer, "Connection: close");
    Serial.println(buffer);
    webClient.println(buffer);
    Serial.println();
    webClient.println(); // The empty line that seperates the HTTP request's header and body sections.
    Serial.println(messageBody);
    webClient.println(messageBody);
    Serial.println("============================");

    // Now read the server's response, character by character.
    Serial.println("Reading server's response...");
    Serial.println("============================");
    while (webClient.available()) {
      char c = webClient.read();
      Serial.print(c);
    }
    Serial.println();
    Serial.println("============================");
    
    // Clean up client side part of connection if connection closed by server.
    if (!webClient.connected()) {
      webClient.stop();
    }
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
  sendPostRequest("varmarken.com", 443, "/tools/echo_post.php", NULL, "hello");
  // Perform a request every 30 seconds.
  delay(30000);

  // // turn the LED on (HIGH is the voltage level)
  // digitalWrite(LED_BUILTIN, HIGH);

  // // wait for a second
  // delay(1000);

  // // turn the LED off by making the voltage LOW
  // digitalWrite(LED_BUILTIN, LOW);

  //  // wait for a second
  // delay(1000);

  // Serial.print("MAC address: ");
  // Serial.println(getMacAddress());
}