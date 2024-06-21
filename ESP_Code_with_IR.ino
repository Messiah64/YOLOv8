#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// WiFi credentials
const char* ssid = "Hacker";
const char* password = "mo123456";
const char* host = "192.168.169.215";
const int port = 12345;

// NeoPixel setup
#define NUM_PIXELS_PER_MATRIX 64 // Number of pixels per 8x8 matrix

// Define the GPIO pins for the 4 matrices
const int matrixPins[] = {38, 37, 36, 35};
Adafruit_NeoPixel matrices[] = {
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(NUM_PIXELS_PER_MATRIX, matrixPins[3], NEO_GRB + NEO_KHZ800)
};

#define IR_RECEIVE_PIN 5 // Using GPIO5 (D1) for the IR receiver pin

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

bool lightsOn = false; // Flag to track the light state

void setup() {
    Serial.begin(9600);
    delay(10); 

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

    irrecv.enableIRIn(); // Start the IR receiver
}

void loop() {
    if (irrecv.decode(&results)) {
        // IR remote signal received
        handleIRRemote();
        irrecv.resume(); // Resume receiving the next value
    } else {
        // No IR remote signal, handle server connection
        handleServerConnection();
    }
}

void handleIRRemote() {
    // Print the raw value in hexadecimal format
    Serial.print("Decoded raw data: ");
    Serial.println((unsigned long)(results.value), HEX);

    if (results.value == 0x20DF10EF) {
        if (lightsOn) {
            Serial.println("OFF Button Pressed");
            turnOffAllMatrices();
            lightsOn = false;
        } else {
            Serial.println("ON Button Pressed");
            turnOnAllMatrices();
            lightsOn = true;
        }
    } else {
        Serial.println("False Alarm");
        // turnOffAllMatrices();
        // lightsOn = false;
    }

    // Print human-readable details of the received signal
    Serial.println(resultToHumanReadableBasic(&results));

    // Stop the server connection after receiving the first IR signal
    WiFi.disconnect();
    Serial.println("Disconnected from server");
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
    int index = 0;
    String receivedData = "";

    while (true) {
        // Check for IR signal before processing server data
        if (irrecv.decode(&results)) {
            // IR remote signal received, break out of the loop
            Serial.println("IR signal received, exiting server connection");
            return;
        }

        if (client.available()) {
            char c = client.read();
            receivedData += c;
            Serial.print(c);

            // Check if the character is a digit
            if (isDigit(c)) {
                int value = c - '0'; // Convert char to int
                // Light up or turn off the corresponding matrix
                setMatrixState(index, value);
                index = (index + 1) % 4; // Wrap around to 0 after 3
            }
        }
        // Print the received data after processing all the characters
        if (receivedData.length() > 0) {
            Serial.print("\nReceived data: ");
            Serial.println(receivedData);
            receivedData = ""; // Clear the received data string
        }

        // Small delay to avoid blocking the IR receiver
        delay(10);
    }
}

void setMatrixState(int matrixIndex, int state) {
    for (int i = 0; i < NUM_PIXELS_PER_MATRIX; i++) {
        matrices[matrixIndex].setPixelColor(i, state ? matrices[matrixIndex].Color(255, 255, 255) : 0); // Set to white or turn off
    }
    matrices[matrixIndex].show(); // Update the LEDs
}

void turnOnAllMatrices() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < NUM_PIXELS_PER_MATRIX; j++) {
            matrices[i].setPixelColor(j, matrices[i].Color(255, 255, 255)); // Set all pixels to white
        }
        matrices[i].show(); // Update the LEDs
    }
}

void turnOffAllMatrices() {
    for (int i = 0; i < 4; i++) {
        matrices[i].clear(); // Turn off all pixels
        matrices[i].show(); // Update the LEDs
    }
}
