#include "mbed.h"

#include "../common/pecan.hpp"

// The CAN var used by PCAN
CAN can1(p30, p29, 500E3);

// Default receiver
int16_t defaultPacketRecv(CANPacket* packet) {
    printf("Default handler: id %d, with data: %s\n", packet->id, packet->data);
    return 0;
}

int16_t waitPackets(CANPacket* recv_pack, PCANListenParamsCollection* plpc) {
    // If we don't get one passed in,
    // that means the sender doesn't want it
    if (recv_pack == NULL) {
        CANPacket p;
        recv_pack = &p;
    }

    CANMessage msg;
    CANListenParam clp;
    int16_t out, id;
    if ((out = can1.read(msg))) {
        id = msg.id;
        recv_pack->id = msg.id;
        recv_pack->dataSize = msg.len;
        // Read and temporarily store all the packet data into PCAN
        // packet (on stack memory)
        for (int8_t j = 0; j < msg.len; j++) {
            recv_pack->data[j] = msg.data[j];
        }
        // Then match the packet id with our params; if none
        // match, use default handler
        for (int16_t i = 0; i < plpc->size; i++) {
            clp = plpc->arr[i];
            if (matcher[clp.mt](id, clp.listen_id)) {
                return clp.handler(recv_pack);
            }
        }
        // If we got this far, we don't have any matches, just a general packet.
        // Call default.
        return plpc->defaultHandler(recv_pack);
    }
    return NOT_RECEIVED;
}

int16_t sendPacket(CANPacket* p) {
    if (p->dataSize > MAX_SIZE_PACKET_DATA) {
        return PACKET_TOO_BIG;
    }
    CANMessage msg(p->id, (uint8_t*)p->data, p->dataSize);
    if (can1.write(msg)) {
        return SUCCESS;
    }
    return GEN_FAILURE;
}
