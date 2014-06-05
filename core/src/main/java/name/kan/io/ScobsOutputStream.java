package name.kan.io;

import java.io.IOException;
import java.io.OutputStream;

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
public class ScobsOutputStream extends OutputStream
{
	private static final int BUF_SIZE = 125;
	private final OutputStream out;
	private final byte buffer[] = new byte[BUF_SIZE];
	private int buffered = 0;

	public ScobsOutputStream(final OutputStream out)
	{
		this.out = out;
	}

	@Override
	public void write(final int b) throws IOException
	{
		if(b == 0)
		{
			finishBlock(true);
		}
		else
		{
			buffer[buffered++] = (byte) b;
			if(buffered == BUF_SIZE)
				finishBlock(false);
		}
	}

	@Override
	public void flush() throws IOException
	{
		finishBlock(false);
		out.flush();
	}

	@Override
	public void close() throws IOException
	{
		finishBlock(false);
		out.close();
	}

	private void finishBlock(final boolean hasZero) throws IOException
	{
		if(buffered == 0 && !hasZero)
			return;
		final int stuff = hasZero ? -(buffered + 1) : buffered;
		out.write(stuff);
		out.write(buffer, 0, buffered);
		buffered = 0;
	}
}
