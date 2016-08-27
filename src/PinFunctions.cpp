#include <BowlerCom.h>
#ifdef ARDUINO_ARCH_ARC32
#include "CurieIMU.h"
#endif
int ax, ay, az;         // accelerometer values
int gx, gy, gz;         // gyrometer values
		/**
		 * Private functions
		 */
uint8_t getInterpolatedPin(uint8_t pin);
void SetServoPos(uint8_t pin, uint16_t val, float time);
void SetServoPosDataTable(uint8_t pin, uint16_t val, float time);

/**
 * Data structures
 */
DATA_STRUCT * DyioPinFunctionData;
Servo myservo[MAX_SERVOS];  // create servo object to control a servo
INTERPOLATE_DATA velocity[MAX_SERVOS];  // servo position interpolation

boolean startupFlag = false;
int32_t GetConfigurationDataTable(uint8_t pin) {
	return EEReadValue(pin);
}


boolean setMode(uint8_t pin, uint8_t mode) {
	if (pinHasFunction(pin, mode)) {
		println_I("Setting Pin ");
		p_int_I(pin);
		print_I(" to New Mode ");
		printMode(pin, mode, INFO_PRINT);
		if (pin < 2) {
			println_I("Bailing because this is the communications channel");
			return true;
		}
		if (GetChannelMode(pin) == mode && startupFlag) {
			println_I("Bailing because this mode is already set");
			return true;
		}
		if (GetChannelMode(pin) == IS_SERVO) {
			println_I("Detaching servo");
			myservo[PIN_TO_SERVO(pin)].detach();
		}

		_EEWriteMode(pin, mode);
		//getBcsIoDataTable(pin)->PIN.currentChannelMode = mode;

		switch (GetChannelMode(pin)) {
		case IS_DI:
			pinMode(PIN_TO_DIGITAL(pin), INPUT);
			digitalWrite(pin, HIGH);       // turn on pullup resistors
			break;
		case IS_DO:
			pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
			break;
		case IS_ANALOG_IN:
			// arduino analogs are not changable
			break;
		case IS_SERVO:
			myservo[PIN_TO_SERVO(pin)].attach(PIN_TO_SERVO(pin));
			break;
		case IS_DEBUG_TX:
//			pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
//			pinMode(PIN_TO_DIGITAL(pin - 1), INPUT);
			_EEWriteMode(pin - 1, IS_DEBUG_RX);
			break;
		case IS_DEBUG_RX:
//			pinMode(PIN_TO_DIGITAL(pin + 1), OUTPUT);
//			pinMode(PIN_TO_DIGITAL(pin), INPUT);
			_EEWriteMode(pin + 1, IS_DEBUG_TX);
			break;
		}

		return true;
	}
	println_I("CAN NOT BE MODE Pin ");
	p_int_I(pin);
	print_I(" to New Mode ");
	printMode(pin, mode, INFO_PRINT);
	return false;
}

/**
 * Set Channel Values
 * This function takes a
 * @param pin pin index
 * @param numValues a number of values to be delt with
 * @param data an array of data values
 * @param ms the time for the transition to take
 *
 */
boolean SetChanelValueHW(uint8_t pin, uint8_t numValues, int32_t * data,
		float ms) {
	//uint8_t mode = GetChannelMode(pin);
	SetChanVal(pin, data[0], ms);
	return true;
}

/**
 * Set Channel Values
 * This function takes a pin index, a number of values to be delt with, and an array of data values
 * Data is stored into numValues and data
 */
boolean GetChanelValueHW(uint8_t pin, uint8_t * numValues, int32_t * data) {
	//uint8_t mode = GetChannelMode(pin);

	numValues[0] = 1;

	data[0] = GetChanVal(pin);

	return true;

}

/**
 * Set Channel Values
 * This function takes a
 * @param data an array of data values
 * @param ms the time for the transition to take
 *
 */
boolean SetAllChanelValueHW(int32_t * data, float ms) {
	int i;
	for (i = 0; i < GetNumberOfIOChannels(); i++) {
		SetChanelValueHW(i, 1, &data[i], ms);
	}
	return true;
}

/**
 * Set Channel Values
 * This function takes a pin index, a number of values to be delt with, and an array of data values
 * Data is stored into numValues and data
 */
boolean GetAllChanelValueHW(int32_t * data) {
	int i;
	uint8_t numValues;
	for (i = 0; i < GetNumberOfIOChannels(); i++) {
		GetChanelValueHW(i, &numValues, &data[i]);
	}
	return true;
}

/**
 * Configure Channel
 * @param pin the index of the channel to configure
 * @param numValues The number of values passed in to deal with
 * @param data the array of values to use in the configuration step
 */

boolean ConfigureChannelHW(uint8_t pin, uint8_t numValues, int32_t * data) {

	EEWriteValue(pin, data[0]);

	return true;
}

/**
 * Set Stream
 * This function takes a
 * @param pin pin index
 * @param numValues a number of values to be dealt with
 * @param data an array of data values
 *
 */
boolean SetStreamHW(uint8_t pin, uint8_t numValues, uint8_t * data) {

	return true;

}

/**
 * Get Stream
 * This function takes a pin index, a number of values to be dealt with, and an array of data values
 * Data is stored into numValues and data
 */
boolean GetStreamHW(uint8_t pin, uint8_t* numValues, uint8_t * data) {

	return true;
}

boolean SetChanVal(uint8_t pin, int32_t bval, float time) {
	if (pin < 2) {
		return true;
	}
//	print_W("\r\n");
//	p_int_E(pin);
//	print_W(" Set value of pin to ");
//	p_int_W(bval);
	switch (GetChannelMode(pin)) {

	case IS_DO:
		digitalWrite(PIN_TO_DIGITAL(pin), bval == 0 ? LOW : HIGH);
		break;
	case IS_ANALOG_IN:
		// arduino analogs are not changable
	case IS_SERVO:
		SetServoPos(pin, bval, time);
		break;
	case IS_DEBUG_TX:
	case IS_DEBUG_RX:
		// do not change the mode of these pins
		break;
	}

	return true;
}
int32_t GetChanVal(uint8_t pin) {
	if (pin < 2) {
		return 0;
	}
	switch (GetChannelMode(pin)) {
	case IS_DI:
		return digitalRead(PIN_TO_DIGITAL(pin));
	case IS_ANALOG_IN:
#ifdef ARDUINO_ARCH_ARC32
		if (PIN_TO_ANALOG(pin) < 6)
			return analogRead(PIN_TO_ANALOG(pin));
		switch (PIN_TO_ANALOG(pin) - 6) {
		case 0:
			return ax;
		case 1:
			return ay;
		case 2:
			return az;
		case 3:
			return gx;
		case 4:
			return gy;
		case 5:
			return gz;
		}
#else
		return analogRead(PIN_TO_ANALOG(pin));
#endif
		break;
	case IS_SERVO:
		return GetServoPos(pin);
	case IS_DEBUG_TX:
	case IS_DEBUG_RX:
		// do not change the mode of these pins
		break;
	}

	return getDataTableCurrentValue(pin);
}
void InitPinFunction(DATA_STRUCT * functionData) {
	println_W("Initializing DYIO Functions");
	DyioPinFunctionData = functionData;
	int i;
	for (i = 0; i < TOTAL_PINS; i++) {
		DyioPinFunctionData[i].FUNCTION.HAS_ANALOG_IN = IS_PIN_ANALOG(i);
		DyioPinFunctionData[i].FUNCTION.HAS_PWM = IS_PIN_PWM(i);
		DyioPinFunctionData[i].FUNCTION.HAS_UART_T = IS_PIN_DIGITAL(i)
				&& i % 2 != 0;  //odd pins are tx
		DyioPinFunctionData[i].FUNCTION.HAS_UART_R = IS_PIN_DIGITAL(i)
				&& i % 2 == 0;  // even pins are rx
		DyioPinFunctionData[i].FUNCTION.HAS_SPI_C = false;
		DyioPinFunctionData[i].FUNCTION.HAS_SPI_I = false;
		DyioPinFunctionData[i].FUNCTION.HAS_SPI_O = false;
		DyioPinFunctionData[i].FUNCTION.HAS_COUNTER_INPUT_I = false;
		DyioPinFunctionData[i].FUNCTION.HAS_COUNTER_OUTPUT_I = false;
		DyioPinFunctionData[i].FUNCTION.HAS_COUNTER_INPUT_D = false;
		DyioPinFunctionData[i].FUNCTION.HAS_COUNTER_OUTPUT_D = false;
		DyioPinFunctionData[i].FUNCTION.HAS_COUNTER_INPUT_H = false;
		DyioPinFunctionData[i].FUNCTION.HAS_COUNTER_OUTPUT_H = false;
		DyioPinFunctionData[i].FUNCTION.HAS_DC_MOTOR = false;
		DyioPinFunctionData[i].FUNCTION.HAS_PPM = false;
	}
	println_W("Initializing IO Functions");
	InitilizeBcsIo( TOTAL_PINS, DyioPinFunctionData, &SetChanelValueHW,
			&GetChanelValueHW, &SetAllChanelValueHW, &GetAllChanelValueHW,
			&ConfigureChannelHW, &SetStreamHW, &GetStreamHW);
	println_W("Initializing Setmode Functions");
	InitilizeBcsIoSetmode(&setMode);
	println_W("Initializing Advanced Async Functions");
	initAdvancedAsync();  // after the IO namespace is set up
	//Initialize pina after stack is initialized
	for (i = 0; i < TOTAL_PINS; i++) {
		println_I("Initializing ");
		p_int_I(i);
		print_I(" of ");
		p_int_I(TOTAL_PINS);
		//Get mode from EEPROm
		uint8_t mode = GetChannelMode(i);
		//Set up hardware in startup mode so it forces a hardware set
		setMode(i, mode);

		// Get value using hardware setting.
		int32_t currentValue;
		if (isOutputMode(mode) == true) {

			currentValue = GetConfigurationDataTable(i);
		} else {
			currentValue = GetChanVal(i);
		}
		DyioPinFunctionData[i].PIN.currentValue = currentValue;
		DyioPinFunctionData[i].PIN.asyncDataPreviousVal = currentValue;
	}
	
#ifdef ARDUINO_ARCH_ARC32
	// initialize device
	println_I("Initializing IMU device...");
	CurieIMU.begin();

	// verify connection
	println_I("Testing device connections...");
	if (CurieIMU.begin()) {
		println_I("CurieIMU connection successful");
	} else {
		println_E("CurieIMU connection failed");
	}
	println_I("About to calibrate. Make sure your board is stable and upright");
	delay(5000);

	// The board must be resting in a horizontal position for
	// the following calibration procedure to work correctly!
	println_I("Starting Gyroscope calibration and enabling offset compensation...");
	CurieIMU.autoCalibrateGyroOffset();
	print_I(" Done");

	println_I("Starting Acceleration calibration and enabling offset compensation...");
	CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
	CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
	CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
	print_I(" Done");

	println_I("Internal sensor offsets AFTER calibration...");
	p_int_I(CurieIMU.getAccelerometerOffset(X_AXIS));
	print_I("\t");// -76
	p_int_I(CurieIMU.getAccelerometerOffset(Y_AXIS));
	print_I("\t");// -2359
	p_int_I(CurieIMU.getAccelerometerOffset(Z_AXIS));
	print_I("\t");// 1688
	p_int_I(CurieIMU.getGyroOffset(X_AXIS));
	print_I("\t");// 0
	p_int_I(CurieIMU.getGyroOffset(Y_AXIS));
	print_I("\t");// 0
	p_int_I(CurieIMU.getGyroOffset(Z_AXIS));
#endif
	startupFlag = true;
}

uint8_t GetServoPos(uint8_t pin) {
	return velocity[PIN_TO_SERVO(pin)].set;
}
void SetServoPos(uint8_t pin, uint16_t val, float time) {
	SetServoPosDataTable(pin, val, time);

	getInterpolatedPin(pin);
}
void updateServos() {
	//println_E("Update");
#ifdef ARDUINO_ARCH_ARC32
	if(startupFlag)
		CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);
#endif

	for (int i = 0; i < MAX_SERVOS; i++) {
		if (GetChannelMode(i) == IS_SERVO)
			myservo[PIN_TO_SERVO(i)].write(getInterpolatedPin(PIN_TO_SERVO(i)));
	}

}

void SetServoPosDataTable(uint8_t pin, uint16_t val, float time) {
	if (time < 30 || isnan(velocity[pin].set)) {
		velocity[pin].setTime = 0;
		velocity[pin].start = (float) val;
	} else {
		// Set the start value to the pervious value
		velocity[pin].setTime = time;
		velocity[pin].start = velocity[pin].set;
	}
	velocity[pin].set = (float) val;
	velocity[pin].startTime = getMs();
}

uint8_t getInterpolatedPin(uint8_t pin) {
	if (GetChannelMode(pin) != IS_SERVO) {
		return 0;
	}
	//char cSREG;
	//cSREG = SREG;
	/* store SREG value */
	/*
	 disable interrupts during timed sequence */
	//StartCritical();
	float ip = interpolate(&velocity[pin], getMs());
	//SREG = cSREG;
	boolean error = false;
	if (ip > (255 - SERVO_BOUND)) {
		println_W("Upper=");
		error = true;
	}
	if (ip < SERVO_BOUND) {
		println_W("Lower=");
		error = true;
	}
	int dataTableSet = (getDataTableCurrentValue(pin) & 0x000000ff);
	int interpolatorSet = ((int32_t) velocity[pin].set);
	float time = (float) ((getDataTableCurrentValue(pin) >> 16) & 0x0000ffff);

	if (dataTableSet != interpolatorSet) {
//		println_W("Setpoint=");
//				error = true;
		SetServoPosDataTable(pin, dataTableSet, time);
		//myservo[PIN_TO_SERVO(pin)].write(bval);
	}
	if (error) {
//		p_fl_W(ip);print_W(" on chan=");p_int_W(pin);print_W(" target=");p_int_W(interpolatorSet);
//		print_W(" Data Table=");p_int_W(dataTableSet);
//		println_W("set=      \t");p_fl_W(velocity[pin].set);
//		println_W("start=    \t");p_fl_W(velocity[pin].start);
//		println_W("setTime=  \t");p_fl_W(velocity[pin].setTime);
//		println_W("startTime=\t");p_fl_W(velocity[pin].startTime);
		ip = velocity[pin].set;
		SetServoPosDataTable(pin, dataTableSet, time);
	}
	int tmp = (int) ip;

	return tmp;
}
