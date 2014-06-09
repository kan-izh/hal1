package name.kan.hal1.core;

import jssc.SerialPort;
import name.kan.hal1.arduino.Arduino;
import name.kan.hal1.core.arduino.SignalProcessor;
import name.kan.hal1.core.sensor.temperature.LinearTemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureProcessor;
import name.kan.io.ScobsInputStream;
import name.kan.io.SerialInputStream;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * @author kan
 * @since 2014-04-27 23:50
 */
public class Main
{

	private static final Logger log = LoggerFactory.getLogger(Main.class);

	public static void main(String[] args) throws Exception
	{
		final Map<Arduino.Thermometer.Type, TemperatureConverter> map = new HashMap<>();
		map.put(Arduino.Thermometer.Type.LM35, new LinearTemperatureConverter());
		final SignalProcessor processor = new SignalProcessor(new DumpTemperature(), map);

		final SerialPort serialPort = new SerialPort("/dev/ttyACM3");
		if(!serialPort.openPort())
			throw new IOException("Cannot open port");
		final SerialInputStream sis = new SerialInputStream(serialPort);
		while(true)
		{
			alignStream(sis);
			try
			{
				while(true)
				{
					final ByteArrayOutputStream baos = readPacket(sis);
					final ScobsInputStream scis = new ScobsInputStream(new ByteArrayInputStream(baos.toByteArray()));
					processor.process(Arduino.Signals.parseFrom(scis));
				}
			}
			catch(IOException e)
			{
				log.warn("IO problem: {}", e.getMessage());
			}
		}
	}

	private static ByteArrayOutputStream readPacket(final SerialInputStream sis) throws IOException
	{
		final ByteArrayOutputStream baos = new ByteArrayOutputStream();
		while(true)
		{
			final int read = sis.read();
			if(read != 0)
				baos.write(read);
			else
				return baos;
		}
	}

	private static void alignStream(final SerialInputStream sis) throws IOException
	{
		while(true)
		{
			final int read = sis.read();
			if(read == 0)
				return;
		}
	}

	private static class DumpTemperature implements TemperatureProcessor
	{
		int cnt;
		@Override
		public void recordTemperature(final Arduino.Thermometer.Device device, final int milliCelsius)
		{
			if(cnt++ % 60000 == 0)
				log.info("Measure \u2116{}, Device {}: {}\u00B0C", cnt, device, (milliCelsius / 1000.0));
		}
	}

}
