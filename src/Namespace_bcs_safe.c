/**
 * @file Namespace_bcs_safe.c
 *
 * Created on: Mar 16, 2013
 * @author hephaestus
 */
#include <BowlerCom.h>
int getHeartBeatTime();
boolean getHeartBeatLock();
void setHeartBeatState(boolean hb, int time);

//char safeNSName[] = "bcs.safe.*;0.3;;";

boolean bcsSafeAsyncEventCallback(BowlerPacket *Packet, boolean(*pidAsyncCallbackPtr)(BowlerPacket *Packet)) {
    //println_I("Safe Async ");

    return false; 
}

boolean bcsSafeProcessor_g(BowlerPacket * Packet) {
    //uint8_t temp0;
    UINT16_UNION timeUnion;
    switch (Packet->use.head.RPC) {
        case SAFE:
            Packet->use.head.Method = BOWLER_POST;
            Packet->use.data[0] = getHeartBeatLock();
            timeUnion.Val = getHeartBeatTime();
            Packet->use.data[1] = timeUnion.byte.SB;
            Packet->use.data[2] = timeUnion.byte.LB;
            Packet->use.head.DataLegnth = 4 + 3;
            break;
        default:
            return false; 
    }
    return true; 
}

boolean bcsSafeProcessor_p(BowlerPacket * Packet) {
    //uint8_t temp0;
    UINT16_UNION timeUnion;
    uint8_t zone = 5;
    switch (Packet->use.head.RPC) {

        case SAFE:
            timeUnion.byte.SB = Packet->use.data[1];
            timeUnion.byte.LB = Packet->use.data[2];
            setHeartBeatState(Packet->use.data[0], timeUnion.Val);
            READY(Packet, zone, 7);
            break;
        default:
            return false; 
    }
    return true; 
}

  RPC_LIST bcsSafe_safe_g = {BOWLER_GET, // Method
    "safe", //RPC as string
    &bcsSafeProcessor_g, //function pointer to a packet parsinf function
    { 0}, // Calling arguments
    BOWLER_POST, // response method
    {BOWLER_I08,// heartbeat lockout
        BOWLER_I16,// heartbeet time
        0
    }, // Calling arguments
    NULL //Termination
};

  RPC_LIST bcsSafe_safe_p = {BOWLER_POST, // Method
    "safe", //RPC as string
    &bcsSafeProcessor_p, //function pointer to a packet parsinf function
    {BOWLER_I08,// heartbeat lockout
        BOWLER_I16,// heartbeet time
        0
    }, // Calling arguments
    BOWLER_POST, // response method
    {BOWLER_I08, // code
        BOWLER_I08, // trace
        0
    }, // Calling arguments
    NULL //Termination
};



  NAMESPACE_LIST bcsSafe = {"bcs.safe.*;0.3;;", // The string defining the namespace
    NULL, // the first element in the RPC list
    &bcsSafeAsyncEventCallback, // async for this namespace
    NULL// no initial elements to the other namesapce field.
};

  boolean bcsSafenamespcaedAdded = false;

NAMESPACE_LIST * get_bcsSafeNamespace() {
    if (!bcsSafenamespcaedAdded) {
        //POST
        //Add the RPC structs to the namespace
        addRpcToNamespace(&bcsSafe, & bcsSafe_safe_g);
        addRpcToNamespace(&bcsSafe, & bcsSafe_safe_p);

        bcsSafenamespcaedAdded = true;
    }

    return &bcsSafe; //Return pointer to the struct
}
