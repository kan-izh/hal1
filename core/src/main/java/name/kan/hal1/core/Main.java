package name.kan.hal1.core;

import com.google.protobuf.InvalidProtocolBufferException;
import jssc.SerialPort;
import jssc.SerialPortException;
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
import java.io.InputStream;
import java.io.StreamCorruptedException;
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

		int portNo=0;
		final String [] ports = new String[]{"/dev/ttyACM3", "/dev/ttyACM4"};
		while(true)
		{
			final String portName = ports[portNo++ % ports.length];
			final SerialPort serialPort = new SerialPort(portName);
			try
			{
				serialPort.openPort();
				serialPort.setParams(115200, 8, 0, 0);
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
					catch(InvalidProtocolBufferException e)
					{
						Thread.sleep(500);
						log.warn("IO problem: {}", e.getMessage());
					}
				}
			}
			catch(IOException | SerialPortException e)
			{
				log.warn("IO problem: {}", e.getMessage());
				Thread.sleep(500);
			}
			finally
			{
				try
				{
					serialPort.closePort();
				}
				catch(Exception e)
				{
					log.debug("Cannot close port", e);
				}
			}
		}
	}

	private static ByteArrayOutputStream readPacket(final InputStream sis) throws IOException
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

	private static void alignStream(final InputStream sis) throws IOException
	{
		for(int dropped = 0; dropped < 1000; ++dropped)
		{
			final int read = sis.read();
			if(read == 0)
			{
				log.info("Dropped {} bytes during packet synchronisation", dropped);
				return;
			}
		}
		throw new StreamCorruptedException("Too many dropped");
	}

	private static class DumpTemperature implements TemperatureProcessor
	{
		int cnt;
		@Override
		public void recordTemperature(final Arduino.Thermometer.Device device, final int milliCelsius)
		{
			if(++cnt % 10000 == 0)
				log.info("Measure \u2116{}, Device {}: {}\u00B0C", cnt, device, (milliCelsius / 1000.0));
		}
	}

}
