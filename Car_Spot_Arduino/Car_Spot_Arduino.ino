#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

// WiFi and server credentials
const char* ssid = "MOSTAFA";
const char* password = "0987654321";
const char* serverAddress = "192.168.137.106";
int port = 5000;

// Pin definitions for spot 1
#define TRIGGER_PIN_1 2
#define ECHO_PIN_1 3
#define RED_PIN_1 10
#define GREEN_PIN_1 12

// Pin definitions for spot 2
#define TRIGGER_PIN_2 21
#define ECHO_PIN_2 20
#define RED_PIN_2 19
#define GREEN_PIN_2 18

// Pin definitions for spot 3
#define TRIGGER_PIN_3 14
#define ECHO_PIN_3 15
#define RED_PIN_3 16
#define GREEN_PIN_3 17

#define THRESHOLD 50 // 100 cm threshold

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

// Previous status of spots
bool prevStatusSpot1 = false;
bool prevStatusSpot2 = false;
bool prevStatusSpot3 = false;

void setup() {
  Serial.begin(9600);

  // Initialize WiFi connection
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, password);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Initialize pins for spot 1
  pinMode(TRIGGER_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(RED_PIN_1, OUTPUT);
  pinMode(GREEN_PIN_1, OUTPUT);
  digitalWrite(GREEN_PIN_1, HIGH); // Default to green
  digitalWrite(RED_PIN_1, LOW);

  // Initialize pins for spot 2
  pinMode(TRIGGER_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(RED_PIN_2, OUTPUT);
  pinMode(GREEN_PIN_2, OUTPUT);
  digitalWrite(GREEN_PIN_2, HIGH); // Default to green
  digitalWrite(RED_PIN_2, LOW);

  // Initialize pins for spot 3
  pinMode(TRIGGER_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
  pinMode(RED_PIN_3, OUTPUT);
  pinMode(GREEN_PIN_3, OUTPUT);
  digitalWrite(GREEN_PIN_3, HIGH); // Default to green
  digitalWrite(RED_PIN_3, LOW);
}

void loop() {
  Spot1();
  Spot2();
  Spot3();

  delay(1000);
}

void Spot1() {
  long duration, cm;

  digitalWrite(TRIGGER_PIN_1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_1, LOW);

  duration = pulseIn(ECHO_PIN_1, HIGH);

  cm = microsecondsToCentimeters(duration);

  bool currentStatus = (cm > 0 && cm < THRESHOLD);
  if (currentStatus != prevStatusSpot1) {
    controlLED(RED_PIN_1, GREEN_PIN_1, currentStatus, 1);
    prevStatusSpot1 = currentStatus;
  }
}

void Spot2() {
  long duration, cm;

  digitalWrite(TRIGGER_PIN_2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_2, LOW);

  duration = pulseIn(ECHO_PIN_2, HIGH);

  cm = microsecondsToCentimeters(duration);

  bool currentStatus = (cm > 0 && cm < THRESHOLD);
  if (currentStatus != prevStatusSpot2) {
    controlLED(RED_PIN_2, GREEN_PIN_2, currentStatus, 2);
    prevStatusSpot2 = currentStatus;
  }
}

void Spot3() {
  long duration, cm;

  digitalWrite(TRIGGER_PIN_3, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_3, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_3, LOW);

  duration = pulseIn(ECHO_PIN_3, HIGH);

  cm = microsecondsToCentimeters(duration);

  bool currentStatus = (cm > 0 && cm < THRESHOLD);
  if (currentStatus != prevStatusSpot3) {
    controlLED(RED_PIN_3, GREEN_PIN_3, currentStatus, 3);
    prevStatusSpot3 = currentStatus;
  }
}

void controlLED(int redPin, int greenPin, bool status, int spotNumber) {
  if (status) {
    digitalWrite(redPin, HIGH);   // Something is within the threshold, turn red LED on
    digitalWrite(greenPin, LOW);  // Ensure green LED is off
    spotTaken(spotNumber); // Update the spot in server
  } else {
    digitalWrite(greenPin, HIGH); // Nothing within the threshold, turn green LED on
    digitalWrite(redPin, LOW);    // Ensure red LED is off
    spotFree(spotNumber); // Update the spot in server
  }
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

void spotTaken(int spotNumber) {
  DynamicJsonDocument jsonDoc(2048); // Create a JSON document
  jsonDoc["spotNumber"] = String(spotNumber); // Convert spotNumber to string
  jsonDoc["occupied"] = true; // Add the occupied property

  String data;
  serializeJson(jsonDoc, data); // Convert the JSON document to a string

  Serial.println("Data sent:");
  Serial.println(data); // Print the data sent

  client.post("/spot-taken", "application/json", data);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

void spotFree(int spotNumber) {
  DynamicJsonDocument jsonDoc(2048); // Create a JSON document
  jsonDoc["spotNumber"] = String(spotNumber); // Convert spotNumber to string
  jsonDoc["occupied"] = false; // Add the occupied property

  String data;
  serializeJson(jsonDoc, data); // Convert the JSON document to a string

  Serial.println("Data sent:");
  Serial.println(data); // Print the data sent

  client.post("/spot-free", "application/json", data);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}