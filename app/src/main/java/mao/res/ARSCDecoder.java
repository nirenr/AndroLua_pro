
package mao.res;


import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;


import mao.util.LEDataInputStream;
import mao.util.LEDataOutputStream;

public class ARSCDecoder
{
    private final LEDataInputStream mIn;
    public StringBlock mTableStrings;
    private StringBlock mTypeNames;
    private StringBlock mSpecNames;
    private int mResId;
    int packageCount;


    byte[] buf;
    String name;
    int id;



    public static final int ARSC_CHUNK_TYPE=0x000c0002;
    public static final int CHECK_PACKAGE=512;
    ByteArrayOutputStream byteOut=new ByteArrayOutputStream();


    private ARSCDecoder(InputStream arscStream)
    {
        this.mIn = new LEDataInputStream(arscStream);
    }

    private void readTable(List<String> list) throws IOException{
        int type=mIn.readInt();
        checkChunk(type,ARSC_CHUNK_TYPE);
        mIn.readInt();//chunk size

        packageCount = this.mIn.readInt();

        this.mTableStrings = StringBlock.read(this.mIn);
        readPackage();
		this.mTableStrings.getStrings(list);
    }

    public void write(List<String> list,OutputStream out)throws IOException{
        write(list,new LEDataOutputStream(out));
    }

    public void write(List<String> list,LEDataOutputStream out)throws IOException{


        ByteArrayOutputStream baos=new ByteArrayOutputStream();
        LEDataOutputStream buf=new LEDataOutputStream(baos);
        buf.writeInt(packageCount);
        mTableStrings.write(list,buf);
        writePackage(buf);
//write to out
        out.writeInt(ARSC_CHUNK_TYPE);
        out.writeInt(baos.size()+8);
        out.writeFully(baos.toByteArray());
    }


    public static ARSCDecoder read(List<String> list,InputStream in)throws IOException{

        ARSCDecoder arsc=new ARSCDecoder(in);

        arsc.readTable(list);
        return arsc;
    }

 /*   public static void main(String[] args)throws Exception{
        if(args.length>1){
        FileInputStream file=new FileInputStream(args[0].trim());
        ARSCDecoder arsc=ARSCDecoder.read(file);
        StringBlock sb=arsc.mTableStrings;
        List<String> list=new ArrayList<String>();
        sb.getStrings(list);
              for(int i=0;i<list.size();i++)
                System.out.println(i+" "+list.get(i));
        ByteArrayOutputStream out=new ByteArrayOutputStream();
        arsc.write(list,new LEDataOutputStream(out));
        FileOutputStream outFile=new FileOutputStream(args[1].trim());
        outFile.write(out.toByteArray());
        outFile.close();
        }else{
            System.out.println("<input> <output>" );
        }

    }
*/

    
    public void writePackage( LEDataOutputStream out)throws IOException{
        /*
        out.writeShort((short)CHECK_PACKAGE);
        out.writeShort((short)package_unknow1);
        out.writeInt(package_chunksize);
        out.writeInt(id);
        //int start=out.size();
        out.writeNulEndedString(name,128,true);
       // System.out.println("len "+(out.size()-start));
        out.writeFully(buf);
        mTypeNames.write(out);
        mSpecNames.write(out);
        */
        out.writeFully(byteOut.toByteArray());
    }


    private void readPackage() throws IOException {
        /*
        package_type=mIn.readShort();
        checkChunk(package_type,CHECK_PACKAGE);
        package_unknow1=mIn.readShort();
        package_chunksize=mIn.readInt();
        id = mIn.readInt();
        name = mIn.readNulEndedString(128, true);
//        System.out.println("read "+mIn.size());
        buf=new byte[16];
     //   this.mIn.skipInt();
       // this.mIn.skipInt();
     //   this.mIn.skipInt();
      //  this.mIn.skipInt();
        mIn.readFully(buf);

        mTypeNames = StringBlock.read(mIn);
        mSpecNames = StringBlock.read(mIn);
*/
        byte[] buf=new byte[2048];

        int num;
        while((num=mIn.read(buf,0,2048))!=-1)
            byteOut.write(buf,0,num);

    }

    private void checkChunk(int type,int expectedType) throws IOException {
        if (type != expectedType)
            throw new IOException(String.format("Invalid chunk type: expected=0x%08x, got=0x%08x", new Object[] { Integer.valueOf(expectedType), Short.valueOf((short)type) }));
    }

}
