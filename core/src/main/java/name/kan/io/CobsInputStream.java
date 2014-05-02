package name.kan.io;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.StreamCorruptedException;

/**
 * Implementation of http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing algorithm
 * @author kan
 * @since 2014-04-30 21:11
 */
public class CobsInputStream extends InputStream
{
	private final InputStream in;
	private byte stuff;
	private boolean overflowStuff;

	public CobsInputStream(final InputStream in)
	{
		this.in = in;
	}

	@Override
	public int read() throws IOException
	{
		if(stuff == 0)
		{
			final int read = inRead();
			if(read == -1)
				throw new EOFException("Expected stuffing");
			setStuff(read);
		}
		final int read = inRead();
		if(read == -1)
			return -1;
		stuff--;
		if(stuff == 0)
		{
			if(overflowStuff)
			{
				setStuff(read);
				stuff--;
				return inRead();
			} else
			{
				setStuff(read);
				return 0;
			}
		}
		else
		{
			return read;
		}
	}

	private int inRead() throws IOException
	{
		int read = in.read();
		if(read == 0)
			throw new StreamCorruptedException("Unexpected zero");
		return read;
	}

	private void setStuff(final int read) throws IOException
	{
		stuff = (byte) read;
		overflowStuff = read == 255;
	}
}
