/**
 *
 * Copyright 2009 Neuron Robotics, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "BowlerStack/include/Bowler/Bowler.h"
#include "Debug_DyIO.h"
#include "ChannelMode.h"
#include "ChannelValues.h"
#include "BowlerStack/include/Bowler/AbstractPID.h"
#ifndef DYIO_DEF_H_
#define DYIO_DEF_H_
#ifdef __cplusplus
extern "C"{
#endif

//#define ROBOSUB_DEMO

//#define NUM_PINS   			TOTAL_PINS

#define SERVO_BOUND 		0

//#define MAJOR_REV			3
//#define MINOR_REV			7
//#define FIRMWARE_VERSION	19


//neuronrobotics.dyio
#define _REV				0x7665725f // '_rev'  Get the revision number
#define INFO				0x6f666e69 // 'info'  Get and set the name string
#define _PWR				0x7277705f // '_pwr'  Request the power state

//private internal RPC's for communication between the host and co processor
#define EEPD				0x64706565// Interface with the EEPROM
#define DASN			    0x6e736164 // 'dasn'  Get 8 bit Async data
#define AASN				0x6e736161 // 'aasn'  Get 16 bit Async data
#define SAVE				0x65766173 // 'save'  Get the save values for servos
#define _MAC				0x63616D5F// Provision MAC address
#define POVR				0x72766f70 // 'povr'  Power override packet

//bcs.pid.dypid
#define DPID				0x64697064 // 'dpid'  Dynamic PID

#define FLAG_BLOCK 0
#define FLAG_OK    1

#define NUM_PID_GROUPS 4
#define NOT_USED_IN_PID 0xff
#define lockCodeValue  0x37

struct _local_pid{
		uint8_t lockCode;
        unsigned 					:5;
        unsigned		Async		:1;
        unsigned		Polarity	:1;
        unsigned		Enabled 	:1;
        unsigned 		char inputMode;
        unsigned 		char inputChannel;
        unsigned 		char outputMode;
        unsigned 		char outputChannel;
        struct{
            double P;
            double I;
            double D;
        }K;
};

typedef struct  _pid_vales
{
	union{
		struct _local_pid data;
		uint8_t stream[sizeof(struct _local_pid)];
	};
} pid_vales;

typedef struct _EESTORAGE
{

		uint8_t lockByte;
		pid_vales pid[NUM_PID_GROUPS];
		uint8_t end;

} EESTORAGE;

#define NUM_PPM_CHAN 6
#define INVALID_PPM_LINK 0xff



#define pidValSize sizeof(pid_vales)
#define NAMESIZE 17
#define LOCKSIZE 5

#define NAMESTART 0
#define LOCKSTART (NAMESIZE+NAMESTART)
#define DATASTART (LOCKSTART +LOCKSIZE)
#define DATAVALID 37

#define PID_VAL_END ((pidValSize*(NUM_PID_GROUPS)))
#define PPM_END	     (PID_VAL_END+NUM_PPM_CHAN)
#define BROWNOUT_START  (PPM_END+1)
#define BROWNOUT_END 	(BROWNOUT_START+1)
#define  nameSize 17





boolean ConfigChannel(BowlerPacket * Packet);
void ClearPinState(uint8_t pin);
void InitPins(void);


boolean pinHasFunction(uint8_t pin, uint8_t function);
void runDyIOMain(void);

NAMESPACE_LIST * get_neuronRoboticsDyIONamespace();
NAMESPACE_LIST * get_bcsPidDypidNamespace() ;

NAMESPACE_LIST * get_bcsSafeNamespace();
void setCoProcBrownOutMode(boolean b);
uint8_t SetAllCoProcValues();
boolean LoadSerialTxData(uint8_t numValues,uint8_t * data);
uint8_t GetSerialRxData(uint8_t * data);
uint32_t getNumberOfSerialRxBytes();
void SetNewConfigurationDataTable(uint8_t pin, int32_t value);
int32_t GetConfigurationDataTable(uint8_t pin);
/**
 * A function for updating the servos intrpolation state
 */
void updateServos();
/**
 * A function for reading the state of the interpolation stack
 */
uint8_t GetServoPos(uint8_t pin);

/**
 * A function for setting the state of the interpolation stack
 */
void SetServoPos(uint8_t pin, uint16_t value, float ms);

void SyncDataTable();
/**
 * Formulate a Power packet
 */
void POWER(BowlerPacket * packet);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DYIO_DEF_H_ */
