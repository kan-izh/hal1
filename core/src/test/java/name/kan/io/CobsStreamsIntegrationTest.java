package name.kan.io;

import com.google.common.io.ByteStreams;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Random;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;

/**
 * @author kan
 * @since 2014-05-02 21:45
 */
public class CobsStreamsIntegrationTest
{
	private void testRandom() throws Exception
	{
		final Random random = new Random();
		final byte[] expected = new byte[random.nextInt(1000)];
		random.nextBytes(expected);
		byte[] coded = encode(expected);
		for(byte b : coded)
		{
			assertThat(b, not(equalTo((byte)0)));
		}
		final byte[] actual = decode(coded);
		assertArrayEquals(expected, actual);
	}

	@Test
	public void testRandomRepeated() throws Exception
	{
		for(int i=0; i<100; ++i)
			testRandom();
	}

	@Test
	public void testEmpty() throws Exception
	{
		assertEquals(0, decode(encode(new byte[0])).length);
	}

	private byte[] decode(final byte[] coded) throws IOException
	{
		final CobsInputStream is = new CobsInputStream(new ByteArrayInputStream(coded));
		final ByteArrayOutputStream actualOs = new ByteArrayOutputStream();
		ByteStreams.copy(is, actualOs);
		actualOs.flush();
		return actualOs.toByteArray();
	}

	private byte[] encode(final byte[] expected) throws IOException
	{
		final ByteArrayOutputStream codedOs = new ByteArrayOutputStream();
		final CobsOutputStream os = new CobsOutputStream(codedOs);
		ByteStreams.copy(new ByteArrayInputStream(expected), os);
		os.flush();
		return codedOs.toByteArray();
	}
}
