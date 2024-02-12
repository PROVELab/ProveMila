#include "pecan.hpp"

// The general, independent implementation
int16_t combinedID(int16_t fn_id, int16_t node_id){
    return (fn_id << 7) + node_id;
}

void setSensorID(CANPacket * p, uint8_t sensorId){
    p->data[0] = sensorId;
}

int16_t addParam(PCANListenParamsCollection * plpc, CANListenParam clp){
    if (plpc->size + 1 > MAX_PCAN_PARAMS){
        return NOSPACE;
    } else {
        plpc->arr[plpc->size] = clp;
        plpc->size++;
        return SUCCESS;
    }
}

int16_t writeData(CANPacket * p, int8_t * dataPoint, int16_t size){
    
    int16_t i = p->dataSize;
    if (i + size > MAX_SIZE_PACKET_DATA){
        return NOSPACE;
    }

    for (; i < size; i++){
        // DataSize can be interpreted as both
        // Size, and Index
        // Casting to 16-bit because compiler not happy
        p->data[(int16_t)p->dataSize] = dataPoint[i];
        p->dataSize++;

        // This check should've been working above
        // But just in case, we'll do it in the loop as well
        if (i > MAX_SIZE_PACKET_DATA){
            return NOSPACE;
        }
    }
    return SUCCESS;
}

template <typename T>
void fillBuf(char * b, T value){
    union {
        T value;
        uint8_t buffer[sizeof(T)];
    } convert_o_tron;
    convert_o_tron.value = value;

    for (int16_t i = sizeof(T)-1; i >= 0; i--){
        b[i] = convert_o_tron.buffer[i];
    }
}

template <typename T>
void unFillBuf(char * b, T * valRef){
    union {
        T value;
        uint8_t buffer[sizeof(T)];
    } convert_o_tron;
    for (int16_t i = 0; i < sizeof(T); i++){
        convert_o_tron.buffer[i] = b[i];
    }
    *valRef = convert_o_tron.value;
}

bool exact(int id, int mask) {
    return id == mask;
}

bool similar(int id, int mask){
    return (id & mask) == id;
}

