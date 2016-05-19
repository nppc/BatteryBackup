/*
In case of two batteries setup, BATT1 is the main battery, and BATT2 is the backup battery.

In case of one battery setup, BATT2 should be the main battery.

Analog reference. For Battery and BEC measurement we are using VCC as reference. 

*/

#include "BB_config.h"

byte batt1_ok = 0;	// Stattus for Battery 1. 0 - not detected; 1 - battery is OK
byte batt2_ok = 0;	// Stattus for Battery 2. 0 - not detected; 1 - battery is OK
byte bec1_ok = 0;	// Status for BEC 1. 0 - no voltage after BEC; 1 - BEC is OK
byte bec2_ok = 0;	// Status for BEC 2. 0 - no voltage after BEC; 1 - BEC is OK

// voltage dividers for batt voltage sense
const float BATT_V1_div = (float)BATT_A1_R2 / (float)(BATT_A1_R1 + BATT_A1_R2); // R1/R2 divider
const float BATT_V2_div = (float)BATT_A2_R2 / (float)(BATT_A2_R1 + BATT_A2_R2); // R1/R2 divider
// Voltage after Battery
float batt1_voltage = 0;
float batt2_voltage = 0;

// voltage dividers for BEC voltage sense
#ifdef WITHBEC
	const float BEC_V1_div = (float)BEC_A1_R2 / (float)(BEC_A1_R1 + BEC_A1_R2); // R1/R2 divider
	const float BEC_V2_div = (float)BEC_A2_R2 / (float)(BEC_A2_R1 + BEC_A2_R2); // R1/R2 divider
	// Voltage after BEC
	float bec1_voltage = 0;
	float bec2_voltage = 0;
#endif

#define filterSamples   13              // filterSamples should  be an odd number, no smaller than 3
int batt1_voltage_SmoothArray [filterSamples];   // array for holding raw sensor values for Pressure sensor 
int batt2_voltage_SmoothArray [filterSamples];   // array for holding raw sensor values for Pressure sensor 
int bec1_voltage_SmoothArray [filterSamples];   // array for holding raw sensor values for Pressure sensor 
int bec2_voltage_SmoothArray [filterSamples];   // array for holding raw sensor values for Pressure sensor 
unsigned int analog_read = 0;
//byte battery_in_use = 1;// 1 - Main battery is connected; 2 - secondary battery is connected
byte state = state_INIT;

unsigned long time_before_BATT_switch = 0;	// 0 - counter is not started
unsigned long time_INIT = 0; // Time while waiting for all voltages to settle
unsigned long time_FAILSAFE; // Time for waiting for BEC2 get voltage and ADC readings are actual

#ifdef DEBUG 
	unsigned long lastMillisDEBUG;
#endif 


void setup()
{
#ifdef DEBUG 
	Serial.begin(57600); 
	delay(5000);
	Serial.print(millis()); Serial.println(F(" *** OXS Battery Backup V2.0 ***"));	// welcome screen
#endif 
	// Need to turn on mosfets to check BECs health
	TURN_MOSFET_1_ON;	// first set state of the pin (enable or disable pullup resistor)
	TURN_MOSFET_2_ON; // first set state of the pin (enable or disable pullup resistor)
	pinMode(PIN_BATT_M1, OUTPUT);	// and then set pin direction
	pinMode(PIN_BATT_M2, OUTPUT);	// and then set pin direction
	#ifdef BACKUPLED
		pinMode(PIN_LED_M2, OUTPUT);	// and then set pin direction for LED if used
	#endif
	time_INIT = millis(); // Start a timer for counting 500ms after BECs on, to get caps charged :)
	// discard first readings to let Analog reference voltage to stabilize and to fill smooth array
	for(byte i=0;i<30;i++){
		analog_read = digitalSmooth(analogRead(PIN_BATT_A1), batt1_voltage_SmoothArray);
		analog_read = digitalSmooth(analogRead(PIN_BATT_A2), batt2_voltage_SmoothArray);
		analog_read = digitalSmooth(analogRead(PIN_BEC_A1), bec1_voltage_SmoothArray);
		analog_read = digitalSmooth(analogRead(PIN_BEC_A2), bec2_voltage_SmoothArray);
		delay(1);
	}	
	
#ifdef DEBUG 
	lastMillisDEBUG = millis();
#endif 
}

void loop()
{
	// read analog inputs (Battery voltages)
	//analogReference(DEFAULT);	// 5V. 
	analog_read = digitalSmooth(analogRead(PIN_BATT_A1), batt1_voltage_SmoothArray);
	//analog_read = getADCValueFromPIN(PIN_BATT_A1);
	//analog_read = analogRead(PIN_BATT_A1);
	batt1_voltage = (((float)analog_read/1024.0) * VrefBATT) / BATT_V1_div + BATT1_V_OFFSET; 
	analog_read = digitalSmooth(analogRead(PIN_BATT_A2), batt2_voltage_SmoothArray);
	//analog_read = getADCValueFromPIN(PIN_BATT_A2);
	//analog_read = analogRead(PIN_BATT_A2);
	batt2_voltage = (((float)analog_read/1024.0) * VrefBATT) / BATT_V2_div + BATT2_V_OFFSET; 
  	// check voltages and update variables accordingly 
  	// check Batteries
  	if(batt1_voltage > BATT_LOW_THRSLD_SHORT) {batt1_ok = 1;} else  {batt1_ok = 0;}
  	if(batt2_voltage > BATT2_DETECT_THRSLD) {batt2_ok = 1;} else  {batt2_ok = 0;}

	#ifdef WITHBEC
		//analogReference(INTERNAL);	// 1.1V. 
		//for(byte i=0;i<20;i++){analog_read = analogRead(PIN_BEC_A1);}	// discard first readings to let Analog reference voltage stabilize
		analog_read = digitalSmooth(analogRead(PIN_BEC_A1), bec1_voltage_SmoothArray);
		//analog_read = analogRead(PIN_BEC_A1);
		bec1_voltage = (((float)analog_read/1024.0) * VrefBEC) / BEC_V1_div + BEC1_V_OFFSET; 
		analog_read = digitalSmooth(analogRead(PIN_BEC_A2), bec2_voltage_SmoothArray);
		//analog_read = analogRead(PIN_BEC_A2);
		bec2_voltage = (((float)analog_read/1024.0) * VrefBEC) / BEC_V2_div + BEC2_V_OFFSET; 
	  	// check voltages and update variables accordingly 
	  	// Check BECs
	  	if(bec1_voltage > BEC_LOW_THRSLD_BAD) {bec1_ok = 1;} else  {bec1_ok = 0;}
	  	if(bec2_voltage > BEC_LOW_THRSLD_BAD) {bec2_ok = 1;} else  {bec2_ok = 0;}
	#else
		bec1_ok = 1;	// No BEC setup
		bec2_ok = 1;	// No BEC setup
	#endif

// Proccess the state of device
switch (state) {
	case state_INIT:	// Initialization, detect what batteries are connected
		// Delay for 500ms after mosfets ON before continue... but not interrupt main loop
		if (time_INIT + 500 < millis()) {
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" state_INIT"));
			#endif
		  	// check what configuration we have and go to the next stage
		  	if((batt1_ok + bec1_ok == 2) && (batt2_ok + bec2_ok == 0)){
		  		state = state_FIRSTBATT;
	  			TURN_MOSFET_2_OFF; // turn off unused BEC
				#ifdef DEBUG 
					Serial.print(millis()); Serial.print(F(" First Battery is detected...")); Serial.println(batt1_voltage);
				#endif
		  	} else if((batt2_ok + bec2_ok == 2) && (batt1_ok + bec1_ok == 0)){
		  		state = state_SECONDBATT;
	  			TURN_MOSFET_1_OFF; // turn off unused BEC
				#ifdef DEBUG 
					Serial.print(millis()); Serial.print(F(" Second Battery is detected...")); Serial.println(batt2_voltage);
				#endif
		  	} else if(batt1_ok + batt2_ok + bec1_ok + bec2_ok == 4) {
		  		state = state_TWOBATT;
				#ifdef DEBUG 
					Serial.print(millis()); Serial.print(F(" TWO Battery setup is detected..."));
					Serial.print(" Batt 1: "); Serial.print(batt1_voltage); 
					Serial.print(", Batt 2: "); Serial.print(batt2_voltage); 
					#ifdef WITHBEC
						Serial.print(", BEC 1: "); Serial.print(bec1_voltage); 
						Serial.print(", BEC 2: "); Serial.print(bec2_voltage); 
					#endif
					Serial.println();
				#endif
		  	}
		  	else {
		  		state = state_HWFAILED; // One of the BECs is not giving power out
				#ifdef DEBUG 
					Serial.print(millis()); Serial.println(F(" Failure is detected..."));
				#endif
		  	}	
		}
	  break;
	case state_FIRSTBATT:	// Only the first battery is connected. This is not allowed configuration, so, we need to wait for second battery connected.
		// wait in this state until second battery connected. Remember, taht BEC2 will not have any power...
		TURN_MOSFET_1_OFF; // in this scenario mosfet should be OFF
		if(batt2_ok == 1){state = state_TWOBATT;} // Backup battery is connected now
		//if(batt1_ok == 1 && bec1_ok == 0){state = state_HWFAILED;} // BEC1 is not giving power out
		break;
	case state_SECONDBATT:	// Second battery is connected. This is one battery configuration. If later will be connected first battery, we will switch to state with two batteries
		// Should we change a state?
		if(batt1_ok == 1){state = state_TWOBATT;}
		// connect second battery to RC
		if(IS_MOSFET_2_OFF) {
			TURN_MOSFET_2_ON; // Turn on second Mosfet
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" ONE Battery setup. Batt2 is connected."));
			#endif
		}	
		//if(batt2_ok == 1 && bec2_ok == 0){state = state_HWFAILED;} // BEC2 is not giving power out
		// in this setup we do not have a backup battery, so stay in this mode until first battery is connected or forever... :)
		break;
	case state_TWOBATT:	// First and Second battery is connected.
		// This is the two battery setup. Now first battery is the main battery, and second is backup.
		//First of all lets connect first battery and disconnect second if it is not already done...
		if(IS_MOSFET_1_OFF) {
			TURN_MOSFET_1_ON; // Turn on first Mosfet
			delay(10);	//delay 10ms just to make sure that everything is connected
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" TWO Battery setup. First Battery is connected."));
			#endif
		}	
		if(IS_MOSFET_2_ON) {
			TURN_MOSFET_2_OFF; // Turn off second Mosfet
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" TWO Battery setup. Second Battery is disconnected as it is not needed."));
			#endif
		}	
		// now check for low voltage thresholds
		// *************** Check is Main Battery is shorted ***************
		if(batt1_voltage <= BATT_LOW_THRSLD_SHORT) {	// Main battery is shorted, switch imideatly
			state = state_FAILSAFE1;	// change state to failsafe to connect to backup battery
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" TWO Battery setup. Main battery is shorted, switch imideatly."));
			#endif
		}
		// *************** Check is Main Battery is LOW ****************
		if(batt1_voltage <= BATT_LOW_THRSLD_NORM) {	// Main battery is LOW, switch to secondary batt but after a delay
			if(time_before_BATT_switch == 0) {
				time_before_BATT_switch = millis() + BATT_LOW_SAFE_TIME; // start timer countdown
				#ifdef DEBUG 
					Serial.print(millis()); Serial.println(F(" TWO Battery setup. Main battery is LOW, start delay counter..."));
				#endif
			} else {
				if(millis() > time_before_BATT_switch) {
					state = state_FAILSAFE1;	// change state to failsafe to connect to backup battery
					#ifdef DEBUG 
						Serial.print(millis()); Serial.println(F(" TWO Battery setup. Main battery is LOW, switch after delay."));
					#endif
				}
			}
		} else {
			time_before_BATT_switch = 0;	// BATT1 is back normal within timeout
		}
		if(bec1_ok == 0){
			state = state_FAILSAFE1;
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" TWO Battery setup. Main battery BEC is dead."));
			#endif
			
		} // First BEC can fail. In this case we have backup battery
		//if(bec2_ok == 0){state = state_HWFAILED;} // Second BEC should not fail. This is BAD!
		break;
	case state_FAILSAFE1:	// Something went wrong with first battery (shorted or runs out of juice)
		// switch to the backup battery
		if(IS_MOSFET_2_OFF) {
			TURN_MOSFET_2_ON; // Turn on second Mosfet
			delay(10);	//delay 10ms just to make sure that everything is connected
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" TWO Battery setup. FAILSAFE1. Trying to switch to the second Battery..."));
			#endif
		}	
		// Before switch off Batt 1 lets check is Backup Battery is operational (both battery and BEC)
		// mosfet is already 10ms ON, so there is should be some voltage, but lets wait for some time more because of slow ADC and smoothing...
		state = state_FAILSAFE2;
		// start the timer for waiting for voltage to stabilize on BEC2
		time_FAILSAFE = millis();
		//break;
	case state_FAILSAFE2:	// Second stage of failsafe, switching Batt1 off if Batt2 is OK.
		// check the timer
		if (time_FAILSAFE+100 < millis()) {
			// now check, can we switch off Batt1...
			if (batt2_ok + bec2_ok == 2) {
				if(IS_MOSFET_1_ON) {
					TURN_MOSFET_1_OFF; // Turn off first Mosfet
					#ifdef DEBUG 
						Serial.print(millis()); Serial.println(F(" TWO Battery setup. FAILSAFE2. First Battery is disconnected."));
					#endif
				}	
			} else {
				Serial.print(millis()); Serial.println(F(" TWO Battery setup. FAILSAFE2. Backup battery is dead. Can't switch to it."));
				TURN_MOSFET_2_OFF; // stay on the first battery
				// stay forewer here
				state = state_DONOTHING; // TODO later add to the config
			}
		}
		break;	
	case state_HWFAILED:	// Worsest case - our hardware is failed.
		// TODO: should we blink with LEDs or what we should do?
			#ifdef DEBUG 
				Serial.print(millis()); Serial.println(F(" HARDWARE FAILED. CHECK YOUR BECs!"));	// dead screen
				//delay(500);
				state = state_INIT;	// try to detect setup again...
				TURN_MOSFET_1_ON;	// first set state of the pin (enable or disable pullup resistor)
				TURN_MOSFET_2_ON; // first set state of the pin (enable or disable pullup resistor)
				time_INIT = millis(); //Initialize the time counter for initialization step.
			#endif
		break;
	case state_DONOTHING:
		break;
}


#ifdef DEBUG 
	if (millis() > lastMillisDEBUG + 1000) { 
        lastMillisDEBUG = millis() ;  
        Serial.print(millis());
		Serial.print(" Batt 1: "); Serial.print(batt1_voltage); 
		Serial.print("   Batt 2: "); Serial.print(batt2_voltage); 
		#ifdef WITHBEC
			Serial.print("   BEC 1: "); Serial.print(bec1_voltage); 
			Serial.print("   BEC 2: "); Serial.print(bec2_voltage); 
		#endif
		Serial.println();
	}   
#endif 

}

/*
unsigned int getADCValueFromPIN(byte aPin) {
	unsigned int areadscount = 0;
	for(byte i=0;i<64;i++){areadscount += analogRead(aPin);}
	areadscount = areadscount / 64;
	return areadscount;
	
}
*/

// smooth algorytm for ADC reading
int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1); 
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
  }

  return total / k;    // divide by number of samples
}
