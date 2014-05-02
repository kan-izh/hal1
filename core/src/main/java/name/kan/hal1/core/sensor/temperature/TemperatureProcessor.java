package name.kan.hal1.core.sensor.temperature;

/**
 * @author kan
 * @since 2014-04-23 00:03
 */
public interface TemperatureProcessor
{
	void recordTemperature(int deviceId, int milliCelsius);
}
