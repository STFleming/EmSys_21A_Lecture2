// author: stf

int a; // a variable
int *ptr_a; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        a = 10;
        ptr_a = &a; // ptr is pointing to the address of a

        Serial.print("Variable a has value:");
        Serial.println(a);

        Serial.print("and lives at address 0x");
        Serial.println((unsigned int)ptr_a, HEX);
}

void loop() {

}
