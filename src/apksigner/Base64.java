package apksigner;

import java.io.IOException;


public class Base64 {
    private Base64() {
    }


    private final static char[] ALPHABET = {
	//       0    1    2    3    4    5    6    7
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',  // 0
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',  // 1
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',  // 2
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',  // 3
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',  // 4
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',  // 5
		'w', 'x', 'y', 'z', '0', '1', '2', '3',  // 6
		'4', '5', '6', '7', '8', '9', '+', '/'  // 7
	};

    private static int valueDecoding[] = new int[128];

    static {
	for (int i = 0; i < valueDecoding.length; i++) {
	    valueDecoding[i] = -1;
	}

	for (int i = 0; i < ALPHABET.length; i++) {
	    valueDecoding[ALPHABET[i]] = i;
	}
    }

    public static String encode(byte[] data, int offset, int length) {
        int i;
        int encodedLen;
        char[] encoded;

        // 4 chars for 3 bytes, run input up to a multiple of 3
        encodedLen = (length + 2) / 3 * 4;
        encoded = new char [encodedLen];

        for (i = 0, encodedLen = 0; encodedLen < encoded.length;
             i += 3, encodedLen += 4) {
            encodeQuantum(data, offset + i, length - i, encoded, encodedLen);
        }
               
        return new String(encoded);
    }
    public static String encode(byte[] data){
    	return encode(data,0,data.length);
    }

    private static void encodeQuantum(byte in[], int inOffset, int len,
                                      char out[], int outOffset) {
	byte a = 0, b = 0, c = 0;

        a = in[inOffset];
        out[outOffset] = ALPHABET[(a >>> 2) & 0x3F];

        if (len > 2) {
            b = in[inOffset + 1];
            c = in[inOffset + 2];
            out[outOffset + 1] = ALPHABET[((a << 4) & 0x30) +
                                         ((b >>> 4) & 0xf)];
	    out[outOffset + 2] = ALPHABET[((b << 2) & 0x3c) +
                                          ((c >>> 6) & 0x3)];
	    out[outOffset + 3] = ALPHABET[c & 0x3F];
        } else if (len > 1) {
            b = in[inOffset + 1];
            out[outOffset + 1] = ALPHABET[((a << 4) & 0x30) +
                                         ((b >>> 4) & 0xf)];
	    out[outOffset + 2] =  ALPHABET[((b << 2) & 0x3c) +
                                          ((c >>> 6) & 0x3)];
	    out[outOffset + 3] = '=';
        } else {
            out[outOffset + 1] = ALPHABET[((a << 4) & 0x30) +
                                         ((b >>> 4) & 0xf)];
	    out[outOffset + 2] = '=';
	    out[outOffset + 3] = '=';
        }
    }

    public static byte[] decode(String encoded)
            throws IOException {
        return decode(encoded, 0, encoded.length());
    }

    public static byte[] decode(String encoded, int offset, int length)
            throws IOException {
        int i;
        int decodedLen;
        byte[] decoded;

        // the input must be a multiple of 4
        if (length % 4 != 0) {
            throw new IOException(
                "Base64 string length is not multiple of 4");
        }

        // 4 chars for 3 bytes, but there may have been pad bytes
        decodedLen = length / 4 * 3;
        if (encoded.charAt(offset + length - 1) == '=') {
            decodedLen--;
            if (encoded.charAt(offset + length - 2) == '=') {
                decodedLen--;
            }
        }

        decoded = new byte [decodedLen];
        for (i = 0, decodedLen = 0; i < length; i += 4, decodedLen += 3) {
            decodeQuantum(encoded.charAt(offset + i),
                          encoded.charAt(offset + i + 1),
                          encoded.charAt(offset + i + 2),
                          encoded.charAt(offset + i + 3),
                          decoded, decodedLen);
        }
               
        return decoded;
    }

    private static void decodeQuantum(char in1, char in2, char in3, char in4, 
                                byte[] out, int outOffset)
	throws IOException {
	int a = 0, b = 0, c = 0, d = 0;
        int pad = 0;

        a = valueDecoding[in1 & 127];
        b = valueDecoding[in2 & 127];

        if (in4 == '=') {
            pad++;
            if (in3 == '=') {
                pad++;
            } else {
                c = valueDecoding[in3 & 127];
            }
        } else {
            c = valueDecoding[in3 & 127];
            d = valueDecoding[in4 & 127];
        }

	if (a < 0 || b < 0 || c < 0 || d < 0) {
            throw new IOException("Invalid character in Base64 string");
        }

        // the first byte is the 6 bits of a and 2 bits of b
        out[outOffset] = (byte)(((a << 2) & 0xfc) | ((b >>> 4) & 3));

        if (pad < 2) {
            // the second byte is 4 bits of b and 4 bits of c
            out[outOffset + 1] = (byte)(((b << 4) & 0xf0) | ((c >>> 2) & 0xf));

            if (pad < 1) {
                // the third byte is 2 bits of c and 4 bits of d
                out[outOffset + 2] =
                    (byte)(((c << 6) & 0xc0) | (d  & 0x3f));
            }
        }
    }
}
