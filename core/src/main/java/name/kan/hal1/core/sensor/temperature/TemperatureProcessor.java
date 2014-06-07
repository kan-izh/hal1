package name.kan.hal1.core.sensor.temperature;

import name.kan.hal1.arduino.Arduino;

/**
 * @author kan
 * @since 2014-04-23 00:03
 */
public interface TemperatureProcessor
{
	void recordTemperature(Arduino.Thermometer.Device device, int milliCelsius);
}
