package com.nirenr;

public class Color {
    public int red;
    public int green;
    public int blue;

    public Color(int color) {
        red = color << 8 >>> 24;
        green = color << 16 >>> 24;
        blue = color << 24 >>> 24;
    }

    public Color(int r, int g, int b) {
        red = r;
        green = g;
        blue = b;
    }

    public int getInt() {
        return 0xFF000000|red<<16|green<<8|blue;
    }

    @Override
    public String toString() {
        return "Color(" + red + ", " + green + ", " + blue + ")";
    }

}
