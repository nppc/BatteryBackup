#define DEBUG
#define WITHBEC

// ***** USER SETTINGS ******
// Analog pins for Battery and BEC sensing
#define PIN_BATT_A1      A0    // Analog input from voltage divider from main Battery  
#define PIN_BATT_A2      A1    // Analog input from voltage divider from secondary Battery  
#define PIN_BEC_A1       A2    // Analog input from voltage divider from Receiver (Arduino) voltage  
#define PIN_BEC_A2       A3    // Analog input from voltage divider from Receiver (Arduino) voltage secondary

// Digital pins for Battery MOSFETS
#define PIN_BATT_M1		6	// Arduino Nano have LED connected to this pin. Useful for debugging
#define PIN_BATT_M2		3	//

// Voltage dividers for Battery and BEC voltage sensing
// Battery 1
#define BATT_A1_R1		98.5 //  22K (value in Kohms * 10) resistor to Positive lead of battery 
#define BATT_A1_R2		32.6  //  10K (value in Kohms * 10) resistor to the GND 
// Battery 2
#define BATT_A2_R1		98.5 //  22K (value in Kohms * 10) resistor to Positive lead of battery 
#define BATT_A2_R2		32.6  //  10K (value in Kohms * 10) resistor to the GND
// BEC 1
#define BEC_A1_R1	    118 //  22K (value in Kohms * 10) resistor to Positive lead to VDD 
#define BEC_A1_R2	  	67  //  3.3K (value in Kohms * 10) resistor to the GND
// BEC 2
#define BEC_A2_R1       118 //  22K (value in Kohms * 10) resistor to Positive lead to VDD 
#define BEC_A2_R2       67  //  3.3K (value in Kohms * 10) resistor to the GND

// Internal voltage reference (5v or 1.1v)
#define VrefBEC			3.280 // value in V
#define VrefBATT		3.280 // value in V

// voltage offsets for fine tuning voltage readings (value will be added to the final voltage reading)
#define BATT1_V_OFFSET	0.030	// Offset for adjusting final voltage calculation
#define BATT2_V_OFFSET	0.090	// Offset for adjusting final voltage calculation
#define BEC1_V_OFFSET	-0.020	// Offset for adjusting final voltage calculation
#define BEC2_V_OFFSET	0.000	// Offset for adjusting final voltage calculation

// Threshold voltage for switching from the main battery to secondary
#define BATT_LOW_THRSLD_NORM 	4.5 // 9 volts is the threshold for switching from the main battery to backup after a delay
#define BATT_LOW_THRSLD_SHORT	3.5 // 8 volts is the threshold for switching from the main battery to backup imideatly
#define BATT_LOW_SAFE_TIME 	   2000 // time in ms to allow the main battery to be under the LOW threshold (if Full throttle is applied)
#define BEC_LOW_THRSLD_BAD      3.5 // 3 volts - is the voltage from BEC showing that something wrong with it. Working BEC should give out 5 volts.
#define BATT2_DETECT_THRSLD     2.0 // threshold used at initialization step to determine does backup battery is connected.

// ***** END OF USER SETTINGS ******

// **** Please, do not change remaining lines! ****
#define state_INIT       	0 // Initial state. detect, what batteries is connected and what BECs are working
#define state_FIRSTBATT     1 // First battery is connected.
#define state_SECONDBATT    2 // Second Battery is connected
#define state_TWOBATT    	3 // Both batteries is connected
#define state_FAILSAFE   	4 // In case of two batteries setup, failsafe state is detected
#define state_HWFAILED   	5 // One of the BECs is not working. It means we have BAD hardware.

#define TURN_MOSFET_1_ON  digitalWrite(PIN_BATT_M1, HIGH);
#define TURN_MOSFET_1_OFF digitalWrite(PIN_BATT_M1, LOW);
#define IS_MOSFET_1_ON   (digitalRead(PIN_BATT_M1) == HIGH)
#define IS_MOSFET_1_OFF   (digitalRead(PIN_BATT_M1) == LOW)

#define TURN_MOSFET_2_ON  digitalWrite(PIN_BATT_M2, LOW);
#define TURN_MOSFET_2_OFF digitalWrite(PIN_BATT_M2, HIGH);
#define IS_MOSFET_2_ON   (digitalRead(PIN_BATT_M2) == LOW)
#define IS_MOSFET_2_OFF   (digitalRead(PIN_BATT_M2) == HIGH)

#ifdef DEBUG 
	#include "HardwareSerial.h" 
#endif 

