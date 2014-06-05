package name.kan.io;

import com.google.common.io.ByteStreams;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.StreamCorruptedException;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

/**
 * @author kan
 * @since 2014-05-02 19:48
 */
public class ScobsInputStreamTest
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

	@Test
	public void testRead0() throws Exception
	{
		final byte[] actual = decode(new byte[]{-0x01});
		assertArrayEquals(new byte[]{0x00}, actual);
	}

	@Test
	public void testRead1() throws Exception
	{
		final byte[] actual = decode(new byte[]{-0x03, 0x11, 0x22, 0x01, 0x33});
		assertArrayEquals(new byte[]{0x11, 0x22, 0x00, 0x33}, actual);
	}

	@Test
	public void testRead2() throws Exception
	{
		final byte[] actual = decode(new byte[]{-0x02, 0x11, -0x01, -0x01});
		assertArrayEquals(new byte[]{0x11, 0x00, 0x00, 0x00}, actual);
	}

	@Test
	public void testRead3() throws Exception
	{
		final byte[] actual = decode(new byte[]{0x01, 0x01});
		assertArrayEquals(new byte[]{0x01}, actual);
	}

	@Test
	public void testRead4() throws Exception
	{
		final ByteArrayOutputStream input = new ByteArrayOutputStream();
		input.write(125);
		for(int i = 1; i <= 125; ++i)
			input.write(i);
		input.write(125);
		for(int i = 126; i <= 250; ++i)
			input.write(i);
		input.write(5);
		for(int i = 251; i <= 255; ++i)
			input.write(i);
		final byte[] actual = decode(input.toByteArray());
		assertEquals(255, actual.length);
		for(int i = 1; i <= 255; ++i)
			assertEquals("i=" + i, (byte)i, actual[i - 1]);
	}

	@Test
	public void testRead5() throws Exception
	{
		final byte[] actual = decode(new byte[]{-0x01, -0x01});
		assertArrayEquals(new byte[]{0x00, 0x00}, actual);
	}

	private byte[] decode(final byte[] input) throws Exception
	{
		final ScobsInputStream testObject = new ScobsInputStream(new ByteArrayInputStream(input));
		final ByteArrayOutputStream os = new ByteArrayOutputStream();
		ByteStreams.copy(testObject, os);
		return os.toByteArray();
	}

}
