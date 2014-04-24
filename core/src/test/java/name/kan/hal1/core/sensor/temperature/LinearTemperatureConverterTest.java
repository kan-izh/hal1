package name.kan.hal1.core.sensor.temperature;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author kan
 * @since 2014-04-24 21:03
 */
public class LinearTemperatureConverterTest
{
	LinearTemperatureConverter testObject = new LinearTemperatureConverter();

	@Test
	public void testRawToMilliCelsius() throws Exception
	{
		assertEquals(17089, testObject.rawToMilliCelsius(35));
	}
}
