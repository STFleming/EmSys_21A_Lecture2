// author: stf

int a[8] = {42, 43, 44, 45, 46, 47, 48, 49}; // a variable
int *ptr_a; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        ptr_a = &a[0]; 

        for(int i=0; i<8; i++) {
            Serial.print("[0x");
            Serial.print((unsigned int)&a[i],HEX);
            Serial.print("] = ");
            Serial.println(a[i]);
        }

}


void loop() {

}
