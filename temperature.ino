#include <Wire.h>

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	Wire.begin(69);
	//Wire.setClock(1000000);
	Wire.onReceive(receiveEvent);

	Serial.println("Ready");
}

void loop() {}

void receiveEvent(int howMany) {
	digitalWrite(LED_BUILTIN, HIGH);

	while (Wire.available()) {  // loop through all but the last
		char c = Wire.read();   // receive byte as a character
		Serial.println(c);		// print the character
	}
	Wire.write("dc");
	digitalWrite(LED_BUILTIN, LOW);
}
