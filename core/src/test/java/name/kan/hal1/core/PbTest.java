package name.kan.hal1.core;

import name.kan.hal1.arduino.Arduino;
import org.junit.Test;

import java.io.ByteArrayInputStream;

import static org.junit.Assert.assertEquals;

/**
 * @author kan
 * @since 2014-04-22 21:34
 */
public class PbTest
{
	@Test
	public void testPb() throws Exception
	{
		final ByteArrayInputStream is = new ByteArrayInputStream(testSignals());
		final Arduino.Signals signals = Arduino.Signals.parseFrom(is);
		assertEquals("ard1", signals.getDeviceId());
		assertEquals("room1", signals.getThermometers(0).getDeviceId());
	}

	private byte[] testSignals()
	{
		final Arduino.Thermometer t1 = Arduino.Thermometer.newBuilder()
				.setDeviceId("room1")
				.setType(Arduino.Thermometer.Type.LM35)
				.build();
		final Arduino.Signals signals = Arduino.Signals.newBuilder()
				.setDeviceId("ard1")
				.addThermometers(t1)
				.build();
		return signals.toByteArray();
	}
}
