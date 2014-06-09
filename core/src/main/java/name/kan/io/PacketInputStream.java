package name.kan.io;

import java.io.IOException;
import java.io.InputStream;

/**
 * @author kan
 * @since 2014-06-08 22:50
 */
public class PacketInputStream extends InputStream
{
	private final InputStream in;
	boolean packetAligned;

	public PacketInputStream(final InputStream in)
	{
		this.in = in;
	}

	@Override
	public int read() throws IOException
	{
		if(!packetAligned)
		{
			while(true)
			{
				final int read = in.read();
				if(read == -1)
					return -1;
				if(read == 0)
					break;
			}
			packetAligned = true;
		}
		final int read = in.read();
		if(read != 0)
			return read;
		else
			return -1;
	}
}
