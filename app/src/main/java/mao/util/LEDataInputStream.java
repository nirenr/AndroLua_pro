package mao.util;

import java.io.DataInput;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

public final class LEDataInputStream
  implements DataInput
{
  protected final DataInputStream dis;
  protected final InputStream is;
  protected final byte[] work;
  int s,end;

  public LEDataInputStream(InputStream in)
  {
    is = in;
    dis = new DataInputStream(in);
    work = new byte[8];
  }


  public int size(){
      return end;
  }

  public final boolean readBoolean()
    throws IOException
  {
    return dis.readBoolean();
  }

  public final byte readByte()
    throws IOException
  {
    return dis.readByte();
  }

  public final char readChar()
    throws IOException
  {
    dis.readFully(work, 0, 2);
    return (char)((work[1] & 0xFF) << 8 | work[0] & 0xFF);
  }

  public final double readDouble()
    throws IOException
  {
    return Double.longBitsToDouble(readLong());
  }

  public int[] readIntArray(int length) throws IOException {
    int[] array = new int[length];
    for (int i = 0; i < length; i++) {
      array[i] = readInt();
    }
    return array;
  }
  public void skipInt() throws IOException {
    skipBytes(4);
  }

  public void skipCheckInt(int expected) throws IOException {
    int got = readInt();
    if (got != expected)
      throw new IOException(String.format("Expected: 0x%08x, got: 0x%08x", new Object[] { Integer.valueOf(expected), Integer.valueOf(got) }));
  }

  public void skipCheckShort(short expected)
    throws IOException
  {
    short got = readShort();
    if (got != expected)
      throw new IOException(String.format("Expected: 0x%08x, got: 0x%08x", new Object[] { Short.valueOf(expected), Short.valueOf(got) }));
  }

  public void skipCheckByte(byte expected)
    throws IOException
  {
    byte got = readByte();
    if (got != expected)
      throw new IOException(String.format("Expected: 0x%08x, got: 0x%08x", new Object[] { Byte.valueOf(expected), Byte.valueOf(got) }));
  }

  public String readNulEndedString(int length, boolean fixed)
    throws IOException
  {
    StringBuilder string = new StringBuilder(16);
    while (length-- != 0) {
      short ch = readShort();
      end+=2;
      if (ch == 0) {
        break;
      }
      string.append((char)ch);
    }
    if (fixed) {
      skipBytes(length * 2);
      end+=length*2;
    }



    return string.toString();
  }


  public int read(byte[] b,int a,int len)throws IOException{
      return dis.read(b,a,len);
  }



  public final float readFloat()
    throws IOException
  {
    return Float.intBitsToFloat(readInt());
  }

  public final void readFully(byte[] ba)
    throws IOException
  {
    dis.readFully(ba, 0, ba.length);
  }

  public final void readFully(byte[] ba, int off, int len)
    throws IOException
  {
    dis.readFully(ba, off, len);
  }

  public final int readInt()
    throws IOException
  {
    dis.readFully(work, 0, 4);
    return work[3] << 24 | (work[2] & 0xFF) << 16 | (work[1] & 0xFF) << 8 | work[0] & 0xFF;
  }

  /** @deprecated */
  public final String readLine()
    throws IOException
  {
    return dis.readLine();
  }

  public final long readLong()
    throws IOException
  {
    dis.readFully(work, 0, 8);
    return work[7] << 56 | (work[6] & 0xFF) << 48 | (work[5] & 0xFF) << 40 | (work[4] & 0xFF) << 32 | (work[3] & 0xFF) << 24 | (work[2] & 0xFF) << 16 | (work[1] & 0xFF) << 8 | work[0] & 0xFF;
  }

  public final short readShort()
    throws IOException
  {
    dis.readFully(work, 0, 2);
    return (short)((work[1] & 0xFF) << 8 | work[0] & 0xFF);
  }

  public final String readUTF()
    throws IOException
  {
    return dis.readUTF();
  }

  public final int readUnsignedByte()
    throws IOException
  {
    return dis.readUnsignedByte();
  }

  public final int readUnsignedShort()
    throws IOException
  {
    dis.readFully(work, 0, 2);
    return (work[1] & 0xFF) << 8 | work[0] & 0xFF;
  }

  public final int skipBytes(int n)
    throws IOException
  {
    return dis.skipBytes(n);
  }
}
