package name.kan.hal1.core.arduino;

import name.kan.hal1.arduino.Arduino;
import name.kan.hal1.core.sensor.temperature.TemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureProcessor;

import javax.inject.Inject;
import java.io.IOException;
import java.util.List;
import java.util.Map;

/**
 * @author kan
 * @since 2014-04-22 23:53
 */
public class SignalProcessor
{
	private final TemperatureProcessor dao;
	private final Map<Arduino.Thermometer.Type, TemperatureConverter> converters;

	@Inject
	public SignalProcessor(
			final TemperatureProcessor dao,
			final Map<Arduino.Thermometer.Type, TemperatureConverter> converters)
	{
		this.dao = dao;
		this.converters = converters;
	}

	public void process(final Arduino.Signals signals) throws IOException
	{
		processThermometers(signals.getThermometersList());
	}

	private void processThermometers(final List<Arduino.Thermometer> thermometers)
	{
		for(final Arduino.Thermometer thermometer : thermometers)
		{
			final Arduino.Thermometer.Type type = thermometer.getType();
			final TemperatureConverter converter = converters.get(type);
			final int milliCelsius = converter.rawToMilliCelsius(thermometer.getValue());
			dao.recordTemperature(thermometer.getDevice(), milliCelsius);
		}
	}
}
