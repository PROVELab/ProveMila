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
};

// A CANPacket: takes in 11-bit id, 8 bytes of data
struct CANPacket {
    int16_t id;
    int8_t data[MAX_SIZE_PACKET_DATA] = {0};
    int8_t dataSize = 0;
};

// A single CANListenParam
struct CANListenParam {
    int16_t listen_id;
    int16_t (*handler)(CANPacket*);
    MATCH_TYPE mt;

    CANListenParam() {}
    CANListenParam(int16_t x, int16_t (*y)(CANPacket*), MATCH_TYPE z) {
        listen_id = x;
        handler = y;
        mt = z;
    }
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

// Returns true if id == mask
bool exact(int id, int mask);
// Returns true if id matches the bits of mask
bool similar(int id, int mask);
static bool (*matcher[2])(int, int) = {exact, similar};

// Write size bytes to the packet, accounting
// For Max Length
int16_t writeData(CANPacket* p, int8_t* dataPoint, int16_t size);

struct PTask {
    void (*function)(void);  // Function to call
    int16_t delay = 0;       // Milliseconds from start before a task runs
    int16_t interval;        // Milliseconds between task runs
    bool locked;             // Lock CAN - Only applicable to multithreading
};

/* "Scheduler/TaskManager" */
class PScheduler {
private:
    int16_t ctr = 0;  // Counts how many events are in queue
    PTask tasks[MAX_TASK_COUNT];

public:
    PScheduler();
    /* Add the task to the task queue (will all be enabled
    when mainloop is called)
    return: PCAN_ERR - denotes if the we have too many tasks,
        or success
     */
    PCAN_ERR scheduleTask(PTask t);
    // Loop through the tasks, enabling all of them with
    // their specifications listening for packets
    [[noreturn]] void mainloop(int8_t* inp);
};

#endif