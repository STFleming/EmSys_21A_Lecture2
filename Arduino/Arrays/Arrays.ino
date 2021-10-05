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

        Serial.print("\n\n\n");
        
        for(int i=0; i<8; i++) {
                Serial.print("[0x");
                Serial.print((unsigned int)(ptr_a), HEX);
                Serial.print("] = ");
                Serial.println(*ptr_a);
                ptr_a = ptr_a + 1;
        }

        Serial.print("\n\n\n");
        // a[b] == *(a + b)
        Serial.println(0[a]); 
        Serial.println(1[a]);
        Serial.println(2[a]);
        Serial.println(3[a]); // 3[a] == *(3 + a) == *(a + 3)
        Serial.println(4[a]);
        Serial.println(5[a]);
        
        
        
}

void loop() {

}
