package com.androlua.util;


import com.androlua.LuaApplication;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.security.Key;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;

import javax.crypto.Cipher;

public class RSASecurity {
    /** 指定加密算法为RSA */
    private static final String ALGORITHM = "RSA";
    /** 密钥长度，用来初始化 */
    private static final int KEY_SIZE = 1024;
    /** 指定公钥存放文件 */
    private static String PUBLIC_KEY_FILE = LuaApplication.getInstance().getLuaExtDir("PublicKey");
    /** 指定私钥存放文件 */
    private static String PRIVATE_KEY_FILE = LuaApplication.getInstance().getLuaExtDir("PrivateKey");

    /**
     * 生成密钥对
     * @throws Exception
     */
    public static void generateKeyPair() throws Exception {

//      /** RSA算法要求有一个可信任的随机数源 */
      SecureRandom secureRandom = new SecureRandom();

		/** 为RSA算法创建一个KeyPairGenerator对象 */
		KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(ALGORITHM);

		/** 利用上面的随机数据源初始化这个KeyPairGenerator对象 */
	    keyPairGenerator.initialize(KEY_SIZE, secureRandom);
		keyPairGenerator.initialize(KEY_SIZE);

		/** 生成密匙对*/
		KeyPair keyPair = keyPairGenerator.generateKeyPair();

		/** 得到公钥*/
		Key publicKey = keyPair.getPublic();

		/** 得到私钥*/
		Key privateKey = keyPair.getPrivate();

		ObjectOutputStream oos1 = null;
		ObjectOutputStream oos2 = null;
		try {
			/** 用对象流将生成的密钥写入文件 */
			oos1 = new ObjectOutputStream(new FileOutputStream(PUBLIC_KEY_FILE));
			oos2 = new ObjectOutputStream(new FileOutputStream(PRIVATE_KEY_FILE));
			oos1.writeObject(publicKey);
			oos2.writeObject(privateKey);
		} catch (Exception e) {
			throw e;
		}
		finally{
			/** 清空缓存，关闭文件输出流 */
			oos1.close();
			oos2.close();
		}
    }

    /**
     * 加密方法
     * @param source 源数据
     * @return
     * @throws Exception
     */
    public static byte[] encrypt(String source) throws Exception {
		generateKeyPair();
		Key publicKey;
		ObjectInputStream ois = null;
		try {
		
			/** 将文件中的公钥对象读出 */
			ois = new ObjectInputStream(new FileInputStream(
											PUBLIC_KEY_FILE));
			publicKey = (Key) ois.readObject();
		} catch (Exception e) {
			throw e;
		}
		finally{
			ois.close();
		}

		/** 得到Cipher对象来实现对源数据的RSA加密 */
		Cipher cipher = Cipher.getInstance(ALGORITHM);
		cipher.init(Cipher.ENCRYPT_MODE, publicKey);
		byte[] b = source.getBytes();
		/** 执行加密操作*/
		byte[] b1 = cipher.doFinal(b);
		return b1;
    }

    /**
     * 解密算法
     * @param cryptograph    密文
     * @return
     * @throws Exception
     */
    public static byte[] decrypt(byte[] cryptograph) throws Exception {
		Key privateKey;
		ObjectInputStream ois = null;
		try {
			/** 将文件中的私钥对象读出 */
			ois = new ObjectInputStream(new FileInputStream(
											PRIVATE_KEY_FILE));
			privateKey = (Key) ois.readObject();
		} catch (Exception e) {
			throw e;
		}
		finally{
			ois.close();
		}

		/** 得到Cipher对象对已用公钥加密的数据进行RSA解密 */
		Cipher cipher = Cipher.getInstance(ALGORITHM);
		cipher.init(Cipher.DECRYPT_MODE, privateKey);
		
		/** 执行解密操作*/
		byte[] b = cipher.doFinal(cryptograph);
		return b;
    }

    
}
