// author: stf

unsigned int a; // a variable
unsigned int *ptr_a; 
char *cptr; 

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");


        a = 0xAABBCCDD;
        ptr_a = &a; // ptr is pointing to the address of a

        Serial.print("Variable a has value:");
        Serial.println(a);
        Serial.print("and lives at address 0x");
        Serial.println((unsigned int)ptr_a, HEX);

        Serial.print("\n\n");

        cptr = (char *)ptr_a;

        for(int i=0; i<4; i++) {
            Serial.println(*(cptr + i), HEX);
        }
}

void loop() {

}
