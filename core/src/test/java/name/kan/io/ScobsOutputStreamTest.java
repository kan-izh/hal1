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
public class ScobsOutputStreamTest
{
	private ByteArrayOutputStream out;
	private ScobsOutputStream testObject;

	@Before
	public void setUp() throws Exception
	{

		out = new ByteArrayOutputStream();
		testObject = new ScobsOutputStream(out);
	}

	@Test
	public void testWrite0() throws Exception
	{
		testObject.write(new byte[]{0x00});
		assertArrayEquals(new byte[]{-0x01}, actual());
	}

	@Test
	public void testWrite1() throws Exception
	{
		testObject.write(new byte[]{0x11, 0x22, 0x00, 0x33});
		assertArrayEquals(new byte[]{-0x03, 0x11, 0x22, 0x01, 0x33}, actual());
	}

	@Test
	public void testWrite2() throws Exception
	{
		testObject.write(new byte[]{0x11, 0x00, 0x00, 0x00});
		assertArrayEquals(new byte[]{-0x02, 0x11, -0x01, -0x01}, actual());
	}

	@Test
	public void testWrite3() throws Exception
	{
		for(int i = 1; i <= 255; ++i)
			testObject.write(i);
		final byte[] actual = actual();
		assertEquals(258, actual.length);
		assertEquals((byte)125, actual[0]);
		assertEquals((byte)1, actual[1]);
		assertEquals((byte)2, actual[2]);
		assertEquals((byte)124, actual[124]);
		assertEquals((byte)125, actual[125]);
		assertEquals((byte)125, actual[126]);
		assertEquals((byte)126, actual[127]);
		assertEquals((byte)250, actual[251]);
		assertEquals((byte)5, actual[252]);
		assertEquals((byte)251, actual[253]);
		assertEquals((byte)252, actual[254]);
		assertEquals((byte)253, actual[255]);
		assertEquals((byte)255, actual[257]);
	}

	@Test
	public void testWrite4() throws Exception
	{
		testObject.write(new byte[]{0x01});
		assertArrayEquals(new byte[]{0x01, 0x01}, actual());
	}

	@Test
	public void testWrite5() throws Exception
	{
		testObject.write(new byte[]{0x00, 0x00});
		assertArrayEquals(new byte[]{-0x01, -0x01}, actual());
	}

	private byte[] actual() throws IOException
	{
		testObject.close();
		return out.toByteArray();
	}
}
