#ifndef RF_COMM_H
#define RF_COMM_H

const uint8_t RF_COMM_ADDRESS_WIDTH = 3;
const uint8_t RF_COMM_ADDRESS_RPI_TO_ARDUINO[] = {4, 5, 6};
const uint8_t RF_COMM_ADDRESS_ARDUINO_TO_RPI[] = {1, 2, 3};
const uint8_t RF_COMM_CHANNEL_ARDUINO_TO_RPI = 0x4c;
const uint8_t RF_PAYLOAD_SIZE = 8;


#endif //RF_COMM_H
