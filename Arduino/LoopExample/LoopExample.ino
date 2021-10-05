// Setting up the serial connection
// author: stf

void setup() {
        Serial.begin(115200);
        Serial.print("Hello World\n");
}

void loop() {
    delay(1000); // delay by 1000ms == 1s
    Serial.println("hi again...");
}
