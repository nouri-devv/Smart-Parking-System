#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// WiFi and server credentials
const char* ssid = "MOSTAFA"; // WiFi SSID
const char* password = "0987654321"; // WiFi password
const char* serverAddress = "192.168.137.106"; // Server IP address
int port = 5000; // Server port

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

#define THRESHOLD 50 // Distance threshold for detecting an object in centimeters

WiFiClient wifi; // WiFi client
HttpClient client = HttpClient(wifi, serverAddress, port); // HTTP client
int status = WL_IDLE_STATUS; // WiFi connection status

// Previous status of spots to detect changes
bool prevStatusSpot1 = false;
bool prevStatusSpot2 = false;
bool prevStatusSpot3 = false;

#define MAX_ATTEMPTS 5 // Maximum attempts to read from the sensor
float lastValidReading1 = -1; // Last valid reading for spot 1
float lastValidReading2 = -1; // Last valid reading for spot 2
float lastValidReading3 = -1; // Last valid reading for spot 3

// Historical data for sensor readings
int historicalData1[3] = {0, 0, 0};
int historicalData2[3] = {0, 0, 0};
int historicalData3[3] = {0, 0, 0};

int currentIndex1 = 0;
int currentIndex2 = 0;
int currentIndex3 = 0;

void setup() {
  Serial.begin(9600); // Initialize serial communication

  // Initialize WiFi connection
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, password);
    delay(10000); // Add delay to prevent rapid retries
  }

  // Print the SSID of the network
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print the WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Initialize pins for spot 1
  pinMode(TRIGGER_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(RED_PIN_1, OUTPUT);
  pinMode(GREEN_PIN_1, OUTPUT);
  digitalWrite(GREEN_PIN_1, HIGH); // Default to green (spot is free)
  digitalWrite(RED_PIN_1, LOW);

  // Initialize pins for spot 2
  pinMode(TRIGGER_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(RED_PIN_2, OUTPUT);
  pinMode(GREEN_PIN_2, OUTPUT);
  digitalWrite(GREEN_PIN_2, HIGH); // Default to green (spot is free)
  digitalWrite(RED_PIN_2, LOW);

  // Initialize pins for spot 3
  pinMode(TRIGGER_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
  pinMode(RED_PIN_3, OUTPUT);
  pinMode(GREEN_PIN_3, OUTPUT);
  digitalWrite(GREEN_PIN_3, HIGH); // Default to green (spot is free)
  digitalWrite(RED_PIN_3, LOW);
}

void loop() {
  Spot1(); // Check status of spot 1
  Spot2(); // Check status of spot 2
  Spot3(); // Check status of spot 3

  delay(1000); // Delay between checks
}

void Spot1() {
  float distance = readSensor(TRIGGER_PIN_1, ECHO_PIN_1, lastValidReading1, historicalData1, currentIndex1); // Read distance from sensor

  bool currentStatus = (distance > 0 && distance < THRESHOLD); // Determine if spot is occupied
  if (currentStatus != prevStatusSpot1) { // Check if status has changed
    controlLED(RED_PIN_1, GREEN_PIN_1, currentStatus, 1); // Update LEDs and server status
    prevStatusSpot1 = currentStatus; // Update previous status
  }
}

void Spot2() {
  float distance = readSensor(TRIGGER_PIN_2, ECHO_PIN_2, lastValidReading2, historicalData2, currentIndex2); // Read distance from sensor

  bool currentStatus = (distance > 0 && distance < THRESHOLD); // Determine if spot is occupied
  if (currentStatus != prevStatusSpot2) { // Check if status has changed
    controlLED(RED_PIN_2, GREEN_PIN_2, currentStatus, 2); // Update LEDs and server status
    prevStatusSpot2 = currentStatus; // Update previous status
  }
}

void Spot3() {
  float distance = readSensor(TRIGGER_PIN_3, ECHO_PIN_3, lastValidReading3, historicalData3, currentIndex3); // Read distance from sensor

  bool currentStatus = (distance > 0 && distance < THRESHOLD); // Determine if spot is occupied
  if (currentStatus != prevStatusSpot3) { // Check if status has changed
    controlLED(RED_PIN_3, GREEN_PIN_3, currentStatus, 3); // Update LEDs and server status
    prevStatusSpot3 = currentStatus; // Update previous status
  }
}

// Read sensor data with error handling
float readSensor(int triggerPin, int echoPin, float &lastValidReading, int historicalData[], int &currentIndex) {
  long duration;
  float distance;

  for (int attempts = 0; attempts < MAX_ATTEMPTS; attempts++) {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration * 0.034) / 2;

    if (distance > 0 && distance < 400) { // Valid range for ultrasonic sensor
      lastValidReading = distance; // Update last valid reading
      historicalData[currentIndex] = distance;
      currentIndex = (currentIndex + 1) % 3;
      return distance;
    }
    delay(100); // Short delay before retrying
  }

  // If sensor fails, use the estimated distance from historical data
  int estimatedDistance = (historicalData[0] + historicalData[1] + historicalData[2]) / 3;
  return estimatedDistance;
}

// Control LEDs based on spot status and update server
void controlLED(int redPin, int greenPin, bool status, int spotNumber) {
  if (status) {
    digitalWrite(redPin, HIGH);   // Spot is occupied, turn red LED on
    digitalWrite(greenPin, LOW);  // Turn green LED off
  } else {
    digitalWrite(greenPin, HIGH); // Spot is free, turn green LED on
    digitalWrite(redPin, LOW);    // Turn red LED off
    updateSpotStatus(spotNumber, false); // Notify server spot is free
  }
}

// Update server with spot status and add checksum for data integrity
void updateSpotStatus(int spotNumber, bool occupied) {
  DynamicJsonDocument jsonDoc(2048); // Create a JSON document
  jsonDoc["spotNumber"] = String(spotNumber); // Add spot number
  jsonDoc["occupied"] = occupied; // Add occupied status

  String data;
  serializeJson(jsonDoc, data); // Convert JSON document to a string

  String checksum = calculateChecksum(data); // Calculate checksum
  jsonDoc["checksum"] = checksum; // Add checksum to the JSON document
  serializeJson(jsonDoc, data); // Convert the updated JSON document to a string

  Serial.println("Data sent:");
  Serial.println(data); // Print the data sent

  // Send data to server with retry mechanism
  if (!postWithRetry("/spot-free", "application/json", data)) {
    Serial.println("Failed to send data after multiple attempts.");
  }
}

// Post data to server with retry mechanism
bool postWithRetry(const char* path, const char* contentType, const String& data) {
  const int MAX_RETRIES = 5;
  const int RETRY_DELAY = 1000; // 1 second delay between retries

  for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
    client.post(path, contentType, data); // Send POST request

    int statusCode = client.responseStatusCode(); // Get response status code
    String response = client.responseBody(); // Get response body

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (statusCode == 200) { // Assuming 200 is the success status code
      return true; // Successful request
    } else {
      Serial.print("Attempt ");
      Serial.print(attempt);
      Serial.println(" failed. Retrying...");
      delay(RETRY_DELAY); // Delay before retrying
    }
  }
  return false; // All attempts failed
}

// Calculate checksum for data integrity
String calculateChecksum(String data) {
  uint8_t checksum = 0;
  for (int i = 0; i < data.length(); i++) {
    checksum ^= data[i]; // XOR each byte to calculate checksum
  }
  return String(checksum, HEX); // Return checksum as a hex string
}
