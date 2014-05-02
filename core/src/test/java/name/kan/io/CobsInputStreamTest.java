package name.kan.io;

import com.google.common.io.ByteStreams;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StreamCorruptedException;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

/**
 * @author kan
 * @since 2014-05-02 19:48
 */
public class CobsInputStreamTest
{
	@Test(expected = StreamCorruptedException.class)
	public void testMalformed0() throws Exception
	{
		decode(new byte[]{0});
	}

	@Test(expected = StreamCorruptedException.class)
	public void testMalformed1() throws Exception
	{
		decode(new byte[]{0x01, 0});
	}

	@Test(expected = StreamCorruptedException.class)
	public void testMalformed2() throws Exception
	{
		decode(new byte[]{0x02, 0});
	}

	@Test(expected = IOException.class)
	public void testMalformed3() throws Exception
	{
		final ByteArrayOutputStream input = new ByteArrayOutputStream();
		input.write(0xff);
		for(int i = 1; i <= 254; ++i)
			input.write(i);
		input.write(0x01);
		input.write(0xff);
		decode(input.toByteArray());
	}

	@Test
	public void testRead0() throws Exception
	{
		final byte[] actual = decode(new byte[]{0x01, 0x01});
		assertArrayEquals(new byte[]{0x00}, actual);
	}

	@Test
	public void testRead1() throws Exception
	{
		final byte[] actual = decode(new byte[]{0x03, 0x11, 0x22, 0x02, 0x33});
		assertArrayEquals(new byte[]{0x11, 0x22, 0x00, 0x33}, actual);
	}

	@Test
	public void testRead2() throws Exception
	{
		final byte[] actual = decode(new byte[]{0x02, 0x11, 0x01, 0x01, 0x01});
		assertArrayEquals(new byte[]{0x11, 0x00, 0x00, 0x00}, actual);
	}

	@Test
	public void testRead3() throws Exception
	{
		final byte[] actual = decode(new byte[]{0x02, 0x01});
		assertArrayEquals(new byte[]{0x01}, actual);
	}

	@Test
	public void testRead4() throws Exception
	{
		final ByteArrayOutputStream input = new ByteArrayOutputStream();
		input.write(0xff);
		for(int i = 1; i <= 254; ++i)
			input.write(i);
		input.write(0x02);
		input.write(0xff);
		final byte[] actual = decode(input.toByteArray());
		assertEquals(255, actual.length);
		for(int i = 1; i <= 255; ++i)
			assertEquals("i=" + i, (byte)i, actual[i - 1]);
	}

	private byte[] decode(final byte[] input) throws Exception
	{
		final CobsInputStream testObject = new CobsInputStream(new ByteArrayInputStream(input));
		final ByteArrayOutputStream os = new ByteArrayOutputStream();
		ByteStreams.copy(testObject, os);
		return os.toByteArray();
	}

}
