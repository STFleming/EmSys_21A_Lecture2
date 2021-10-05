TINYPICO=/dev/ttyUSB0
ARDUINO_CLI=/home/stf0x2a/teaching/EmSys/Labs/EmSys_labSetup/arduino-cli/arduino-cli

serial: Serial/Serial.ino
	$(ARDUINO_CLI) compile --fqbn esp32:esp32:tinypico Serial
	$(ARDUINO_CLI) upload -p $(TINYPICO) --fqbn esp32:esp32:tinypico Serial

basicpointer: BasicPointer/BasicPointer.ino
	$(ARDUINO_CLI) compile --fqbn esp32:esp32:tinypico BasicPointer 
	$(ARDUINO_CLI) upload -p $(TINYPICO) --fqbn esp32:esp32:tinypico BasicPointer 
