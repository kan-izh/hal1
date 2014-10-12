package name.kan.io;

import java.io.IOException;

/**
 * @author kan
 * @since 2014-06-12 20:55
 */
public class CommunicationException extends IOException
{
	public CommunicationException(final Throwable e)
	{
		super(e);
	}
}
