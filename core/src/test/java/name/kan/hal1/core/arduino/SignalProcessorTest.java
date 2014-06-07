package name.kan.hal1.core.arduino;

import name.kan.hal1.arduino.Arduino;
import name.kan.hal1.core.sensor.temperature.TemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureProcessor;
import org.junit.Before;
import org.junit.Test;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import java.io.ByteArrayInputStream;
import java.util.HashMap;
import java.util.Map;

import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * @author kan
 * @since 2014-04-23 00:41
 */
public class SignalProcessorTest
{
	private static final Arduino.Thermometer.Device ARD1_ROOM1_DEVICE_ID = Arduino.Thermometer.Device.Test;
	private static final Arduino.Thermometer.Device ARD1_ROOM2_DEVICE_ID = Arduino.Thermometer.Device.Desk;

	@InjectMocks SignalProcessor testObject;

	@Mock TemperatureProcessor dao;
	@Spy Map<Arduino.Thermometer.Type, TemperatureConverter> converters = new HashMap<>();
	@Mock TemperatureConverter lm35Converter;

	@Before
	public void setUp() throws Exception
	{
		MockitoAnnotations.initMocks(this);
		when(lm35Converter.rawToMilliCelsius(35)).thenReturn(17089);
		when(lm35Converter.rawToMilliCelsius(36)).thenReturn(17578);

		converters.put(Arduino.Thermometer.Type.LM35, lm35Converter);
	}

	@Test
	public void testProcess() throws Exception
	{
		final ByteArrayInputStream is = new ByteArrayInputStream(testSignals());
		testObject.process(is);
		verify(dao).recordTemperature(ARD1_ROOM1_DEVICE_ID, 17089);
		verify(dao).recordTemperature(ARD1_ROOM2_DEVICE_ID, 17578);

	}

	private byte[] testSignals()
	{
		final Arduino.Thermometer t1 = Arduino.Thermometer.newBuilder()
				.setDevice(ARD1_ROOM1_DEVICE_ID)
				.setType(Arduino.Thermometer.Type.LM35)
				.setValue(35)
				.build();
		final Arduino.Thermometer t2 = Arduino.Thermometer.newBuilder()
				.setDevice(ARD1_ROOM2_DEVICE_ID)
				.setType(Arduino.Thermometer.Type.LM35)
				.setValue(36)
				.build();
		final Arduino.Signals signals = Arduino.Signals.newBuilder()
				.setDevice(Arduino.Signals.Device.Auno)
				.addThermometers(t1)
				.addThermometers(t2)
				.build();
		return signals.toByteArray();
	}
}
