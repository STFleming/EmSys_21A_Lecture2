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

