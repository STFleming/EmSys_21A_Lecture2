// author: stf

unsigned int * gpio_enable; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        gpio_enable = (unsigned int*)(0x3FF44020);

        Serial.println(*gpio_enable, BIN);

        // bitwise operations
        // a =      ...0110
        // b =      ...1001  
        // a | b == ...1111  
        // a & b == ...0000
        // !a    == ...1001
        // a || b == true / false
        // 
        // logical shifts: 
        // c =         ...0001
        // (c << 1) == ...0010
        // (c << 2) == ...0100
        // (c << 5) == .100000
        // d =         ...1000
        // (d >> 1) == ...0100

        // manipulate bits to set the exact part of the hardware
        // register you want to set
        Serial.print("\n\n");

        // enable GPIO 5 
        unsigned int new_gpio_enable = *gpio_enable;
        new_gpio_enable = new_gpio_enable | (1 << 5);
        *gpio_enable = new_gpio_enable;

        Serial.println(*gpio_enable, BIN);

        // exercise for you all... how can you disable a GPIO bit?
}

void loop() {

}  
