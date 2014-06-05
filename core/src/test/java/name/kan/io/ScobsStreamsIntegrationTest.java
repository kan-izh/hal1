package name.kan.io;

import com.google.common.io.ByteStreams;
import com.google.common.primitives.Bytes;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Arrays;
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
public class ScobsStreamsIntegrationTest
{

	private static final Logger LOG = LoggerFactory.getLogger(ScobsStreamsIntegrationTest.class);

	private void testRandom() throws Exception
	{
		final Random random = new Random();
		final byte[] expected = new byte[random.nextInt(400)];
		random.nextBytes(expected);


		try
		{
			byte[] coded = encode(expected);
			for(byte b : coded)
			{
				assertThat(b, not(equalTo((byte)0)));
			}
			final byte[] actual = decode(coded);
			assertArrayEquals(expected, actual);
		} catch(Exception e)
		{
			LOG.info("Input: {}", Bytes.asList(expected));
			throw e;
		}
	}

	@Test
	public void testRandomRepeated() throws Exception
	{
		for(int i=0; i<1000; ++i)
			testRandom();
	}

	@Test
	public void testEmpty() throws Exception
	{
		assertEquals(0, decode(encode(new byte[0])).length);
	}

	@Test
	public void testBug1() throws Exception
	{
		final byte[] expected = new byte[254];
		Arrays.fill(expected, (byte)42);
		final byte[] coded = encode(expected);
		final byte[] actual = decode(coded);
		assertArrayEquals(expected, actual);
	}

	@Test
	public void testBug2() throws Exception
	{
		final byte zero = 0;
		final byte[] expected = new byte[456];
		Arrays.fill(expected, (byte)1);
		expected[198] = 0;
		expected[453] = 0;
		final byte[] coded = encode(expected);
		final byte[] actual = decode(coded);
		assertArrayEquals(expected, actual);
	}

	private byte[] decode(final byte[] coded) throws IOException
	{
		final ScobsInputStream is = new ScobsInputStream(new ByteArrayInputStream(coded));
		final ByteArrayOutputStream actualOs = new ByteArrayOutputStream();
		ByteStreams.copy(is, actualOs);
		actualOs.flush();
		return actualOs.toByteArray();
	}

	private byte[] encode(final byte[] expected) throws IOException
	{
		final ByteArrayOutputStream codedOs = new ByteArrayOutputStream();
		final ScobsOutputStream os = new ScobsOutputStream(codedOs);
		ByteStreams.copy(new ByteArrayInputStream(expected), os);
		os.flush();
		return codedOs.toByteArray();
	}
}
