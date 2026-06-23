#include <TM1637Display.h>
#include <Encoder.h>

#define CLK_PIN 11 // Cluck Pin
#define DIO_PIN 12 // Diode Pin

#define Daylight_Pin 10 // For daylight(Cold) control mosfet
#define Tungsten_Pin 9  // For Tungstenlight(Warm) control mosfet

// Rotery Encoder(For Brightness)
#define BRT_CLK 2
#define BRT_DT 4

// Rotery Encoder(For Control Tempeture)
#define CCT_CLK 3
#define CCT_DT 5

// Create Entity
TM1637Display display(CLK_PIN, DIO_PIN);
Encoder encBright(BRT_CLK, BRT_DT);
Encoder encCCT(CCT_CLK, CCT_DT);

long oldBrightPos = -999;
long oldCCCTPos = -999;

int brightness = 50; // Initial Brightness (0 ~ 100%)
int cct = 4000;      // Initial Tempeture (3000K ~ 6500)

unsigned long lastInteractionTime = 0;
bool isDisplayOn = ture;

void setup(){
	// Setting PinMode
	pinMode(Tungsten_pin, OUTPUT);
	pinMode(Daylight_pin, OUTPUT);
	
	// Initialize Brightness(0 ~ 7)
	display.setBrightness(4);
	display.showNumberDec(cct, false); // Display Initialize color tempeture when start
					
	updateLEDs(); 			    // Apply Initialize light status
}

void loop(){
	// Read Encoder value (4 step per bright)
	long newBrtPos = encBright.read() / 4;
	long newCCTPos = encCCT.read() / 4;

	bool changed = false;

	// When bright control  knob is move
	if (newBrtPos != oldBrightPos){
		int diff = newBrtPos - oldBrightPos;
		brightness = brightness + (diff * 5); // Change brightness 5% per 1 step
		brightness = constrain(brightness, 0, 100); // Set bright limit 0% to 100%
	
		display.showNumberDec(brightness, false); // Display brightness on the screen
		oldBrightPos = newBrtPos;
		changed = true;
	}

	if (changed){
		updateLEDs();
		lastInteractionTime = millis();
		if (!isDisplayOn){
			display.setBrightness(4);
			isDisplayOn = true;
		}
	}

	if (isDisplayOn && (millis() - lastInteractionTime > 10000)){
		display.clear();
		isDisplayOn = false;
	}
}

void updateLEDs(){
	// Calculate CCT Ratio
	float daylightRatio = (float)(cct - 3000) / (6500 - 3000);
	float tungstenRatio = 1.0  - daylightRatio;

	// Calculate final PWM (Scale : 0 ~ 255)
	float dayPWM = (int)(255.0 * (brightness / 100.0) * daylightRatio);
	float tungPWM = (int)(255.0 * (brightness / 100.0) * tungstenRatio);

	// AnalogOutput (Pass to MOSFET)
	analogWrite(Tungsten_Pin, tungPWM);
	analogWrite(Daylight_Pin, dayPWM);
}


