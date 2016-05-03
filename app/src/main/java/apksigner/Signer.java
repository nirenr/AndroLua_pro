package apksigner;



import apksigner.io.ZioEntry;
import apksigner.io.ZipInput;
import apksigner.io.ZipOutput;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.File;

import java.security.DigestOutputStream;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.PrivateKey;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.Key;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.regex.Pattern;

import javax.crypto.Cipher;
import javax.crypto.EncryptedPrivateKeyInfo;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;



public class Signer
{

    private static final String CERT_SF_NAME = "META-INF/CERT.SF";
    private static final String CERT_RSA_NAME = "META-INF/CERT.RSA";

    // Files matching this pattern are not copied to the output.
    private static Pattern stripPattern =
	Pattern.compile("^META-INF/(.*)[.](SF|RSA|DSA)$");




    public static final String[] DEFAULT_KEYS={
        "platform","testkey"};
    static PrivateKey privateKey;

    static X509Certificate publicKey;
    static byte[] sigBlockTemp;
    private static Signer res=new Signer();

	public Signer()
	{

	}

	public Signer(String name) throws IOException, GeneralSecurityException
	{
		loadKeys(name);
	}

    private static X509Certificate readPublicKey(InputStream input)throws IOException, GeneralSecurityException
	{
        try
		{
            CertificateFactory cf=CertificateFactory.getInstance("X.509");
            return (X509Certificate)cf.generateCertificate(input);
        }
        finally
		{
            input.close();
        }
    }
    /**    * Reads the password from stdin and returns it as a string.    *    * @param keyFile The file containing the private key.  Used to prompt the user.    */
    private static KeySpec decryptPrivateKey(byte[] encryptedPrivateKey, String keyPassword)throws GeneralSecurityException
	{
        EncryptedPrivateKeyInfo epkInfo=null;
        try
		{
            epkInfo = new EncryptedPrivateKeyInfo(encryptedPrivateKey);
        }
        catch (IOException ex)
		{
            return null;

        }
        char[] password=keyPassword.toCharArray();
        SecretKeyFactory skFactory=SecretKeyFactory.getInstance(epkInfo.getAlgName());
        Key key=skFactory.generateSecret(new PBEKeySpec(password));
        Cipher cipher=Cipher.getInstance(epkInfo.getAlgName());
        cipher.init(Cipher.DECRYPT_MODE, key, epkInfo.getAlgParameters());
        try
		{
            return epkInfo.getKeySpec(cipher);
        }
        catch (InvalidKeySpecException ex)
		{
            System.err.println("PrivateKey may be bad.");
            throw ex;
        }
    }
    /** Read a PKCS 8 format private key. */
    private static PrivateKey readPrivateKey(InputStream input)throws IOException, GeneralSecurityException
	{
        try
		{
            byte[] bytes=readBytes(input);

            KeySpec spec=decryptPrivateKey(bytes, "");
            if (spec == null)
			{
                spec = new PKCS8EncodedKeySpec(bytes);
            }
            try
			{
                return KeyFactory.getInstance("RSA").generatePrivate(spec);
            }
            catch (InvalidKeySpecException ex)
			{
                return KeyFactory.getInstance("DSA").generatePrivate(spec);
            }
        }
        finally
		{
            input.close();
        }
    }


    private static byte[] readBytes(InputStream in)throws IOException
	{
        byte[] buf=new byte[1024];
        ByteArrayOutputStream out=new ByteArrayOutputStream();
        int num;
        while ((num = in.read(buf, 0, buf.length)) != -1)
            out.write(buf, 0, num);
        return out.toByteArray();
    }

    private static Manifest addDigestsToManifest(Map<String,ZioEntry> entries)
	throws IOException, GeneralSecurityException 
    {
        Manifest input = null;
        ZioEntry manifestEntry = entries.get(JarFile.MANIFEST_NAME);
        if (manifestEntry != null)
		{
            input = new Manifest();
            input.read(manifestEntry.getInputStream());
        }
        Manifest output = new Manifest();
        Attributes main = output.getMainAttributes();
        if (input != null)
		{
            main.putAll(input.getMainAttributes());
        }
		else
		{
            main.putValue("Manifest-Version", "1.0");
            main.putValue("Created-By", "1.0 (Android SignApk)");
        }

        MessageDigest md = MessageDigest.getInstance("SHA1");
        byte[] buffer = new byte[4096];
        int num;

        // We sort the input entries by name, and add them to the
        // output manifest in sorted order.  We expect that the output
        // map will be deterministic.

        TreeMap<String, ZioEntry> byName = new TreeMap<String, ZioEntry>();
        byName.putAll(entries);

        for (ZioEntry entry: byName.values())
		{
            String name = entry.getName();
            if (!entry.isDirectory() && !name.equals(JarFile.MANIFEST_NAME) &&
				!name.equals(CERT_SF_NAME) && !name.equals(CERT_RSA_NAME) &&
				(stripPattern == null ||
				!stripPattern.matcher(name).matches()))
            {

                InputStream data = entry.getInputStream();
                while ((num = data.read(buffer)) > 0)
				{
                    md.update(buffer, 0, num);
                }

                Attributes attr = null;
                if (input != null)
				{
                    java.util.jar.Attributes inAttr = input.getAttributes(name); 
                    if (inAttr != null) attr = new Attributes(inAttr);
                }
                if (attr == null) attr = new Attributes();
                attr.putValue("SHA1-Digest", Base64.encode(md.digest()));
                output.getEntries().put(name, attr);
            }
        }

        return output;
    }







    /** Write to another stream and also feed it to the Signature object. */


    private static void generateSignatureFile(Manifest manifest, OutputStream out)
	throws IOException, GeneralSecurityException
	{
        out.write(("Signature-Version: 1.0\r\n").getBytes());
        out.write(("Created-By: 1.0 (Android SignApk)\r\n").getBytes());


        // BASE64Encoder base64 = new BASE64Encoder();
        MessageDigest md = MessageDigest.getInstance("SHA1");
        PrintStream print = new PrintStream(
			new DigestOutputStream(new ByteArrayOutputStream(), md),
			true, "UTF-8");

        // Digest of the entire manifest
        manifest.write(print);
        print.flush();

        out.write(("SHA1-Digest-Manifest: " + Base64.encode(md.digest()) + "\r\n\r\n").getBytes());

        Map<String, Attributes> entries = manifest.getEntries();
        for (Map.Entry<String, Attributes> entry : entries.entrySet())
		{
            // Digest of the manifest stanza for this entry.
            String nameEntry = "Name: " + entry.getKey() + "\r\n"; 
            print.print(nameEntry);
            for (Map.Entry<Object, Object> att : entry.getValue().entrySet())
			{
                print.print(att.getKey() + ": " + att.getValue() + "\r\n");
            }
            print.print("\r\n");
            print.flush();

            out.write(nameEntry.getBytes());
            out.write(("SHA1-Digest: " +  Base64.encode(md.digest()) + "\r\n\r\n").getBytes());
        }

    }


    public   void loadKeys(String name)
	throws IOException, GeneralSecurityException
    {


        // load the private key

        privateKey = readPrivateKey(getClass().getResource("/keys/" + name + ".pk8").openStream());

        // load the certificate
        publicKey = readPublicKey(getClass().getResource("/keys/" + name + ".x509.pem").openStream());

        // load the signature block template

        sigBlockTemp = readBytes(getClass().getResource("/keys/" + name + ".sbt").openStream());
    }


    /** Write a .RSA file with a digital signature. */
    private static void writeSignatureBlock(Signature signature, X509Certificate publicKey, OutputStream out)throws IOException, GeneralSecurityException,Exception
	{
        out.write(sigBlockTemp);
        out.write(signature.sign());

    }


    private static void copyFiles(Manifest manifest, Map<String,ZioEntry> input, ZipOutput output, long timestamp)
	throws IOException 
    {
        Map<String, Attributes> entries = manifest.getEntries();
        List<String> names = new ArrayList<String>(entries.keySet());
        Collections.sort(names);
        for (String name : names)
		{
            ZioEntry inEntry = input.get(name);
            inEntry.setTime(timestamp);
            output.write(inEntry);

        }
    }

    public static void sign(String in, String ou) throws IOException, GeneralSecurityException
	{
		if (privateKey == null)
			res.loadKeys("testkey");
		ZipInput input=null;
        ZipOutput zipOut=null;
        try
		{
            input = ZipInput.read(in);

            Map<String,ZioEntry> entries=input.getEntries();
            zipOut = new ZipOutput(new FileOutputStream(ou));


            long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;


            //MANIFEST
            Manifest manifest = addDigestsToManifest(entries);
            ZioEntry ze = new ZioEntry(JarFile.MANIFEST_NAME);
            ze.setTime(timestamp);
            manifest.write(ze.getOutputStream());
            zipOut.write(ze);



            //.SF
            ze = new ZioEntry(CERT_SF_NAME);
            ze.setTime(timestamp);


            Signature signature = Signature.getInstance();
            signature.initSign(privateKey);


            ByteArrayOutputStream out = new ByteArrayOutputStream();
            generateSignatureFile(manifest, out);
            byte[] sfBytes = out.toByteArray();
            ze.getOutputStream().write(sfBytes);
            zipOut.write(ze);
            signature.update(sfBytes);

            //.RSA
            ze = new ZioEntry(CERT_RSA_NAME);
            ze.setTime(timestamp);
            writeSignatureBlock(signature, publicKey, ze.getOutputStream());
            zipOut.write(ze);

            // Everything else
            copyFiles(manifest, entries, zipOut, timestamp);
        }
		catch (Exception ioe)
		{
            ioe.printStackTrace();

        }
		finally
		{
            try
			{
                if (input != null) input.close();
                if (zipOut != null) zipOut.close();
            }
			catch (IOException e)
			{}
        }
    }

    public static void sign(String in, String ou, String name) throws IOException, GeneralSecurityException
	{

		res.loadKeys(name);
		ZipInput input=null;
        ZipOutput zipOut=null;
        try
		{
            input = ZipInput.read(in);

            Map<String,ZioEntry> entries=input.getEntries();
            zipOut = new ZipOutput(new FileOutputStream(ou));


            long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;


            //MANIFEST
            Manifest manifest = addDigestsToManifest(entries);
            ZioEntry ze = new ZioEntry(JarFile.MANIFEST_NAME);
            ze.setTime(timestamp);
            manifest.write(ze.getOutputStream());
            zipOut.write(ze);



            //.SF
            ze = new ZioEntry(CERT_SF_NAME);
            ze.setTime(timestamp);


            Signature signature = Signature.getInstance();
            signature.initSign(privateKey);


            ByteArrayOutputStream out = new ByteArrayOutputStream();
            generateSignatureFile(manifest, out);
            byte[] sfBytes = out.toByteArray();
            ze.getOutputStream().write(sfBytes);
            zipOut.write(ze);
            signature.update(sfBytes);

            //.RSA
            ze = new ZioEntry(CERT_RSA_NAME);
            ze.setTime(timestamp);
            writeSignatureBlock(signature, publicKey, ze.getOutputStream());
            zipOut.write(ze);

            // Everything else
            copyFiles(manifest, entries, zipOut, timestamp);
        }
		catch (Exception ioe)
		{
            ioe.printStackTrace();

        }
		finally
		{
            try
			{
                if (input != null) input.close();
                if (zipOut != null) zipOut.close();
            }
			catch (IOException e)
			{}
        }
    }

    public static void sign(File in, String ou)throws Exception
	{
		if (privateKey == null)
			res.loadKeys("testkey");

        ZipInput input=null;
        ZipOutput zipOut=null;

        input = ZipInput.read(in);

        Map<String,ZioEntry> entries=input.getEntries();
        zipOut = new ZipOutput(new FileOutputStream(ou));


        long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;


        //MANIFEST
        Manifest manifest = addDigestsToManifest(entries);
        ZioEntry ze = new ZioEntry(JarFile.MANIFEST_NAME);
        ze.setTime(timestamp);
        manifest.write(ze.getOutputStream());
        zipOut.write(ze);



        //.SF
        ze = new ZioEntry(CERT_SF_NAME);
        ze.setTime(timestamp);


        Signature signature = Signature.getInstance();
        signature.initSign(privateKey);


        ByteArrayOutputStream out = new ByteArrayOutputStream();
        generateSignatureFile(manifest, out);
        byte[] sfBytes = out.toByteArray();
        ze.getOutputStream().write(sfBytes);
        zipOut.write(ze);
        signature.update(sfBytes);

        //.RSA
        ze = new ZioEntry(CERT_RSA_NAME);
        ze.setTime(timestamp);
        writeSignatureBlock(signature, publicKey, ze.getOutputStream());
        zipOut.write(ze);

        // Everything else
        copyFiles(manifest, entries, zipOut, timestamp);

        input.close();
        zipOut.close();
    }


}

