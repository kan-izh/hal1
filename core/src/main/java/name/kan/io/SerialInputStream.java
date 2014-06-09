package name.kan.io;

import jssc.SerialPort;
import jssc.SerialPortException;

import java.io.IOException;
import java.io.InputStream;

/**
* @author kan
* @since 2014-06-08 22:49
*/
public class SerialInputStream extends InputStream
{
	private final SerialPort serialPort;

	public SerialInputStream(final SerialPort serialPort)
	{
		this.serialPort = serialPort;
	}

	@Override
	public int read() throws IOException
	{
		try
		{
			return serialPort.readBytes(1)[0];
		} catch(SerialPortException e)
		{
			throw new IOException(e);
		}
	}

}
