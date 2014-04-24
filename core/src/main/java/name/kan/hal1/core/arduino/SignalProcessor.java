package name.kan.hal1.core.arduino;

import name.kan.hal1.arduino.Arduino;
import name.kan.hal1.core.device.DeviceDao;
import name.kan.hal1.core.sensor.temperature.TemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureProcessor;

import javax.inject.Inject;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Map;

/**
 * @author kan
 * @since 2014-04-22 23:53
 */
public class SignalProcessor
{
	private final DeviceDao deviceDao;
	private final TemperatureProcessor dao;
	private final Map<Arduino.Thermometer.Type, TemperatureConverter> converters;

	@Inject
	public SignalProcessor(
			final DeviceDao deviceDao,
			final TemperatureProcessor dao,
			final Map<Arduino.Thermometer.Type, TemperatureConverter> converters)
	{
		this.deviceDao = deviceDao;
		this.dao = dao;
		this.converters = converters;
	}

	public void process(InputStream is) throws IOException
	{
		final Arduino.Signals signals = Arduino.Signals.parseFrom(is);
		final String deviceId = signals.getDeviceId();
		processThermometers(deviceId, signals.getThermometersList());
	}

	private void processThermometers(final String deviceId, final List<Arduino.Thermometer> thermometers)
	{
		for(final Arduino.Thermometer thermometer : thermometers)
		{
			final long device = deviceDao.findLogicalDevice(deviceId, thermometer.getDeviceId());
			final Arduino.Thermometer.Type type = thermometer.getType();
			final TemperatureConverter converter = converters.get(type);
			final int milliCelsius = converter.rawToMilliCelsius(thermometer.getValue());
			dao.recordTemperature(device, milliCelsius);
		}
	}
}
