#ifndef RFSETTINGS_H
#define RFSETTINGS_H

#include "../../conf/rf-comm.h"


const uint8_t RF_cepin = 9;
const uint8_t RF_cspin = 10;
const uint8_t *RF_address_read = RF_COMM_ADDRESS_RPI_TO_ARDUINO;
const uint8_t *RF_address_write = RF_COMM_ADDRESS_ARDUINO_TO_RPI;
const uint8_t RF_channel = RF_COMM_CHANNEL_ARDUINO_TO_RPI;

#endif // RFSETTINGS_H
