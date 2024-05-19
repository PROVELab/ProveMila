#include <mbed.h>
#include <rtos.h>
#include "../common/pecan.hpp"
#include "mcu_node.hpp"

#if !DEVICE_CAN
#error[NOT_SUPPORTED] CAN not supported
#endif

Thread thread;

// The constructor takes in RX, and TX pin respectively
CAN can(p30, p29);

void sendCANOpenPacket(int functionCode, int nodeID, uint8_t *data)
{
    int can_id = functionCode | nodeID;
    can.write(CANMessage(can_id, data));
}

void recvSDO(int nodeID, uint16_t index, uint8_t subindex)
{
    uint8_t buffer[8] = {SDO_UPLOAD, (uint8_t) index, (uint8_t) (index >> 8), subindex, 0, 0, 0, 0};
    sendCANOpenPacket(CLIENT_TO_SERVER_SDO, nodeID, buffer);
}

void sendSDO(int nodeID, uint16_t index, uint8_t subindex, uint8_t data)
{
    uint8_t buffer[8] = {SDO_DOWNLOAD_1B, (uint8_t) index, (uint8_t) (index >> 8), subindex, data, 0, 0, 0};
    sendCANOpenPacket(CLIENT_TO_SERVER_SDO, nodeID, buffer);
}

void sendSDO(int nodeID, uint16_t index, uint8_t subindex, uint16_t data)
{
    uint8_t buffer[8] = {SDO_DOWNLOAD_2B, (uint8_t) index, (uint8_t) (index >> 8), subindex, (uint8_t) data, (uint8_t) (data >> 8), 0, 0};
    sendCANOpenPacket(CLIENT_TO_SERVER_SDO, nodeID, buffer);
}

void sendSDO(int nodeID, uint16_t index, uint8_t subindex, uint32_t data)
{
    uint8_t buffer[8] = {SDO_DOWNLOAD_4B, (uint8_t) index, (uint8_t) (index >> 8), subindex, (uint8_t) data, (uint8_t) (data >> 8), (uint8_t) (data >> 16), (uint8_t) (data >> 24)};
    sendCANOpenPacket(CLIENT_TO_SERVER_SDO, nodeID, buffer);
}

void sendPDO(uint16_t nodeID, uint16_t pdo_code, uint64_t data) {
    uint8_t buffer[8] = {(uint8_t) data, (uint8_t) (data >> 8), (uint8_t) (data >> 16), (uint8_t) (data >> 24), (uint8_t) (data >> 32), (uint8_t) (data >> 40), (uint8_t) (data >> 48), (uint8_t) (data >> 56)};
    sendCANOpenPacket(pdo_code, nodeID, buffer);
}

// Must be in pre-op state to change PDO mappings
void mapPDOEntry(uint16_t pdo_index, uint8_t pdo_subindex, uint16_t entry_index, uint8_t entry_subindex, uint8_t entry_size) {
    sendSDO(MOTOR_CONT_ID, pdo_index, pdo_subindex, (uint32_t) (entry_index << 16) | (entry_subindex << 8) | entry_size);
}

void configTPDO1() {
    // Disable TPDO1 communication 
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, TPDO1_PARAMETER_COBID_SUBINDEX, (uint32_t) DISABLE_PDO);

    // Disable TPDO1 mapping
    sendSDO(MOTOR_CONT_ID, TPDO1_MAPPING_INDEX, TPDO1_MAPPING_COUNT_SUBINDEX, (uint8_t) 0);

    // Map TPDO1 entries
    mapPDOEntry(TPDO1_MAPPING_INDEX, TPDO1_MAPPING_ENTRY_1_SUBINDEX, VELOCITY_ACTUAL_VALUE, 0x00, 32); // 4 bytes
    mapPDOEntry(TPDO1_MAPPING_INDEX, TPDO1_MAPPING_ENTRY_2_SUBINDEX, MOTOR_DC_CURRENT, 0x00, 32); // 4 bytes

    // Set transmission type
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, TPDO1_PARAMETER_TRANSMISSION_TYPE_SUBINDEX, (uint8_t) ASYNC);

    // Enable TPDO1 communication
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, TPDO1_PARAMETER_COBID_SUBINDEX, (uint32_t) T_PDO1 | MOTOR_CONT_ID);

    // Enable TPDO1 mapping
    sendSDO(MOTOR_CONT_ID, TPDO1_MAPPING_INDEX, TPDO1_MAPPING_COUNT_SUBINDEX, (uint8_t) 2);
}

void configTDPO2() {
    // Disable TPDO2 communication 
    sendSDO(MOTOR_CONT_ID, TPDO2_PARAMETER_INDEX, TPDO2_PARAMETER_COBID_SUBINDEX, (uint32_t) DISABLE_PDO);

    // Disable TPDO2 mapping
    sendSDO(MOTOR_CONT_ID, TPDO2_MAPPING_INDEX, TPDO2_MAPPING_COUNT_SUBINDEX, (uint8_t) 0);

    // Map TPDO2 entries
    mapPDOEntry(TPDO2_MAPPING_INDEX, TPDO2_MAPPING_ENTRY_1_SUBINDEX, MOTOR_TEMPERATURE, 0x00, 8); // 1 byte 
    mapPDOEntry(TPDO2_MAPPING_INDEX, TPDO2_MAPPING_ENTRY_2_SUBINDEX, MOTOR_DC_CURRENT, 0x00, 32); // 4 bytes

    // Set transmission type
    sendSDO(MOTOR_CONT_ID, TPDO2_PARAMETER_INDEX, TPDO2_PARAMETER_TRANSMISSION_TYPE_SUBINDEX, (uint8_t) ASYNC);

    // Enable TPDO2 communication
    sendSDO(MOTOR_CONT_ID, TPDO2_PARAMETER_INDEX, TPDO2_PARAMETER_COBID_SUBINDEX, (uint32_t) T_PDO2 | MOTOR_CONT_ID);

    // Enable TPDO2 mapping
    sendSDO(MOTOR_CONT_ID, TPDO2_MAPPING_INDEX, TPDO2_MAPPING_COUNT_SUBINDEX, (uint8_t) 2);
}

// Apply state change to all nodes by passing node_id = 0.
void sendNMT(uint16_t node_id, uint8_t nmt_command) {
    uint8_t buffer[2] = {nmt_command, (uint8_t) node_id};
    can.write(CANMessage(0x00, buffer, sizeof buffer));
}

// Starts bootload sequence.
void startBootload()
{
    // Set mode of operation.
    sendSDO(MOTOR_CONT_ID, 0x6060, 0, (uint8_t)9); // 9: Velocity mode

    // Set motor type.
    sendSDO(MOTOR_CONT_ID, 0x2034, 0, (uint8_t)0);

    // Set controller/motor protections.
    sendSDO(MOTOR_CONT_ID, OVERVOLTAGE_LIMIT, 0, (uint16_t)PLACEHOLDER);                                         // Overvoltage limit
    sendSDO(MOTOR_CONT_ID, UNDERVOLTAGE_LIMIT, UNDERVOLTAGE_LIMIT_SUBINDEX, (uint8_t)PLACEHOLDER);               // Undervoltage limit
    sendSDO(MOTOR_CONT_ID, UNDERVOLTAGE_MIN_VOLTAGE, UNDERVOLTAGE_MIN_VOLTAGE_SUBINDEX, (uint16_t)PLACEHOLDER);  // Undervoltage min voltage
    sendSDO(MOTOR_CONT_ID, MAXIMUM_CONTROLLER_CURRENT, 0, (uint32_t)PLACEHOLDER);                                // Maximum controller current
    sendSDO(MOTOR_CONT_ID, SECONDARY_CURRENT_PROTECTION, 0, (uint32_t)PLACEHOLDER);                              // Secondary current protection
    sendSDO(MOTOR_CONT_ID, MOTOR_MAXIMUM_TEMPERATURE, MOTOR_MAXIMUM_TEMPERATURE_SUBINDEX, (uint8_t)PLACEHOLDER); // Motor maximum temperature
    sendSDO(MOTOR_CONT_ID, MAXIMUM_VELOCITY, MAXIMUM_VELOCITY_SUBINDEX, (uint32_t)PLACEHOLDER);                  // Maximum velocity

    // Perform motor position measurement.
    sendSDO(MOTOR_CONT_ID, MOTOR_POLE_PAIRS, 0, (uint8_t)PLACEHOLDER);                                                            // Motor pole pairs
    sendSDO(MOTOR_CONT_ID, FEEDBACK_TYPE, FEEDBACK_TYPE_SUBINDEX, (uint8_t)PLACEHOLDER);                                          // Feedback type
    sendSDO(MOTOR_CONT_ID, MOTOR_PHASE_OFFSET, MOTOR_PHASE_OFFSET_SUBINDEX, (uint16_t)PLACEHOLDER);                               // Motor phase offset
    sendSDO(MOTOR_CONT_ID, HALL_CONFIGURATION, HALL_CONFIGURATION_SUBINDEX, (uint8_t)PLACEHOLDER);                                // Hall configuration
    sendSDO(MOTOR_CONT_ID, FEEDBACK_RESOLUTION, FEEDBACK_RESOLUTION_SUBINDEX, (uint16_t)PLACEHOLDER);                             // Feedback resolution
    sendSDO(MOTOR_CONT_ID, ELECTRICAL_ANGLE_FILTER, ELECTRICAL_ANGLE_FILTER_SUBINDEX, (uint16_t)PLACEHOLDER);                     // Electrical angle filter
    sendSDO(MOTOR_CONT_ID, MOTOR_PHASE_OFFSET_COMPENSATION, MOTOR_PHASE_OFFSET_COMPENSATION_SUBINDEX, (uint16_t)PLACEHOLDER);     // Motor phase offset compensation
    sendSDO(MOTOR_CONT_ID, VELOCITY_ENCODER_FACTOR_NUMERATOR, VELOCITY_ENCODER_FACTOR_NUMERATOR_SUBINDEX, (uint32_t)PLACEHOLDER); // Velocity encoder factor Numerator
    sendSDO(MOTOR_CONT_ID, VELOCITY_ENCODER_FACTOR_DIVISOR, VELOCITY_ENCODER_FACTOR_DIVISOR_SUBINDEX, (uint32_t)PLACEHOLDER);     // Velocity encoder factor Divisor

    // Configure velocity mode parameters.
    sendSDO(MOTOR_CONT_ID, TARGET_VELOCITY, 0, (uint32_t)PLACEHOLDER);                                                            // Target velocity
    sendSDO(MOTOR_CONT_ID, VELOCITY_ACTUAL_VALUE, 0, (uint32_t)PLACEHOLDER);                                                      // Velocity actual value
    sendSDO(MOTOR_CONT_ID, VELOCITY_CONTROL_REGULATOR_P_GAIN, VELOCITY_CONTROL_REGULATOR_P_GAIN_SUBINDEX, (uint16_t)PLACEHOLDER); // Velocity control regulator P gain
    sendSDO(MOTOR_CONT_ID, VELOCITY_CONTROL_REGULATOR_I_GAIN, VELOCITY_CONTROL_REGULATOR_I_GAIN_SUBINDEX, (uint16_t)PLACEHOLDER); // Velocity control regulator I gain
    sendSDO(MOTOR_CONT_ID, MOTOR_RATED_CURRENT, 0, (uint32_t)PLACEHOLDER);                                                        // Motor rated current
}

int16_t acceptSDOResponse(CANPacket *packet) {
    char val[4] = {0};
    memcpy(val, packet->data + 4, 4);
    printf("Received: %lu\n",  (*((uint32_t*) val)));
    return 0;
}

int16_t handleRPDO(CANPacket *packet) {
    char val[2] = {0};
    memcpy(val, packet->data, 2);
    printf("Received: %lu\n",  (*((uint16_t*) val)));
    return 0;
}

void testsendSDO(uint16_t index, uint8_t subindex, uint32_t data) {
    PCANListenParamsCollection pclp;

    CANListenParam stocSDO(SERVER_TO_CLIENT_SDO | MOTOR_CONT_ID, acceptSDOResponse, MATCH_EXACT);
    addParam(&pclp, stocSDO);

    // Read current value.
    printf("Reading Current Value\n");
    recvSDO(MOTOR_CONT_ID, index, subindex);

    while(waitPackets(NULL, &pclp) == NOT_RECEIVED)
        ;

    // Update value.
    printf("Writing new value\n");
    sendSDO(MOTOR_CONT_ID, index, subindex, data);

    while(waitPackets(NULL, &pclp) == NOT_RECEIVED)
        ;
    
    // Read updated value.
    printf("Reading updated value\n");
    recvSDO(MOTOR_CONT_ID, index, subindex);

    while(waitPackets(NULL, &pclp) == NOT_RECEIVED)
        ;

    printf("\n");
}

void testRecvSDO(uint16_t index, uint8_t subindex) {
    PCANListenParamsCollection pclp;

    CANListenParam stocSDO(SERVER_TO_CLIENT_SDO | MOTOR_CONT_ID, acceptSDOResponse, MATCH_EXACT);
    addParam(&pclp, stocSDO);

    printf("Reading OD Index\n");
    recvSDO(MOTOR_CONT_ID, index, subindex);

    while(waitPackets(NULL, &pclp) == NOT_RECEIVED)
        ;

    printf("\n");
}

void testConfigPDO() {
    sendNMT(MOTOR_CONT_ID, ENTER_PREOPERATIONAL);
    
    printf("Entered preop\n");

    ThisThread::sleep_for(4s);

    printf("starting mapping\n");
    // Map TPDO1

    // Disable TPDO1 communication 
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, TPDO1_PARAMETER_COBID_SUBINDEX, (uint32_t) DISABLE_PDO);
    
    // Disable TPDO mapping (set number of mapped objects to 0)
    sendSDO(MOTOR_CONT_ID, TPDO1_MAPPING_INDEX, TPDO1_MAPPING_COUNT_SUBINDEX, (uint8_t) 0);

    // Map TPDO1 entries
    mapPDOEntry(TPDO1_MAPPING_INDEX, TPDO1_MAPPING_ENTRY_1_SUBINDEX, 0x2040, 0x07, 16); // 2 bytes

    // Enable TPDO mapping (set number of mapped objects to 1)
    sendSDO(MOTOR_CONT_ID, TPDO1_MAPPING_INDEX, TPDO1_MAPPING_COUNT_SUBINDEX, (uint8_t) 1);

    // Set transmission type
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, TPDO1_PARAMETER_TRANSMISSION_TYPE_SUBINDEX, (uint8_t) 254);

    // Set event timer
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, 0x05, (uint16_t) 4000);

    // Enable TPDO1 communication
    sendSDO(MOTOR_CONT_ID, TPDO1_PARAMETER_INDEX, TPDO1_PARAMETER_COBID_SUBINDEX, (uint32_t) T_PDO1 | MOTOR_CONT_ID);

    // Save parameters
    sendSDO(MOTOR_CONT_ID, 0x1010, 0x01, (uint32_t) 0x65766173); // This evaluates to "save"
    printf("Saved parameters\n");

    // Reset controller
    printf("Resetting controller\n");
    sendNMT(MOTOR_CONT_ID, RESET_NODE);

    ThisThread::sleep_for(4s); // This wait seems necessary.
    
    // Disable other TPDOs
    // sendSDO(MOTOR_CONT_ID, TPDO2_PARAMETER_INDEX, TPDO2_PARAMETER_COBID_SUBINDEX, (uint32_t) DISABLE_PDO);
    // sendSDO(MOTOR_CONT_ID, TPDO3_PARAMETER_INDEX, TPDO3_PARAMETER_COBID_SUBINDEX, (uint32_t) DISABLE_PDO);
    // sendSDO(MOTOR_CONT_ID, TPDO4_PARAMETER_INDEX, TPDO4_PARAMETER_COBID_SUBINDEX, (uint32_t) DISABLE_PDO);

    // Monitor for response
    PCANListenParamsCollection pclp;
    // CANListenParam monitor_RPDO(T_PDO1 | MOTOR_CONT_ID, handleRPDO, MATCH_EXACT);
    // addParam(&pclp, monitor_RPDO);

    // Enter NMT operational state
    printf("entering op state\n");
    sendNMT(MOTOR_CONT_ID, ENTER_OPERATIONAL);

    while(1){
        waitPackets(NULL, &pclp);
    }
}

void testNMT() {
    while (1) {
        printf("Enter Operational State\n");
        sendNMT(MOTOR_CONT_ID, ENTER_OPERATIONAL);
        ThisThread::sleep_for(4s);

        testRecvSDO(0x2050, 0x00);

        printf("Enter Stop State\n");
        sendNMT(MOTOR_CONT_ID, ENTER_STOP);
        ThisThread::sleep_for(4s);

        printf("Enter Pre-Operational State\n");
        sendNMT(MOTOR_CONT_ID, ENTER_PREOPERATIONAL);
        ThisThread::sleep_for(4s);

        printf("\n");
    }
    
}

int main()
{
    can.frequency(500E3);
    testConfigPDO();
    // testPDOpolling();
    // testNMT();
}
