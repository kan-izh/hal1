#include "Arduino.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "arduino.pb.h"

//after build -- to flash
//sudo avrdude -V -c dragon_isp -p m168 -b 19200 -P usb -U flash:w:cmaketest.hex

unsigned long interval = 2000;
unsigned long time = 0;

bool thermometersList(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    name_kan_hal1_arduino_Thermometer thermometer;
    thermometer.device = name_kan_hal1_arduino_Thermometer_Device_Desk;
    thermometer.type = name_kan_hal1_arduino_Thermometer_Type_LM35;
    thermometer.value = analogRead(0);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    if (!pb_encode_submessage(stream, name_kan_hal1_arduino_Thermometer_fields, &thermometer))
        return false;

    return true;
}

bool serial_writer(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    return Serial.write(buf, count) == count;
}

void sendSignals()
{
    name_kan_hal1_arduino_Signals result;
    result.device = name_kan_hal1_arduino_Signals_Device_Auno;
    result.thermometers.funcs.encode = thermometersList;
    result.thermometers.arg = NULL;
    pb_ostream_t os;
    os.callback = serial_writer;
    os.state = NULL;
    os.max_size = -1;
    os.bytes_written = 0;
    if(!pb_encode(&os, name_kan_hal1_arduino_Signals_fields, &result))
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        return;
    }
    Serial.flush();
}

int main()
{
    init();
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    for(;;)
    {
        unsigned long currentMillis = millis();
        if(currentMillis - time > interval)
        {
          time = currentMillis;
          sendSignals();
        }
    }
    return 0;
}
