package mao.res;


import java.io.*;
import java.util.List;
import java.util.ArrayList;

import mao.util.LEDataInputStream;
import mao.util.LEDataOutputStream;

public class AXmlDecoder{
    private static final int AXML_CHUNK_TYPE=0x00080003;
    public StringBlock mTableStrings;
    private final LEDataInputStream mIn;
    ByteArrayOutputStream byteOut=new ByteArrayOutputStream();

    private void readStrings(List<String> list) throws IOException{
        int type=mIn.readInt();
        checkChunk(type,AXML_CHUNK_TYPE);
        mIn.readInt();//Chunk size
        mTableStrings = StringBlock.read(this.mIn);
        byte[] buf=new byte[2048];
        int num;
        while((num=mIn.read(buf,0,2048))!=-1)
            byteOut.write(buf,0,num);
		mTableStrings.getStrings(list);
	}

    private AXmlDecoder(LEDataInputStream in){
        this.mIn=in;
    }

/*
    public static AXmlDecoder read(InputStream input)throws IOException{
        AXmlDecoder axml=new AXmlDecoder(new LEDataInputStream(input));
        axml.readStrings();
        return axml;
    }*/
	
    public static AXmlDecoder read(List<String> list,InputStream input)throws IOException{
        AXmlDecoder axml=new AXmlDecoder(new LEDataInputStream(input));
        axml.readStrings(list);
		return axml;
    }
	
    public void write(List<String> list,OutputStream out)throws IOException{
        write(list,new LEDataOutputStream(out));
    }


    public void write(List<String> list,LEDataOutputStream out)throws IOException{
        ByteArrayOutputStream baos=new ByteArrayOutputStream();
        LEDataOutputStream buf=new LEDataOutputStream(baos);
        mTableStrings.write(list,buf);
        buf.writeFully(byteOut.toByteArray());
        //write out
        out.writeInt(AXML_CHUNK_TYPE);
        out.writeInt(baos.size()+8);
        out.writeFully(baos.toByteArray());
    }


/*
    public static void main(String[] args)throws IOException{

        FileInputStream file=new FileInputStream("term.xml");
        AXmlDecoder axml=read(file);
        List<String> list=new ArrayList<String>();
        axml.mTableStrings.getStrings(list);
        for(int i=0;i<list.size();i++)
            System.out.println(i+" "+list.get(i));
        FileOutputStream out =new FileOutputStream("test.xml");
        axml.write(list,out);

    }
*/


    private void checkChunk(int type,int expectedType) throws IOException {
        if (type != expectedType)
            throw new IOException(String.format("Invalid chunk type: expected=0x%08x, got=0x%08x", new Object[] { Integer.valueOf(expectedType), Short.valueOf((short)type) }));
    }

}
