package name.kan.io;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Implementation of http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing algorithm
 * @author kan
 * @since 2014-04-30 21:11
 */
public class CobsOutputStream extends OutputStream
{
	private final OutputStream out;
	private final byte buf[] = new byte[255];
	private int bufPos = 0;

	public CobsOutputStream(final OutputStream out)
	{
		this.out = out;
	}

	@Override
	public void write(final int b) throws IOException
	{
		if(b == 0)
		{
			finishBlock();
		}
		else
		{
			buf[bufPos++] = (byte) b;
			if(bufPos == 254)
				finishBlock();
		}
	}

	@Override
	public void flush() throws IOException
	{
		finishBlock();
		out.flush();
	}

	private void finishBlock() throws IOException
	{
		out.write(bufPos + 1);
		out.write(buf, 0, bufPos);
		bufPos = 0;
	}
}
