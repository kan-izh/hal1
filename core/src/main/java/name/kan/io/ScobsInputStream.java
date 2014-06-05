package name.kan.io;

import java.io.IOException;
import java.io.InputStream;
import java.io.StreamCorruptedException;

/**
 * Streamed Consistent Overhead Byte Stuffing
 * A variance of of http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing algorithm, but for Streamed byte
 * sequences.
 * It puts a stuffing byte with to indicate length of following sequence (125 bytes max), sign bit of the byte
 * indicates if the sequence should be terminated by zero.
 * This variance allows to flush byte stream at any point.
 * @author kan
 * @since 2014-04-30 21:11
 */
public class ScobsInputStream extends InputStream
{
	private final InputStream in;
	private byte stuff;
	private boolean putZero;

	public ScobsInputStream(final InputStream in)
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
				return -1;
			setStuff((byte) read);
		}
		stuff--;
		if(stuff == 0 && putZero)
			return 0;
		return inRead();
	}

	private int inRead() throws IOException
	{
/*
		if(next != -1)
		{
			final int next = this.next;
			this.next = -1;
			return next;
		}
*/
		final int read = in.read();
		if(read == 0)
			throw new StreamCorruptedException("Unexpected zero");
		return read;
	}

	private void setStuff(final byte read) throws IOException
	{
		putZero = read < 0;
		stuff = read < 0 ? (byte) -read : read;
	}
}
