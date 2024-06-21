#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
const char* ssid = "Hacker";
const char* password = "mo123456";
const char* host = "192.168.181.113";
const int port = 12345;

// NeoPixel setup
#define NUM_PIXELS_PER_MATRIX 64 // Number of pixels per 8x8 matrix

// Supabase Credentials
String API_URL = "https://hjhxraozowuswznxacop.supabase.co";
String API_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImhqaHhyYW96b3d1c3d6bnhhY29wIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTcxOTAwMzk5NiwiZXhwIjoyMDM0NTc5OTk2fQ.4CA6hh5Zo51B4OljhXjGGr7w0U7GQ3ZNxdnNRFA_96Y";
String TableName = "maintable";
int sendingInterval = 2;
HTTPClient https;
WiFiClientSecure secure_client;

// Create 4 global variables for the 4 quadrants
int q1, q2, q3, q4;

// Define the GPIO pins for the 4 matrices
const int matrixPins[] = {38, 37, 36, 35};
Adafruit_NeoPixel matrices[] = {
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[3], NEO_GRB + NEO_KHZ800)
};

void setup() {
    Serial.begin(115200);
    delay(10);

    secure_client.setInsecure(); // Don't check for HTTPS Security since not needed

    // Initialize each matrix
    for (int i = 0; i < 4; i++) {
        matrices[i].begin();
        matrices[i].show(); // Initialize all pixels to 'off'
    }

    // Connect to WiFi
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    handleServerConnection();
}

void handleServerConnection() {
    Serial.print("Connecting to ");
    Serial.print(host);
    Serial.print(':');
    Serial.println(port);

    // Connect to server
    WiFiClient client;
    if (!client.connect(host, port)) {
        Serial.println("Connection failed");
        delay(5000);
        return;
    }

    Serial.println("Response from server:");
    String receivedData = "";

    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            receivedData += c;
            Serial.print(c);

            // Check if we have received the complete array (e.g., "[1, 2, 3, 4]")
            if (c == ']') {
                // Parse the received data
                parseAndSetMatrixState(receivedData);
                receivedData = ""; // Clear the received data string
                Serial.print("\n");
                
                // Send the parsed data to Supabase
                sendPostRequest(q1, q2, q3, q4);
            }
        }
        // Small delay to avoid blocking
        delay(10);
    }

    client.stop();
    Serial.println("Disconnected from server");
    delay(5000);  // Wait before reconnecting
}

void parseAndSetMatrixState(String data) {
    // Remove square brackets
    data.remove(0, 1); // Remove the first character '['
    data.remove(data.length() - 1); // Remove the last character ']'

    // Split the string by comma and convert to integers
    int counts[4];
    int index = 0;
    while (data.length() > 0 && index < 4) {
        int commaIndex = data.indexOf(',');
        if (commaIndex == -1) {
            counts[index] = data.toInt();
            break;
        } else {
            counts[index] = data.substring(0, commaIndex).toInt();
            data = data.substring(commaIndex + 1);
            index++;
        }
    }

    // Set the state of each matrix
    for (int i = 0; i < 4; i++) {
        setMatrixState(i, counts[i]);
    }

    // Store the values in the global variables
    q1 = counts[0];
    q2 = counts[1];
    q3 = counts[2];
    q4 = counts[3];
}

void setMatrixState(int matrixIndex, int state) {
    for (int i = 0; i < NUM_PIXELS_PER_MATRIX; i++) {
        matrices[matrixIndex].setPixelColor(i, state ? matrices[matrixIndex].Color(255, 255, 255) : 0); // Set to white or turn off
    }
    matrices[matrixIndex].show(); // Update the LEDs
}

void sendPostRequest(int q1, int q2, int q3, int q4) {
    if (WiFi.status() == WL_CONNECTED) {
        https.begin(secure_client, API_URL + "/rest/v1/" + TableName);
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Prefer", "return=representation");
        https.addHeader("apikey", API_KEY);
        https.addHeader("Authorization", "Bearer " + API_KEY);

        String jsonPayload = "{\"q1\":" + String(q1) + ",\"q2\":" + String(q2) + ",\"q3\":" + String(q3) + ",\"q4\":" + String(q4) + "}";
        int httpCode = https.POST(jsonPayload);   // Send the request
        String payload = https.getString();

        Serial.println(httpCode);   // Print HTTP return code
        Serial.println(payload);    // Print request response payload

        https.end();
    } else {
        Serial.println("WiFi not connected");
    }
}
