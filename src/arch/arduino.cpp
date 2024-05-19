#include "Arduino.h"

#include <TaskScheduler.h>

#include "../common/pecan.hpp"
#include "CAN.h"
typedef Task ard_event;

int16_t defaultPacketRecv(CANPacket *packet) {
    Serial.print("Default Received packet, id ");
    Serial.print(packet->id);
    Serial.print(", with data ");
    Serial.println((char*) packet->data);
}

int16_t waitPackets(CANPacket *recv_pack, PCANListenParamsCollection *plpc) {
    if (recv_pack == NULL) {
        CANPacket p;
        recv_pack = &p;
        // We can only use this for handler
        // Because stack
    }

    int8_t packetsize;
    CANListenParam clp;
    int16_t id;
    if ((packetsize = CAN.parsePacket())) {
        id = CAN.packetId();
        recv_pack->id = id;
        recv_pack->dataSize = packetsize;
        // Read and temporarily store all the packet data into PCAN
        // packet (on stack memory)
        for (int8_t j = 0; j < recv_pack->dataSize; j++) {
            recv_pack->data[j] = CAN.read();
        }
        // Then match the packet id with our params; if none
        // match, use default handler
        for (int16_t i = 0; i < plpc->size; i++) {
            clp = plpc->arr[i];
            if (matcher[clp.mt](id, clp.listen_id)) {
                return clp.handler(recv_pack);
            }
        }
        return plpc->defaultHandler(recv_pack);
    }
    return NOT_RECEIVED;
}

int16_t sendPacket(CANPacket *p) {
    if (p->dataSize > MAX_SIZE_PACKET_DATA) {
        return PACKET_TOO_BIG;
    }
    CAN.beginPacket(p->id);
    for (int8_t i = 0; i < p->dataSize; i++) {
        CAN.write(p->data[i]);
    }
    if (CAN.endPacket()) {
        Serial.println("Success!!!");
    } else {
        return GEN_FAILURE;
    }
    return SUCCESS;
}

/**
 * @brief 
 * 
 * @param inp Just pass in null, it's not used; this will allocate
 *              20 stuff on stack by itself 
 */
void PScheduler::mainloop(int8_t *inp) {
    Scheduler ts;
    Task tss[MAX_TASK_COUNT] = {};
    for (int16_t i = 0; i < this->ctr; i++) {
        tss[i].set(this->tasks[i].interval, TASK_FOREVER,
                   this->tasks[i].function);
        ts.addTask(tss[i]);
    }
    ts.enableAll();
    while(1)
        ts.execute();
}