#define DEBUG // uncomment for sending debug info to the hardware serial
#define WITHBEC // uncomment this, if you have BECs after the batteries
#define BACKUPLED // uncomment this, if you like to see Mosfet2 status (ON/OFF) by LED connected to another pin (pin defined later).


// ***** USER SETTINGS ******
// Analog pins for Battery and BEC sensing
#define PIN_BATT_A1      A3    // Analog input from voltage divider from main Battery  
#define PIN_BATT_A2      A0    // Analog input from voltage divider from secondary Battery  
#define PIN_BEC_A1       A2    // Analog input from voltage divider from Receiver (Arduino) voltage  
#define PIN_BEC_A2       A1    // Analog input from voltage divider from Receiver (Arduino) voltage secondary

// Digital pins for Battery MOSFETS
#define PIN_BATT_M1		3	// Mosfet are controlled ON=1, OFF=0
#define PIN_BATT_M2		6	// Mosfet are controlled in inverse eg ON=0, OFF=1
#ifdef BACKUPLED
	#define PIN_LED_M2	13	// Mosfet2 are works in invert. So, here is parrallel pin for LED to be used by human eye :)
#endif

// Voltage dividers for Battery and BEC voltage sensing
// Battery 1
#define BATT_A1_R1		3936 //  22K (value in Kohms * 10) resistor to Positive lead of battery 
#define BATT_A1_R2		1187 //  10K (value in Kohms * 10) resistor to the GND 
// Battery 2
#define BATT_A2_R1		3936 //  22K (value in Kohms * 10) resistor to Positive lead of battery 
#define BATT_A2_R2		1187 //  10K (value in Kohms * 10) resistor to the GND
// BEC 1
#define BEC_A1_R1	    118 //  22K (value in Kohms * 10) resistor to Positive lead to VDD 
#define BEC_A1_R2	  	67  //  3.3K (value in Kohms * 10) resistor to the GND
// BEC 2
#define BEC_A2_R1       118 //  22K (value in Kohms * 10) resistor to Positive lead to VDD 
#define BEC_A2_R2       67  //  3.3K (value in Kohms * 10) resistor to the GND

// Internal voltage reference (5v, 3.3v or 1.1v)
#define VrefBEC			3.291 // value in V
#define VrefBATT		3.291 // value in V

// voltage offsets for fine tuning voltage readings (value will be added to the final voltage reading)
#define BATT1_V_OFFSET	0.000	// Offset for adjusting final voltage calculation
#define BATT2_V_OFFSET	0.000	// Offset for adjusting final voltage calculation
#define BEC1_V_OFFSET	-0.040	// Offset for adjusting final voltage calculation
#define BEC2_V_OFFSET	0.020	// Offset for adjusting final voltage calculation

// Threshold voltage for switching from the main battery to secondary
#define BATT_LOW_THRSLD_NORM 	6.8 // 9 volts is the threshold for switching from the main battery to backup after a delay
#define BATT_LOW_THRSLD_SHORT	6.0 // 8 volts is the threshold for switching from the main battery to backup imideatly
#define BATT_LOW_SAFE_TIME 	   2000 // time in ms to allow the main battery to be under the LOW threshold (if Full throttle is applied)
#define BEC_LOW_THRSLD_BAD      3.5 // 3 volts - is the voltage from BEC showing that something wrong with it. Working BEC should give out 5 volts.
#define BATT2_DETECT_THRSLD     1.0 // threshold used at initialization step to determine does backup battery is connected.

// ***** END OF USER SETTINGS ******

// **** Please, do not change remaining lines! ****
#define state_INIT       	0 // Initial state. detect, what batteries is connected and what BECs are working
#define state_FIRSTBATT     1 // First battery is connected.
#define state_SECONDBATT    2 // Second Battery is connected
#define state_TWOBATT    	3 // Both batteries is connected
#define state_FAILSAFE1   	4 // In case of two batteries setup, failsafe state is detected
#define state_FAILSAFE2   	5 // In case of two batteries setup, failsafe state is detected
#define state_HWFAILED   	6 // One of the BECs is not working. It means we have BAD hardware.
#define state_DONOTHING   	99 // Do nothing state. It is useful to switch here if nwe do not need to do anything anymore. No chance to go out from this state.

#define TURN_MOSFET_1_ON  digitalWrite(PIN_BATT_M1, HIGH);
#define TURN_MOSFET_1_OFF digitalWrite(PIN_BATT_M1, LOW);
#define IS_MOSFET_1_ON   (digitalRead(PIN_BATT_M1) == HIGH)
#define IS_MOSFET_1_OFF   (digitalRead(PIN_BATT_M1) == LOW)

#ifdef BACKUPLED
	#define TURN_MOSFET_2_ON  {digitalWrite(PIN_BATT_M2, LOW);digitalWrite(PIN_LED_M2, HIGH);}
	#define TURN_MOSFET_2_OFF {digitalWrite(PIN_BATT_M2, HIGH);digitalWrite(PIN_LED_M2, LOW);}
#else
	#define TURN_MOSFET_2_ON  digitalWrite(PIN_BATT_M2, LOW);
	#define TURN_MOSFET_2_OFF digitalWrite(PIN_BATT_M2, HIGH);
#endif
#define IS_MOSFET_2_ON   (digitalRead(PIN_BATT_M2) == LOW)
#define IS_MOSFET_2_OFF   (digitalRead(PIN_BATT_M2) == HIGH)

#ifdef DEBUG 
	#include "HardwareSerial.h" 
#endif 

