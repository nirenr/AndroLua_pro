package mao.res;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.io.ByteArrayOutputStream;

import mao.util.LEDataInputStream;
import mao.util.LEDataOutputStream;

import mao.util.Utf8Utils;

public class StringBlock
{
    private int[] m_stringOffsets;
    byte[] m_strings;
    private int[] m_styleOffsets;
    private int[] m_styles;
    private boolean m_isUTF8;
    private int styleOffsetCount;
    private int stylesOffset;
    private int stringsOffset;
    private int flags;
    private int chunkSize;


    private static final CharsetDecoder UTF16LE_DECODER = Charset.forName("UTF-16LE").newDecoder();
    private static final CharsetEncoder UTF16LE_ENCODER = Charset.forName("UTF-16LE").newEncoder();

    private static final CharsetDecoder UTF8_DECODER = Charset.forName("UTF-8").newDecoder();
    private static final CharsetEncoder UTF8_ENCODER = Charset.forName("UTF-8").newEncoder();

    public static final int CHUNK_STRINGBLOCK=1835009;
    public static final int IS_UTF8=0x100;

    public static StringBlock read(LEDataInputStream reader)
        throws IOException
    {
        reader.skipCheckInt(CHUNK_STRINGBLOCK);
        StringBlock block = new StringBlock();

        int chunkSize =block.chunkSize=reader.readInt();
                System.out.println("chunkSize "+chunkSize);

        int stringCount = reader.readInt();
              System.out.println("stringCount "+stringCount);

        int styleOffsetCount =block.styleOffsetCount= reader.readInt();
            System.out.println("styleOffsetCount "+styleOffsetCount);

        int flags = block.flags = reader.readInt();

        int stringsOffset =block.stringsOffset= reader.readInt();
          System.out.println("stringsOffset "+stringsOffset);

        int stylesOffset =block.stylesOffset= reader.readInt();
        System.out.println("stylesOffset "+stylesOffset);

        block.m_isUTF8 = ((flags & IS_UTF8) != 0);
        block.m_stringOffsets = reader.readIntArray(stringCount);
        if (styleOffsetCount != 0) {
            block.m_styleOffsets = reader.readIntArray(styleOffsetCount);
        }

        int size = (stylesOffset == 0 ? chunkSize : stylesOffset) - stringsOffset;
        if (size % 4 != 0) {
            throw new IOException("String data size is not multiple of 4 (" + size + ").");
        }
        block.m_strings = new byte[size];
        reader.readFully(block.m_strings);

        //        System.out.println("m_strings_size "+size);

        if (stylesOffset != 0) {
            size = chunkSize - stylesOffset;
            if (size % 4 != 0) {
                throw new IOException("Style data size is not multiple of 4 (" + size + ").");
            }
            block.m_styles = reader.readIntArray(size / 4);
            System.out.println("m_styles_size "+size);
        }
        System.out.println();

        return block;
    }

    public void getStrings(List<String> list){
        int size=getSize();
        for(int i=0;i<size;i++)
            list.add(Utf8Utils.escapeString(getString(i)));
    }


    public void write(LEDataOutputStream out)throws IOException{
        List<String> list=new ArrayList<String>(getSize());
        getStrings(list);
        write(list,out);
    }


    public void write(List<String> list,LEDataOutputStream out)throws IOException{


        ByteArrayOutputStream outBuf=new ByteArrayOutputStream();
        LEDataOutputStream led=new LEDataOutputStream(outBuf);
        // stringCount
        int size=list.size();

        //m_stringOffsets
        int[] offset=new int[size];
        int len=0;

        //m_strings
        ByteArrayOutputStream bOut=new ByteArrayOutputStream();
        LEDataOutputStream mStrings=new LEDataOutputStream(bOut);
        for(int i=0;i<size;i++){
            offset[i]=len;
            String var=Utf8Utils.escapeSequence(list.get(i));
            char[] charbuf=var.toCharArray();
            mStrings.writeShort((short)charbuf.length);
            mStrings.writeCharArray(charbuf);
            mStrings.writeShort((short)0);
            len+=charbuf.length*2+4;
        }

        int m_strings_size=bOut.size();
        int size_mod=m_strings_size%4;//m_strings_size%4
        //padding 0
        if(size_mod !=0){
            for(int i=0;i<4-size_mod;i++){
                bOut.write(0);
            }
            m_strings_size+=4-size_mod;
        }
        byte[] m_strings=bOut.toByteArray();



        System.out.println("string chunk size: "+chunkSize);

        led.writeInt(size);
        led.writeInt(styleOffsetCount);
        led.writeInt(flags);

        led.writeInt(stringsOffset);
        led.writeInt(stylesOffset);

        led.writeIntArray(offset);
        if(styleOffsetCount!=0){
            System.out.println("write stylesOffset");
            led.writeIntArray(m_styleOffsets);
        }

        led.writeFully(m_strings);

        if(m_styles!=null){
            System.out.println("write m_styles");
            led.writeIntArray(m_styles);
        }
        out.writeInt(CHUNK_STRINGBLOCK);

        byte[] b=outBuf.toByteArray();
        out.writeInt(b.length+8);
        out.writeFully(b);
    }


    public int getChunkSize(){
        return chunkSize;
    }

    public String getString(int index)
    {
        if ((index < 0) || (this.m_stringOffsets == null) || (index >= this.m_stringOffsets.length))
        {
            return null;
        }
        int offset = this.m_stringOffsets[index];
        int length;
        if (!this.m_isUTF8) {
            length = getShort(this.m_strings, offset) * 2;
            offset += 2;
        } else {
            offset += getVarint(this.m_strings, offset)[1];
            int[] varint = getVarint(this.m_strings, offset);
            offset += varint[1];
            length = varint[0];
        }
        return decodeString(offset, length);
    }


    public int getSize(){
        return m_stringOffsets!=null?m_stringOffsets.length:0;
    }


    public String getHTML(int index)
    {
        String raw = getString(index);
        if (raw == null) {
            return raw;
        }
        int[] style = getStyle(index);
        if (style == null) {
            return raw;
        }
        StringBuilder html = new StringBuilder(raw.length() + 32);
        int[] opened = new int[style.length / 3];
        int offset = 0; int depth = 0;
        while (true) {
            int i = -1;
            int j=0;
            for (; j != style.length; j += 3) {
                if (style[(j + 1)] == -1) {
                    continue;
                }
                if ((i == -1) || (style[(i + 1)] > style[(j + 1)])) {
                    i = j;
                }
            }
            int start = i != -1 ? style[(i + 1)] : raw.length();
            for (j = depth - 1; j >= 0; j--) {
                int last = opened[j];
                int end = style[(last + 2)];
                if (end >= start) {
                    break;
                }
                if (offset <= end) {
                    html.append(raw.substring(offset, end + 1));

                    offset = end + 1;
                }
                outputStyleTag(getString(style[last]), html, true);
            }
            depth = j + 1;
            if (offset < start) {
                html.append(raw.substring(offset, start));

                offset = start;
            }
            if (i == -1) {
                break;
            }
            outputStyleTag(getString(style[i]), html, false);
            style[(i + 1)] = -1;
            opened[(depth++)] = i;
        }
        return html.toString();
    }


    private void outputStyleTag(String tag, StringBuilder builder, boolean close)
    {
        builder.append('<');
        if (close) {
            builder.append('/');
        }

        int pos = tag.indexOf(';');
        if (pos == -1) {
            builder.append(tag);
        } else {
            builder.append(tag.substring(0, pos));
            if (!close) {
                boolean loop = true;
                while (loop) {
                    int pos2 = tag.indexOf('=', pos + 1);
                    builder.append(' ').append(tag.substring(pos + 1, pos2)).append("=\"");

                    pos = tag.indexOf(';', pos2 + 1);
                    String val;
                    if (pos != -1) {
                        val = tag.substring(pos2 + 1, pos);
                    } else {
                        loop = false;
                        val = tag.substring(pos2 + 1);
                    }

                    builder.append(val).append('"');
                }
            }
        }

        builder.append('>');
    }

    private int[] getStyle(int index)
    {
        if ((this.m_styleOffsets == null) || (this.m_styles == null) || (index >= this.m_styleOffsets.length))
        {
            return null;
        }
        int offset = this.m_styleOffsets[index] / 4;

        int count = 0;
        for (int i = offset; (i < this.m_styles.length) && 
                (this.m_styles[i] != -1); i++)
        {
            count++;
        }
        if ((count == 0) || (count % 3 != 0)) {
            return null;
        }
        int[] style = new int[count];

        int i = offset; for (int j = 0; (i < this.m_styles.length) && 
                (this.m_styles[i] != -1); )
        {
            style[(j++)] = this.m_styles[(i++)];
        }
        return style;
    }

    private String decodeString(int offset, int length) {
        try {
            return (this.m_isUTF8 ? UTF8_DECODER : UTF16LE_DECODER).decode(ByteBuffer.wrap(this.m_strings, offset, length)).toString();
        }
        catch (CharacterCodingException ex) {
        }return null;
    }

    private static final int getShort(byte[] array, int offset)
    {
        return (array[offset + 1] & 0xFF) << 8 | array[offset] & 0xFF;
    }

    private static final int[] getVarint(byte[] array, int offset)
    {
        int val = array[offset];
        boolean more = (val & 0x80) != 0;
        val &= 127;

        if (!more) {
            return new int[] { val, 1 };
        }
        return new int[] { val << 8 | array[(offset + 1)] & 0xFF, 2 };
    }
}
