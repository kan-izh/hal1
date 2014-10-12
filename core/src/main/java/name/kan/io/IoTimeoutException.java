package name.kan.io;

/**
 * @author kan
 * @since 2014-06-12 20:49
 */
public class IoTimeoutException extends CommunicationException
{
	public IoTimeoutException(final Throwable e)
	{
		super(e);
	}
}
