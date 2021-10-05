// author: stf

unsigned int * unknown_memory_location; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        unknown_memory_location = (unsigned int*)(0x3000000);
        *unknown_memory_location = 1;
        
}

void loop() {

}  
