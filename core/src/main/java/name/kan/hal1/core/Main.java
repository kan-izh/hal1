package name.kan.hal1.core;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;
import name.kan.hal1.arduino.Arduino;
import name.kan.hal1.core.arduino.SignalProcessor;
import name.kan.hal1.core.sensor.temperature.LinearTemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureConverter;
import name.kan.hal1.core.sensor.temperature.TemperatureProcessor;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * @author kan
 * @since 2014-04-27 23:50
 */
public class Main
{
	public static void main(String[] args) throws Exception
	{
		final Map<Arduino.Thermometer.Type, TemperatureConverter> map = new HashMap<>();
		map.put(Arduino.Thermometer.Type.LM35, new LinearTemperatureConverter());
		final SignalProcessor processor = new SignalProcessor(new DumpTemperature(), map);

		final SerialPort serialPort = new SerialPort("/dev/ttyACM3");
		if(!serialPort.openPort())
			throw new IOException("Cannot open port");
		processor.process(new SerialInputStream(serialPort));
	}

	private static class DumpTemperature implements TemperatureProcessor
	{
		@Override
		public void recordTemperature(final Arduino.Thermometer.Device device, final int milliCelsius)
		{
			System.out.println("device = " + device + ", " + (milliCelsius / 1000.0));
		}
	}

	private static class SerialInputStream extends InputStream
	{
		private final SerialPort serialPort;

		public SerialInputStream(final SerialPort serialPort)
		{
			this.serialPort = serialPort;
		}

		@Override
		public int read(final byte[] b, final int off, final int len) throws IOException
		{
			try
			{
				final byte[] bytes = serialPort.readBytes(len, 5000);
			} catch(SerialPortException e)
			{
				e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
			} catch(SerialPortTimeoutException e)
			{
			}
			return super.read(b, off, len);    //To change body of overridden methods use File | Settings | File Templates.
		}

		@Override
		public int read() throws IOException
		{
			try
			{
				final byte[] bytes = serialPort.readBytes(1, 1000);
				return bytes[0];
			} catch(SerialPortException e)
			{
				throw new IOException(e);
			} catch(SerialPortTimeoutException e)
			{
				return -1;
			}
		}
	}
}
