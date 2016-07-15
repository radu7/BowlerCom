/**
 * Bowler Communication System
 */
#include <Arduino.h>
#include <BowlerCom.h>

static BowlerCom * ref;
static Stream * serialPort = NULL;
boolean comsStarted = false;
//DATA_STRUCT functionData[TOTAL_PINS];
//AbsPID mypidGroups[NUM_PID_GROUPS];
//DYIO_PID mydyPid[NUM_PID_GROUPS];
//PidLimitEvent mylimits[NUM_PID_GROUPS];
float getMs(void) {
	return ((float) micros()) * 1000.0f;
}
void putCharDebug(char a) {
	// none
	if (serialPort != NULL) {
		serialPort->print(a);
		serialPort->flush();
	}

}
void EnableDebugTerminal() {
	//none
}
BowlerCom::BowlerCom()
{
	ref = this;
	addedDyIO = false;
	addedPID = false;
	comPort=NULL;
	Bowler_Init();
}

/* begin method for overriding default serial bitrate */
void BowlerCom::begin(Stream * port) {
	begin_local( port);
}

/* begin method for overriding default serial bitrate */
void BowlerCom::begin(long speed) {
	Serial.begin(speed);
	while (!Serial);    // wait for the serial port to open
	begin_local(&Serial);
}

void BowlerCom::begin_local(Stream * port) {
	InitByteFifo(&store, privateRXCom, comBuffSize);
	comPort = port;

}
void showString(PGM_P s,Print_Level l,char newLine) {
	if(!okToprint(l)) {
		return;
	}
	if(newLine) {
		putCharDebug('\n');
		putCharDebug('\r');
	}
	setColor(l);
	char c;
	while ((c = pgm_read_byte(s++)) != 0)
	putCharDebug(c);
}
uint16_t putStream(uint8_t * buffer, uint16_t datalength) {
	uint16_t i;
	for (i = 0; i < datalength; i++) {
		//Grab the response packet one byte at a time and push it out the physical layer
		ref->comPort->write((char) buffer[i]);
	}
	ref->comPort->flush();
	return true;
}
boolean PutBowlerPacketDummy(BowlerPacket * Packet){
	//do nothing
	return true;
}

void BowlerCom::server(void) {
	byte err;
	byte newByte = 0;
	while (comPort->available() > 0) {
		newByte = comPort->read();
		//println_I("Adding byte: ");prHEX8(newByte,INFO_PRINT);
		FifoAddByte(&store, (char) newByte, &err);
	}
	if (GetBowlerPacket(&Packet, &store)) {
		//Now the Packet struct contains the parsed packet data
		Process_Self_Packet(&Packet);
		// The call backs for processing the packet have been called
		// and the Packet struct now contains the data
		// to be sent back to the client as a response.
		PutBowlerPacket(&Packet);
		comsStarted=true;
	}
	if(comsStarted)
		RunNamespaceAsync(&Packet, &PutBowlerPacket);
	else
		RunNamespaceAsync(&Packet, &PutBowlerPacketDummy);
}
void BowlerCom::addDyIO() {
	if (addedDyIO) {
		return;
	}
	addedDyIO = true;
	println_I("Initializing Pins");
	InitPinFunction(new DATA_STRUCT[TOTAL_PINS]);
	println_I("Adding IO Namespace");
	addNamespaceToList(get_bcsIoNamespace());
	println_I("Adding IO.Setmode Namespace");
	addNamespaceToList(get_bcsIoSetmodeNamespace());
	println_I("Adding DyIO Namespace");
	addNamespaceToList(get_neuronRoboticsDyIONamespace());
	println_I("Adding Safe Namespace");
	addNamespaceToList(get_bcsSafeNamespace());

}
void BowlerCom::addDyIOPID() {
	if (addedPID) {
		return;
	}
	addedPID = true;
	addDyIO();
//	InitPID(mypidGroups,
//			mydyPid,
//			mylimits
//			);
	InitPID(new AbsPID [NUM_PID_GROUPS],
				new DYIO_PID [NUM_PID_GROUPS],
				new PidLimitEvent [NUM_PID_GROUPS]
				);
	println_I("Adding PID Namespace");
	addNamespaceToList(getBcsPidNamespace());
	println_I("Adding DyIO PID Namespace");
	addNamespaceToList(get_bcsPidDypidNamespace());
}
void startDebugPint(Stream * port) {
	if (port == NULL)
		return;
	serialPort = port;
	setPrintLevelInfoPrint();
	println_I("\n\n\n##############################");
	println_I("  Welcome To Arduino Bowler!");
	println_I("##############################\n");
	println_E("Error Prints");
	println_W("Warning Prints");
	println_I("Info Prints");
}

