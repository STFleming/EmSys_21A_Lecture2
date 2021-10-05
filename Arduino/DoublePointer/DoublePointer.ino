// author: stf

int a; // a variable
int *ptr_a; // a variable that is a pointer
int **ptr_ptr_a;
int ***ptr_ptr_ptr_a;

void setup() {
        Serial.begin(115200);
        a = 10;
        ptr_a = &a; // ptr is pointing to the address of a
        ptr_ptr_a = &ptr_a; // a pointer to a pointer of a
        ptr_ptr_ptr_a = &ptr_ptr_a;

        Serial.print("Variable a has value:");
        Serial.println(a);
        Serial.println((unsigned int)ptr_a, HEX);

        Serial.print("Double dereference: ");
        Serial.println(**ptr_ptr_a);
        Serial.print("Triple dereference: ");
        Serial.println(***ptr_ptr_ptr_a);
}

void loop() {

}
