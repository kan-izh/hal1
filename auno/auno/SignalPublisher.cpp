#include "Arduino.h"
#include "SignalPublisher.h"
#include "sos.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "arduino.pb.h"

static bool thermometersList(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
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
	SignalPublisher *sp = (SignalPublisher *)stream->state;
	return sp->scobsStream.write(buf, count) == count;
}

void SignalPublisher::send()
{
	name_kan_hal1_arduino_Signals result;
	result.device = name_kan_hal1_arduino_Signals_Device_Auno;
	result.thermometers.funcs.encode = thermometersList;
	result.thermometers.arg = NULL;
	pb_ostream_t os;
	os.callback = serial_writer;
	os.state = this;
	os.max_size = -1;
	os.bytes_written = 0;
	if(!pb_encode(&os, name_kan_hal1_arduino_Signals_fields, &result))
	{
		showSos();
		return;
	}

	scobsStream.flush();
	stream->write((uint8_t) 0);
	stream->flush();
}
