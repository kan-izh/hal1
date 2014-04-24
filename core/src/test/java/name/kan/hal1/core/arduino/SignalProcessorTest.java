package name.kan.hal1.core.arduino;

import name.kan.hal1.arduino.Arduino;
import name.kan.hal1.core.device.DeviceDao;
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
	private static final long ARD1_ROOM1_DEVICE_ID = 123L;
	private static final long ARD1_ROOM2_DEVICE_ID = 234L;
	private static final String BOARD_ID = "ard1";
	private static final String TEMPERATURE_CHIP_ID = "room1";
	private static final String TEMPERATURE_CHIP2_ID = "room2";

	@InjectMocks SignalProcessor testObject;

	@Mock DeviceDao deviceDao;
	@Mock TemperatureProcessor dao;
	@Spy Map<Arduino.Thermometer.Type, TemperatureConverter> converters = new HashMap<>();
	@Mock TemperatureConverter lm35Converter;

	@Before
	public void setUp() throws Exception
	{
		MockitoAnnotations.initMocks(this);
		when(deviceDao.findLogicalDevice("ard1", "room1")).thenReturn(ARD1_ROOM1_DEVICE_ID);
		when(deviceDao.findLogicalDevice("ard1", "room2")).thenReturn(ARD1_ROOM2_DEVICE_ID);
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
				.setDeviceId(TEMPERATURE_CHIP_ID)
				.setType(Arduino.Thermometer.Type.LM35)
				.setValue(35)
				.build();
		final Arduino.Thermometer t2 = Arduino.Thermometer.newBuilder()
				.setDeviceId(TEMPERATURE_CHIP2_ID)
				.setType(Arduino.Thermometer.Type.LM35)
				.setValue(36)
				.build();
		final Arduino.Signals signals = Arduino.Signals.newBuilder()
				.setDeviceId(BOARD_ID)
				.addThermometers(t1)
				.addThermometers(t2)
				.build();
		return signals.toByteArray();
	}
}
