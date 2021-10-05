# EmSys Autumn 2021: Lecture 2
In this lecture we will explore:
* What a System-on-Chip is.
* Why C is the language of choice for programming such systems.
* The basics on an Arduino sketch. 

This will all be framed in the context of your TinyPico device.

## Your TinyPico

![](imgs/TinyPico-01.png)

Above is a doodle of your TinyPico device showing _some_ of the components on the TinyPico printed circuit board (PCB). The heart of the system is the ESP32 microcontroller unit that contains two processor cores that are capable of running at 240MHz, along with a bunch of other goodies that we will explore in a little while.

However, there is a lot more than just the ESP32 on this board. There is an RGB LED, the APA102 2020, a USB to UART converter, a power management integrated circuit, and much much more. Each of these components can be considered an embedded system in their own right, and when combined as they are on the TinyPico board, they also make an embedded system. The point that I am attempting to make here is that embedded systems are hierarchical. Embedded systems are made from embedded systems, which are made from embedded systems.  

Let's take just one example from the TinyPico board, the USB to UART Bridge chip. We can examine the datasheet [[here](https://www.silabs.com/documents/public/data-sheets/cp2104.pdf)] and see that this device is pretty complicated. (Don't worry, I don't expect you to be able to understand this datasheet).

![](imgs/usb-2-uart.png)

Looking at the system diagram for this device (taken from the datasheet) we can see that it has controller hardware, buffers for sending and receiving data, oscillators, and programmable memory. This is a dedicated digital circuit that needs to recieve, process, and transmit data with very precise timing guarantees. In isolation it is an embedded system in it's own right. 

## Zoom in on the ESP32

Let's zoom in on just the ESP32, the heart of our TinyPico device.

![](imgs/TinyPico_esp32_highlighted-01.png)


The ESP32 is what is known as a System-on-Chip (SoC). A SoC is an integrated circuit that combines a central processing unit (CPU) with, memory, input/output peripherals, radio modems, graphics processing units (GPUs), all in a single package.
Integrating common peripherals into a single chip makes a lot of sense. As every time you want to send a signal between two subcomponents of your computer you have to use lots of power and incur considerable latency. 

Let's take a look at what the internal system diagram of the ESP32 looks like:

![](imgs/ESP32_Functional_block_diagram.svg)

At the center of the ESP32 SoC we can see a dual-core 32-bit Xtensa processor, with some memory (in our case we have a dual core system). This is a small RISC microprocessor, meaning it has a minimal instruction set architecture, that runs at a maximum 240MHz. __One thing that you might notice, is that this processor up a reasonably small amount of area on the SoC system diagram, there is a lot of other stuff going on here.__ 

![](imgs/ESP32_cores_highlight.svg)

__Let's explore some of it at a high-level.__ On the left, you'll see a lot of I/O hardware, which is used for the SoC to talk to other systems, such as UART which we looked at [[last lecture](https://github.com/STFleming/EmSys_21A_Lecture1)]. Or is used to interface with the outside world in some way, such as the Analog to digital converters (SAR ADC) or Digital to Analog converters (DACs). We'll be exploring these a bit in later lectures, both the communication hardware (I2C/SPI/etc..) and the analog and digital converters.   

![](imgs/ESP32_io_highlight.svg)

At the top, you'll see radio hardware used for decoding WiFi and bluetooth signals. Having these sorts of radio hardware integrated right next to the processor in the same silicon package enables serious savings in power and overall energy useage, especially for IoT type devices. Exploring power optimisations on this device is something that we will be doing later on in the module also. We can notice that the WiFi modules and the bluetooth modules also share the same underlying radio hardware.   

![](imgs/ESP32_radio_highlight.svg)

To the right of the microprocessors we can see some custom hardware for performing cryptographic operations. Much like how graphics processors (GPUs) are optimised for performing graphical calculations these hardware units have been optimised for performing cryptographic operations. Such operations are very common for IoT applications, so it is likely a wide range of users will benefit from hardware acceleration of the operations. Custom acceleration of these functions greatly decreases the power and latency cost of performing these operations compared to the microprocessor. On the device there is hardware for AES, SHA, RSA, and efficient hardware random number generators (RNG). We might look at the hardware random number generators later on in this lecture. 

![](imgs/ESP32_crypto_highlight.svg)

Below the processor there is the surprsingly, another processor. This is the ultra-low power processor (ULP), which is a small finite-state machine based processor that can be used to sample I/O peripherals, read the low-power memory and perform very basic operations. The ULP operates in low-power modes when the majority of the device is switched off and allows for systems to be developed that can last for years on battery power.  

![](imgs/ESP32_ulp_highlight.svg)

## Software on SoCs

Usually a SoC will contain some sort of general purpose programmable hardware, in this case the Dual Core RISC Xtensa processor. These components in the system can be programmed to interact with the hardware in the rest of the system. Almost exclusively the language for doing this is C, however, languages such as Python and Rust are slowly starting to become more common. For this course we will use C, I find a lot of students asking?

__Why are we using such an old language?__

C has been around for a long time (1973), but with good reason. When we are using C we are operating quite closely to the hardware and have to write it in a way that explicitly considers the underlying architecture. [[Here is a video of Linus more elegantly explaining what I mean](https://www.youtube.com/watch?v=CYvJPra7Ebk)]. In particular, with C, we need to explictly state how we are laying out data in memory and how we are accessing that memory.    

For interacting with hardware C has something called pointers, which are crucial. These are variables that instead of storing a value store an address of a memory location. We can use these pointers to manipulate those memory locations directly. This is both incredibly powerful, and incredibly dangerous, which is why many programming languages abstract that knowledge away and hide it. 
In this lecture, we are going to little bits of C and illustrate why these pointers are so important, all through the context of Arduino.

## How does software interact with hardware?

I just told you how we typically program parts of our SoCs, but how can we interact with hardware from the software? __The answer is something called memory-mapped I/O__. Hardware components, such as our UART harware module, are given a memory address in the system and connected to the memory subsystem. When _anything_ reads or writes to these addresses, be it hardware or software, they are reading and writing to small amounts of memory within the hardware. The data in these small amounts of memory can be used to configure the hardware, or to trigger it to perform operations.  

Let's look at the address space of our ESP32 device.

![](imgs/esp32_address_space.png)

The above picture is taken from the [[ESP32 technical reference manual (TRM)](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)]. What we can see from this map is that we have regions of the address space that are for traditional memory that we can read and write to, such as ``0x3FF80000 - 0x3FFFFFFFF`` for internal memory, and ``0x3F400000 - 0x3F7FFFF`` for external memory. However, we can also see that a sizeable chunk of the address space, ``0x3FF00000 - 0x3FF7FFFF`` is devoted to peripherals. Addresses in this range are used for directly interacting with the hardware, such as the UART controller.

__Memory-mapped IO (MMIO) is a beautiful abstraction.__ All systems have memory for storing state, all systems need to interact with that memory reading and writing to addresses, so by mapping hardware components into memory we have a universal interface abstraction across devices.

__MMIO makes interacting with hardware from C simple.__ We can create a pointer that points to the address of the hardware, and then manipulate the hardware directly.

Let's look into programming our ESP32 with C and interact with some hardware.

## Programming the ESP32 SoC 

To program our ESP32 SoC we will be using Arduino. Arduino is a convienient framefork for developing such systems. We can easily manage packages, compile code for our device, and upload it all in a single IDE. However, this ease of use does come at a cost, and sometimes using the Arduino abstractions will limit our systems performance. In this module we will also explore the cost of these abstractions and how to bypass them to dirctly interact with hardware. 

But let's start with the basics, how to write a "Hello World!" program.

### An Arduino sketch

The Arduino terminology for a project containing code that we will compile and upload to our board is called a sketch. 
Technically Arduino code is C++, however, this course will be using primarily the C subset of C++ for interacting with our devices. I'll leave C++ for Martin to explain on AOOP.
A sketch has the basic following form.

```C
void setup() {

}

void loop() {

}
```

There are two functions: 
* ``setup()`` which is called once when the device boots.
* ``loop()`` which is called continuously while the device is running.


Now, our TinyPico has no screen attached to it, so how are we going to write "Hello World!" ? That's right, over the UART connection that we used to program the device.
The first thing that we need to do is configure our serial connection. In the ``setup()`` function we should add the following:

```C
void setup() {
	Serial.begin(115200);
}
```

The call to S``Seril.begin(115200);`` configures the UART hardware peripheral and tells it that we will be communicated at the baudrate ``115200``. 

To send data down the UART channel to our device we can then call the following in our ``loop()`` function.

```C
void loop() {
	Serial.print("Hello World!\n");
	delay(1000);
}
```

``Serial.print("Hello World\n");`` places the characters of the string "Hello World!" into a buffer in the hardware, after a while this buffer gets flushed and the receiving end can see the message. The d``delay(1000);`` just pauses the processor for a set amount of time (in milliseconds), in this case it will pause the processor for one second. 

On Arduino we can compile and upload this code, and then using the Serial monitor tool view "Hello World!" being sent from our device via the UART/USB connection.  

Now in these lectures I'm not going to go through the basics of the C and the syntax. Things like variables, control flow, functions... This is something that I expect you will be able to pick up as I go along. However, if you are feeling unsure about things I would recommend looking at some of the basic Arduino tutorials, or scheduling a meeting with me (Shane) to chat about things. 

__One thing that I do want to explain in detail however, is pointers__

## C Pointers

Earlier, I discussed the concept of mapping hardware into the address space of the system. By providing memory addresses for hardware we create an incredibly powerful and convenient abstraction for manipulating hardware from software. Every microprocessor will have memory; memory needs addresses, so using the same convention our custom hardware component can be compatible with all systems. 

Now, dealing with memory is dangerous. For instance, it's easy to write into memory regions you shouldn't accidentally, or you can forget to free it when you've finished using it. For these reasons, and many others, most languages perform memory management for the programmer. However, C does not. In C, we have the idea of a pointer -- a variable that instead of storing a value, stores a memory location. The pointers provided by C allow us to manipulate memory directly, and thus we can also interact directly with memory-mapped hardware. It's this reason that almost all drivers and deep parts of operating systems use the C language, and one of the reasons it is still a prevalent language since its development in 1972. 

### Pointers
Look at the following arduino sketch

```C

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
```
[[This code can be found here](Arduino/BasicPointer)]

In this sketch we have a few variables, ``a`` which is assigned the value ``10`` and ``ptr_a`` which is a pointer variable. To declare a pointer we need to use the ``*`` operator, we place this after the type in the variable declaration to make our variable a pointer of that type. In this example, we have ``int * ptr_a;`` which says that ``ptr_a`` is a pointer to an integer type. We can make pointers to any type we wish, for instance, ``float * ptr_b;`` would make a pointer to a floating point variable.  

In order to effectively use a pointer, we need to have the ability to lookup where items are stored in memory. To do this we can use the ``&`` operator. In the example above, we have the line ``ptr_a = &a``, what this effectively says is,_"get the address of where variable ``a`` is stored, and store that address in ``ptr_a``"_.

Let's compile the code and see what the output looks like on the ``Serial`` monitor in Arduino.

```
Variable a has value:10
and lives at address 0x3FFC00C8
```

We can see here that the value of the variable was printed along with it's address ``0x3FFC00C8`` in Hexadecimal. We can now dive into the [[ESP32 Technical Reference Manual (TRM)](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)] to find out what this address region corresponds to in the device we will be working with on this course, the ESP32 System-on-Chip (SoC).

On Page 27 of the TRM we can see the following chart.

![](imgs/address_map_general.png)

What we can see from the chart that our variable ``a`` which has address ``0x3FFC00C8`` lies in the region ``0x3FF80000 - 0x3FFFFFFF`` which maps into the ``512KB Embedded Memory`` of our SoC. Scrolling down a bit further we can find a further breakdown of the address space for the embedded memory.

![](imgs/embedded_memory.png)

From this we can see that our address ``0x3FFC00C8`` is in the region ``0x3FFAE0000 - 0x3FFDFFFF`` which is the ``Internal SRAM 2``, a very fast memory quite close to the CPU in our SoC.

### Derefrencing a pointer

We can also manipulate the data stored at the address a pointer point to. To use pointers in this way we need to do something called derefrencing.

Take the following example:

```C
int a; // a variable
int *ptr_a; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        a = 10;
        ptr_a = &a; // ptr is pointing to the address of a

        *ptr_a = 40;

        Serial.print("Dereferencing a = ");
        Serial.println(*ptr_a);
}
```

This example is almost the same as the previous one, again we have ``a`` and ``ptr_a``. However, now we also have some other operations again using the ``*`` operator.

In the line ``*ptr_a = 40;`` we are using the ``*`` operator to assign the value 40 __into the location that the pointer is pointing to__.
In this case ``ptr_a`` is pointing at the memory location of variable ``a``, so the line of code ``*ptr_a = 40;`` overwrites the ``10`` already stored in ``a`` with ``40``. Essentially what is happening here, is the compiler is issuing a load to get the address stored in ``ptr_a``, it is then immediately using the address returned to perform a store operation with the literal value ``40``. 

A similar story can be said for dereferencing a pointer for reading purposes. In the line ``Serial.print(*ptr_a);`` the ``*ptr_a`` is telling the compiler, load the value stored in ``ptr_a`` then immediately use that returned value to perform another load, getting the contents of the ``a`` variable. 

This dual use of the ``*`` operator in the C syntax can be a source of confusion for people new to pointers, essentially:
* ``*`` is used to:
  * __declare__ a pointer type, e.g. ``int * ptr_a;`` 
  * __dereference__ a pointer, either for reading purposes ``Serial.print(*ptr_a)``, or for writing ``*ptr_a = 40``.
* ``&`` is used to get the address of a variable or object

### Double pointer
We can also go further and make pointers to pointers.

```C
int a; // a variable
int *ptr_a; // a variable that is a pointer
int **ptr_ptr_a;
int ***ptr_ptr_ptr_a;

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");


        a = 10;
        ptr_a = &a; // ptr is pointing to the address of a
        ptr_ptr_a = &ptr_a; // a pointer to a pointer of a
        ptr_ptr_ptr_a = &ptr_ptr_a;

        Serial.print("Variable a has value:");
        Serial.println(a);

        Serial.print("and lives at address 0x");
        Serial.println((unsigned int)ptr_a, HEX);

        Serial.print("Double dereference: ");
        Serial.println(***ptr_ptr_ptr_a);
}

```

In the example above, we have ``ptr_ptr_ptr_a``, which is pointing at the address of ``ptr_ptr_a``, which is pointing at the address of ``ptr_a``, which is pointing at the address of ``a``.
Again all the derefrencing operations above work in these cases also. However, notice the number of ``*``s used increases with the levels of indirection, in both derefrencing the pointer and in declaring it.

Compiling and running this gives the following serial console output:

```
Variable a has value:10
and lives at address 0x3FFC00D0
Double dereference: 10
```

### Pointer types

_Why do we have pointer types? Why dont we just have one address type?_

In the previous example we declared an integer type pointer, ``int * ptr_a;``. But why did we do this, all of our addresses in our system are 32-bit. Why didn't we just declare some sort of 32-bit value to store the address location.

__The reason is because pointers take into account the size of the data item that they are pointing to.__ Different types in the system consume a different number of bytes, consider the following code:

```C
void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");
  
        Serial.print("datatype      size\n");
        Serial.print("int           "); Serial.println(sizeof(int));
        Serial.print("float         "); Serial.println(sizeof(float));
        Serial.print("char          "); Serial.println(sizeof(char));
        Serial.print("uint16_t      "); Serial.println(sizeof(uint16_t));
        Serial.print("bool          "); Serial.println(sizeof(bool));
}
```
This is printing out the size of different datatypes in number of bytes. The ``sizeof()`` function is used to return in bytes the type of the input argument for that system. Running this gives us the following serial console output:

```
 datatype      size
 int           4
 float         4
 char          1
 uint16_t      2
 bool          1
```

What we can see from the output that different datatypes consume different amounts of bytes in memory. In most modern systems memory is byte-addressable, meaning that we can address one byte of memory at a time in hardware. For this reason our smallest datatypes, ``bool`` and ``char`` consume one byte of data. The ``bool`` type is quite wasteful, it can be represented by a single bit but because memory hardware is byte-addressable, requires a whole byte to be stored.  

We can perform arithmetic on pointers, and the compiler will take into account the size of the item when we are manipulating the addresses that the pointer points at.

For instance, consider the following arduino code:

```C 
int a;
int* ptr_a;

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");
        
        a = 10;
        ptr_a = &a; // ptr is pointing to the address of a
        
        Serial.print("Variable a has value:");
        Serial.println(a);
        Serial.print("and lives at address 0x");       
        Serial.println((unsigned int)ptr_a, HEX);
        
        ptr_a = ptr_a + 1;
        Serial.print("ptr_a now points to address 0x");
        Serial.println((unsigned int)ptr_a, HEX);
}
```
Which produces the following serial output:

```
Variable a has value:10
and lives at address 0x3FFC00C8
ptr_a now points to address 0x3FFC00CC
```

In this example, the pointer ``ptr_a`` is of type ``int *``, meaning it is pointing to an integer which is 4 bytes in size. When we perform arithmetic on the pointer with the line ``ptr_a = ptr_a + 1``, what we can see if that the address stored in ``ptr_a`` has changed from ``0x3FFC00C8`` to ``0x3FFC00CC``. Now looking at ``0x3FFC00CC - 0x3FFC00C8 = 4`` the size of an integer.

Next consider the following code:

```C
bool a; // a variable
bool *ptr_a; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        a = true;
        ptr_a = &a; // ptr is pointing to the address of a

        Serial.print("Variable a has value:");
        Serial.println(a);
        Serial.print("and lives at address 0x");
        Serial.println((unsigned int)ptr_a, HEX);      

        ptr_a = ptr_a + 1;
        Serial.print("ptr_a now points to address 0x");
        Serial.println((unsigned int)ptr_a, HEX); 
}
```
In this example the variable ``a`` and ``ptr_a`` have been changed from an ``int`` and ``int *`` type to a ``bool`` and ``bool *`` type. When we execute the code we get the following serial console output:

```
Variable a has value:1
and lives at address 0x3FFC00C8
ptr_a now points to address 0x3FFC00C9
```
What we can see now is that the address only increased by one byte, ``0x3FFC00C9 - 0x3FFC00C8 = 1`` the size of a ``bool`` variable on our system. 

Where this is really useful is with arrays.

### Arrays in C

An array in C essentially blocks out a chunk of contiguous (right next to each other) memory that can be used. Take the example below:

```C
int a[8] = {42, 43, 44, 45, 46, 47, 48, 49}; // an array 

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        for(int i=0; i<8; i++) {
            Serial.print("[0x");
            Serial.print((unsigned int)&a[i],HEX);
            Serial.print("] = ");
            Serial.println(a[i]);
        }

}
```

This declares an integer array ``int a[8]`` and initialises it with the values ``42,43,44,45,46,47,48,49``. Using a for loop and our ``&`` operator discussed earlier we can print out the memory address for each element of our array and the value stored at that location. Running the code produces the following serial console output:

```
[0x3FFBEBE8] = 42
[0x3FFBEBEC] = 43
[0x3FFBEBF0] = 44
[0x3FFBEBF4] = 45
[0x3FFBEBF8] = 46
[0x3FFBEBFC] = 47
[0x3FFBEC00] = 48
[0x3FFBEC04] = 49
```

What we can notice is that for each element of our array the address is increasing by the type of the array, in this case an ``int`` which is 4 bytes.

In the previous section we looked at how pointer arithmetic took into consideration the type of the pointer. If we add 1 to an ``int *`` it will increase the address by 4 Bytes. This means that we can use it to iterate over the array. For example, we can change the code above with:

```C
int a[8] = {42, 43, 44, 45, 46, 47, 48, 49}; // an array
int *ptr_a; // a variable that is a pointer

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        ptr_a = &a[0]; // points to the first element of the array 
        for(int i=0; i<8; i++) {
                Serial.print("[0x");
                Serial.print((unsigned int)(ptr_a), HEX);
                Serial.print("] = ");
                Serial.println(*ptr_a);
                ptr_a = ptr_a + 1;
        }
}
```

Where we have replaced reading the array ``a[i]`` instead with pointer arithmetic on ``ptr_a``, and we would get exactly the same output on the serial console. 

Arrays and pointers are deeply linked, in fact the C standard defines the array syntax as:

```
             a[b] == *(a + b)
```

Which means that ``a[b]`` is equal to the pointer ``a`` where we have added ``b`` and dereferenced it all ``*(a + b)``. Now, since addition is commutative, we can actually do this monstrosity and it will compile and work:

```C
int a[8] = {42, 43, 44, 45, 46, 47, 48, 49}; // an array

void setup() {
        Serial.begin(115200);
        Serial.print("\n\n");

        // a[b] == *(a + b)
        Serial.println(0[a]); 
        Serial.println(1[a]);
        Serial.println(2[a]);
        Serial.println(3[a]); // 3[a] == *(3 + a) == *(a + 3)
        Serial.println(4[a]);
        Serial.println(5[a]);

}
```

Which will run and produce the serial output:

```
42
43
44
45
46
47
```

``5[a]`` to me looks like it shouldn't compile, but hey... 

### Pointer casting

Another useful feature of pointers is that we can cast them into different types to slice up memory in different ways. I wont go into the details of it here, but there is an example in this repo [[here](Arduino/Casting/Casting.ino)] that demonstrates this.

## Memory-mapped hardware

Remember at the start I said that we would be using memory addresses to interact with hardware. Well that is why pointers are so handy, we can assign them memory addresses, and then derefrence them to read or write values. If a block of hardware in our system has a memory-mapped address then we can construct a pointer for it and write directly into our hardware units and manipulate them.

Let's look at a simple hardware block, the true random number generator on the ESP32 chip, which is described on page 594 of the [[Technical Reference Manual (TRM)](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)]. 

![](imgs/random_number_generator.png)

This is a true random number generator that uses samples noise from the ADCs periodically to increase entropy. This unit is memory-mapped in the system and has a very simple interface:

![](imgs/random_number_generator_register.png)

The register for the hardware random number generator is mapped to address ``0x3FF75144`` and it is a ``RO` (read only) register, any writes to it are just thrown away. Let's write some simple code to periodically read from this register, and then we can use the Arduino serial plotter to plot it. 

```C
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
```
In this code we are defining a pointer ``unsigned int *hardware_rng;``, which we are assigning a value ``(unsigned int*)(0x3FF75144)``, the address of our hardware random number generator we got from the datasheet. The ``(unsigned int*)`` is called a cast, and we are using it to make the literal ``0x3FF75144`` the same type as our pointer. 

In the ``loop()`` of our arduino sketch we can then read our true hardware number generator by simply derefrencing the pointer we just defined. We can then pass that over the serial channel where we can plot it with the Arduino plotter (Tools->serial plotter), to get the following output:

![](imgs/random.png)

And that's it. We have used a pointer to interact with memory mapped hardware.




