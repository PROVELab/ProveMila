#ifndef PECAN_H
#define PECAN_H
#include <stdint.h>
#include "./ptypes.hpp"



// Various PCAN Error Code return values
enum PCAN_ERR {
    PACKET_TOO_BIG = -3,
    NOT_RECEIVED = -2,
    NOSPACE = -1,
    SUCCESS = 0,
    GEN_FAILURE = 1,
};

// Exact match or Similar match (with mask)
enum MATCH_TYPE {
    MATCH_EXACT = 0,
    MATCH_SIMILAR = 1,
    MATCH_FUNCTION =2,
};

// A CANPacket: takes in 11-bit id, 8 bytes of data
struct CANPacket {
    int32_t id;
    int8_t data[MAX_SIZE_PACKET_DATA] = {0};
    int8_t dataSize = 0;
    int8_t rtr=0;
};

// A single CANListenParam
struct CANListenParam {
    int32_t listen_id;
    int16_t (*handler)(CANPacket*);
    MATCH_TYPE mt;
};



// The default handler for a packet who
// we couldn't match with other params
int16_t defaultPacketRecv(CANPacket*);

// A collection containing an array of params to listen for
// and the default handler if none of the params match with
// a given packet
struct PCANListenParamsCollection {
    CANListenParam arr[MAX_PCAN_PARAMS];
    int16_t (*defaultHandler)(CANPacket*) = defaultPacketRecv;
    int16_t size = 0;
    
};

/// @brief Blocking wait on a packet with listen_id, other packets are ignored
/// @param recv_pack a pointer to a packet-sized place in
///                  such that the caller can see what
///                  the received packet is if necessary
///                  If NULL, the recv_pack will create its own
/// @param plpc A PCANListenParamsCollection, which specifies a bunch
///                  of ids to listen for, and their corresponding
///                  handler functions
/// @return 0 on success, nonzero on Failure (see PCAN_ERR enum)
int16_t waitPackets(CANPacket* recv_pack, PCANListenParamsCollection* plpc);

/// Adds a CANListenParam to the collection
int16_t addParam(PCANListenParamsCollection* plpc, CANListenParam clp);

/// Sends a CANPacket p
int16_t sendPacket(CANPacket* p);

// Combines a function id and node id into a full 11-bit id
int16_t combinedID(int16_t fn_id, int16_t node_id);

// Only in use with sensor stuff
void setSensorID(CANPacket* p, uint8_t sensorId);

//used to run a task one Time

// Returns true if id == mask
bool exact(int32_t id, int mask);
// Returns true if id matches the bits of mask
bool similar(int id, int mask);
// Returns true if the functionCode of id ==mask
bool exactFunction(int id, int mask);

static bool (*matcher[3])(int32_t, int32_t) = {exact, similar, exactFunction};

// Write size bytes to the packet, accounting
// For Max Length
int16_t writeData(CANPacket* p, int8_t* dataPoint, int16_t size);

int16_t setRTR(struct CANPacket * p);
#endif