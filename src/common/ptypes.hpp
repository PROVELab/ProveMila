#ifndef PTYPES_H
#define PTYPES_H

#define MAX_SIZE_PACKET_DATA 8
// DEFINITIONS, TYPEDEFS
#define MAX_TASK_COUNT 20
#define MAX_PCAN_PARAMS 20

// Fills byte buffer b with valRef using unions
template <typename T>
void fillBuf(char* b, T value);

// Fills valRef from b using unions
template <typename T>
void unFillBuf(char* b, T* valRef);

#endif