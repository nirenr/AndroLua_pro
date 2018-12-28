package com.nirenr;

import android.graphics.Point;

public class ColorPoint {
    public int red;
    public int green;
    public int blue;
    public int x;
    public int y;
    public int offset;

    public ColorPoint(Point p,Color color,int o){
        this(p.x,p.y,color.red,color.green,color.blue,o);
    }

    public ColorPoint(int x, int y, int r, int g, int b, int o) {
        this.x = x;
        this.y = y;
        this.red = r;
        this.green = g;
        this.blue = b;
        this.offset = o;
    }

    public ColorPoint(int[] arg) {
        x = arg[0];
        y = arg[1];
        red = arg[2];
        green = arg[3];
        blue = arg[4];
        offset = arg[5];
    }

    public boolean check(int[][] pixels) {
        return check(pixels, 0, 0);
    }

    public boolean check(int[][] pixels, int x, int y) {
        int r1 = red - offset;
        int r2 = red + offset;
        int g1 = green - offset;
        int g2 = green + offset;
        int b1 = blue - offset;
        int b2 = blue + offset;
        int color = pixels[this.y + y][this.x + x];
        int r = color << 8 >>> 24;
        int g = color << 16 >>> 24;
        int b = color << 24 >>> 24;
        if (r >= r1 && r <= r2 && g >= g1 && g <= g2 && b >= b1 && b <= b2)
            return true;
        return false;
    }
}