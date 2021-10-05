// author: stf

unsigned int *hardware_rng; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        hardware_rng = (unsigned int*)(0x3FF75144);
        
}

void loop() {
    delay(1);
    Serial.println(*hardware_rng);
}  
