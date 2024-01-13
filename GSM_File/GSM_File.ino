#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(2, 3);  // TX  = 2,  RX = 3 from GSM TO existing board
const int triggerPin1 = 5;   // Digital input pin connected to the existing board
const int triggerPin2 = 6;  // Digital input pin connected to the existing board


void setup() {
 gsmSerial.begin(115200);
  Serial.begin(115280);
  Serial.println("GSM BEGIN");
  pinMode(triggerPin1, INPUT);
  pinMode(triggerPin2, INPUT);
  delay(100);
}


void loop() {
  if (digitalRead(triggerPin1) == HIGH) {
    SendMessage1();
    delay(1000);  // Add a delay to prevent multiple triggers in quick succession
  }
  if(digitalRead(triggerPin2) == HIGH) {
    SendMessage2();
    delay(1000);    
    
  }

} 

void SendMessage1() {
 gsmSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(100);

 gsmSerial.print("AT+CMGS=\"+94771659163\"\r"); // Replace with the recipient's phone number
 //gsmSerial.println("\"");
  delay(100);

 gsmSerial.print("BIN 1 full 80%"); // Replace with your message
  delay(100);
 gsmSerial.write(26); // End of message character
  delay(2000); // Wait for response

  // Read and handle response
  String response = "";
  while (gsmSerial.available()) {
    char c =gsmSerial.read();
    response += c;
  }

 Serial.println("Raw response: " + response);

  if (response.indexOf("OK") != -1) {
    Serial.println("Message sent successfully!");
  } else if (response.indexOf("SEND OK") != -1) {
    Serial.println("Message sent and acknowledged!");
  } else if (response.indexOf("SEND FAIL") != -1) {
    Serial.println("Message sending failed.");
  } else {
    Serial.println("Unexpected response: " + response);
  }
}


void SendMessage2() {
 gsmSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(100);

 gsmSerial.print("AT+CMGS=\"+94771659163\"\r"); // Replace with the recipient's phone number
 //gsmSerial.println("\"");
  delay(100);

 gsmSerial.print("BIN 2 full 80%"); // Replace with your message
  delay(100);
 gsmSerial.write(26); // End of message character
  delay(2000); // Wait for response

  // Read and handle response
  String response = "";
  while (gsmSerial.available()) {
    char c =gsmSerial.read();
    response += c;
  }

 Serial.println("Raw response: " + response);

  if (response.indexOf("OK") != -1) {
    Serial.println("Message sent successfully!");
  } else if (response.indexOf("SEND OK") != -1) {
    Serial.println("Message sent and acknowledged!");
  } else if (response.indexOf("SEND FAIL") != -1) {
    Serial.println("Message sending failed.");
  } else {
    Serial.println("Unexpected response: " + response);
  }
}