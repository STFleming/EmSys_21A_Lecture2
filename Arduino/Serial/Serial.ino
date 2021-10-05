// Setting up the serial connection
// author: stf

void setup() {
        Serial.begin(115200);
}

void loop() {
        Serial.print("Hello World\n");
	delay(1000);
}
