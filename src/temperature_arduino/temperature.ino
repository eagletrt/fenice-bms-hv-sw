#include <Wire.h>

#define I2C_CLOCK 351562
#define I2C_ADDR 69

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	Wire.begin(I2C_ADDR);
	Wire.setClock(I2C_CLOCK);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(request);

	Serial.println("Ready");
}

void loop() {}

void request() {
	digitalWrite(LED_BUILTIN, HIGH);

	Serial.println("req");

	uint8_t data[2] = {0x05, 0x5D};

	Wire.write((char *)data);
	digitalWrite(LED_BUILTIN, LOW);
}

void receiveEvent(int howMany) {
	digitalWrite(LED_BUILTIN, HIGH);

	while (Wire.available()) {  // loop through all but the last
		char c = Wire.read();   // receive byte as a character
		Serial.println(c);		// print the character
	}
	digitalWrite(LED_BUILTIN, LOW);
}
