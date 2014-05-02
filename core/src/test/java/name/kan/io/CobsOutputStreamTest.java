package name.kan.io;

import org.junit.Before;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

/**
 * @author kan
 * @since 2014-04-30 21:19
 */
public class CobsOutputStreamTest
{
	private ByteArrayOutputStream out;
	private CobsOutputStream testObject;

	@Before
	public void setUp() throws Exception
	{

		out = new ByteArrayOutputStream();
		testObject = new CobsOutputStream(out);
	}

	@Test
	public void testWrite0() throws Exception
	{
		testObject.write(new byte[]{0x00});
		assertArrayEquals(new byte[]{0x01, 0x01}, actual());
	}

	@Test
	public void testWrite1() throws Exception
	{
		testObject.write(new byte[]{0x11, 0x22, 0x00, 0x33});
		assertArrayEquals(new byte[]{0x03, 0x11, 0x22, 0x02, 0x33}, actual());
	}

	@Test
	public void testWrite2() throws Exception
	{
		testObject.write(new byte[]{0x11, 0x00, 0x00, 0x00});
		assertArrayEquals(new byte[]{0x02, 0x11, 0x01, 0x01, 0x01}, actual());
	}

	@Test
	public void testWrite3() throws Exception
	{
		for(int i = 1; i <= 255; ++i)
			testObject.write(i);
		final byte[] actual = actual();
		assertEquals(257, actual.length);
		assertEquals((byte)0xff, actual[0]);
		assertEquals((byte)0xfe, actual[254]);
		assertEquals((byte)0x02, actual[255]);
		assertEquals((byte)0xff, actual[256]);
	}

	@Test
	public void testWrite4() throws Exception
	{
		testObject.write(new byte[]{0x01});
		assertArrayEquals(new byte[]{0x02, 0x01}, actual());
	}

	@Test
	public void testWrite5() throws Exception
	{
		testObject.write(new byte[]{0x00, 0x00});
		assertArrayEquals(new byte[]{0x01, 0x01, 0x01}, actual());
	}

	private byte[] actual() throws IOException
	{
		testObject.flush();
		return out.toByteArray();
	}
}
