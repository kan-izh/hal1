package name.kan.hal1.core.sensor.temperature;

/**
 * @author kan
 * @since 2014-04-23 00:19
 */
public class LinearTemperatureConverter implements TemperatureConverter
{
	final private int supplyMilliVolts = 5000;
	final private int milliVoltsPerCelsius = 10;
	final private int analogSteps = 1024;

	@Override
	public int rawToMilliCelsius(final int rawValue)
	{
		return (rawValue * 1000 * supplyMilliVolts) / milliVoltsPerCelsius / analogSteps;
	}
}
