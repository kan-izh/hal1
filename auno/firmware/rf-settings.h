#ifndef RFSETTINGS_H
#define RFSETTINGS_H

#include "../../conf/rf-comm.h"


const uint8_t RF_cepin = 8;
const uint8_t RF_cspin = 9;
const uint64_t RF_address_read = RF_COMM_ADDRESS_RPI_TO_ARDUINO;
const uint64_t RF_address_write = RF_COMM_ADDRESS_ARDUINO_TO_RPI;
const uint8_t RF_channel = 0x4c;

#endif // RFSETTINGS_H
