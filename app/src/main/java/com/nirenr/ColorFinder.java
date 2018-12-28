package com.nirenr;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Rect;

import java.util.ArrayList;


/**
 * Created by Administrator on 2018/03/10 0010.
 */

public class ColorFinder {

    private int mWidth;
    private int mHeight;
    private int[][] mPixels;
    private float[][] mValues;
    private float mValue;

    public ColorFinder(String bitmap) {
        init(BitmapFactory.decodeFile(bitmap));
    }

    public ColorFinder(Bitmap bitmap) {
        init(bitmap);
    }

    private void init(Bitmap bitmap) {
        mWidth = bitmap.getWidth();
        mHeight = bitmap.getHeight();
        int[] pixels = new int[mWidth * mHeight];
        bitmap.getPixels(pixels, 0, mWidth, 0, 0, mWidth, mHeight);
        mPixels = new int[mWidth][mHeight];
        for (int h = 0; h < mHeight; h++) {
            for (int w = 0; w < mWidth; w++) {
                mPixels[w][h] = pixels[h * mWidth + w];
            }
        }
    }

    public int[][] getPixels() {
        return mPixels;
    }

    public Point find(int color) {
        for (int h = 0; h < mHeight; h++) {
            for (int w = 0; w < mWidth; w++) {
                if (mPixels[w][h] == color)
                    return new Point(w, h);
            }
        }
        return new Point(-1, -1);
    }

    public Point find(int x, int y, int x2, int y2, int color) {
        for (int h = y; h < y2; h++) {
            for (int w = x; w < x2; w++) {
                if (mPixels[w][h] == color)
                    return new Point(w, h);
            }
        }
        return new Point(-1, -1);
    }

    public Point find(Color color) {
        return find(color.red, color.green, color.blue);
    }

    public Point find(int red, int green, int blue) {
        for (int h = 0; h < mHeight; h++) {
            for (int w = 0; w < mWidth; w++) {
                int color = mPixels[w][h];
                int r = color << 8 >>> 24;
                int g = color << 16 >>> 24;
                int b = color << 24 >>> 24;
                if (r == red && g == green && b == blue)
                    return new Point(w, h);
            }
        }
        return new Point(-1, -1);
    }

    public Point find(Point p1, Point p2, Color color) {
        return find(p1.x, p1.y, p2.x, p2.y, color.red, color.green, color.blue);
    }

    public Point find(int x, int y, int x2, int y2, int red, int green, int blue) {
        for (int h = y; h < y2; h++) {
            for (int w = x; w < x2; w++) {
                int color = mPixels[w][h];
                int r = color << 8 >>> 24;
                int g = color << 16 >>> 24;
                int b = color << 24 >>> 24;
                if (r == red && g == green && b == blue)
                    return new Point(w, h);
            }
        }
        return new Point(-1, -1);
    }

    public Point find(Color color, int offset) {
        return find(color.red, color.green, color.blue, offset);
    }

    public Point find(int red, int green, int blue, int offset) {
        int r1 = red - offset;
        int r2 = red + offset;
        int g1 = green - offset;
        int g2 = green + offset;
        int b1 = blue - offset;
        int b2 = blue + offset;

        for (int h = 0; h < mHeight; h++) {
            for (int w = 0; w < mWidth; w++) {
                int color = mPixels[w][h];
                int r = color << 8 >>> 24;
                int g = color << 16 >>> 24;
                int b = color << 24 >>> 24;
                if (r >= r1 && r <= r2 && g >= g1 && g <= g2 && b >= b1 && b <= b2)
                    return new Point(w, h);
            }
        }
        return new Point(-1, -1);
    }

    public Point find(Point p1, Point p2, Color color, int offset) {
        return find(p1.x, p1.y, p2.x, p2.y, color.red, color.green, color.blue, offset);
    }

    public Point find(int x, int y, int x2, int y2, int red, int green, int blue, int offset) {
        int r1 = red - offset;
        int r2 = red + offset;
        int g1 = green - offset;
        int g2 = green + offset;
        int b1 = blue - offset;
        int b2 = blue + offset;

        for (int h = y; h < y2; h++) {
            for (int w = x; w < x2; w++) {
                int color = mPixels[w][h];
                int r = color << 8 >>> 24;
                int g = color << 16 >>> 24;
                int b = color << 24 >>> 24;
                if (r >= r1 && r <= r2 && g >= g1 && g <= g2 && b >= b1 && b <= b2)
                    return new Point(w, h);
            }
        }
        return new Point(-1, -1);
    }

    public Point find(int x, int y, int x2, int y2, int red, int green, int blue, int offset, int[][] p) {
        ColorPoint[] cp = new ColorPoint[p.length];
        for (int i = 0; i < p.length; i++) {
            cp[i] = new ColorPoint(p[i]);
        }
        return find(x, y, x2, y2, red, green, blue, offset, cp);
    }

    public Point find(int x, int y, int x2, int y2, int red, int green, int blue, int offset, ColorPoint[] cp) {
        int r1 = red - offset;
        int r2 = red + offset;
        int g1 = green - offset;
        int g2 = green + offset;
        int b1 = blue - offset;
        int b2 = blue + offset;

        for (int h = y; h < y2; h++) {
            for (int w = x; w < x2; w++) {
                int color = mPixels[w][h];
                int r = color << 8 >>> 24;
                int g = color << 16 >>> 24;
                int b = color << 24 >>> 24;
                if (r >= r1 && r <= r2 && g >= g1 && g <= g2 && b >= b1 && b <= b2) {
                    boolean ok = true;
                    for (ColorPoint c : cp) {
                        if (!c.check(mPixels, x, y)) {
                            ok = false;
                            break;
                        }
                    }
                    if (ok)
                        return new Point(w, h);
                }
            }
        }
        return new Point(-1, -1);
    }

    public ArrayList<Rect> findLine(int o) {
        return findLine(mWidth / 2, 10, mWidth - 10, mHeight - o * 16, 0.5f, o * 8, o * 4, o);
    }

    public ArrayList<Rect> findLine(float n, int o) {
        return findLine(mWidth / 2, 10, mWidth - 10, mHeight - o * 16, n, o * 8, o * 4, o);
    }

    public ArrayList<Rect> findLine(float n, int h, int o) {
        if (mHeight < mWidth)
            return findLine(mWidth / 2, 0, mWidth - 10, mHeight - h, n, h, o * 4, o);
        else
            return findLine(mWidth / 2, mWidth / 3, mWidth - 10, mWidth, n, h, o * 4, o);
    }

    public ArrayList<Rect> findLine(float n, int h, int w, int o) {
        if (mHeight < mWidth)
            return findLine(mWidth / 2, 0, mWidth - 10, mHeight - h, n, h, w, o);
        else
            return findLine(mWidth / 2, mWidth / 3, mWidth - 10, mWidth, n, h, w, o);
    }

    public ArrayList<Rect> findLine(int x1, int y1, int x2, int y2, float n, int h, int w, int o) {
        if (mValues == null) {
            mValues = new float[mWidth][mHeight];
            float[] hsv = new float[3];
            float v = 0;
            for (int y = 0; y < mHeight; y++) {
                for (int x = 0; x < mWidth; x++) {
                    int color1 = mPixels[x][y];
                    android.graphics.Color.colorToHSV(color1, hsv);
                    mValues[x][y] = hsv[2];
                    v += hsv[2];
                }
            }
            mValue = v / (mWidth * mHeight);
        }

        int[][] colors = new int[mWidth][mHeight];
        float vv = mValue * n;
        for (int y = 0; y < mHeight; y++) {
            for (int x = 0; x < mWidth; x++) {
                int i = x + mWidth * y;
                if (mValues[x][y] > vv) {
                    colors[x][y] = 1;
                } else {
                    colors[x][y] = 0;
                }
            }
        }
        ArrayList<Rect> ret = new ArrayList<>();
        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                int l = check(x, y, colors, h, w, o);
                if (l > -1) {
                    x += o;
                    ret.add(new Rect(x, y, x,x+l));
                    break;
                }
            }
        }
        /*for (int x = 10; x < x1; x++) {
            for (int y = 10; y < mHeight-h*2; y++) {
                int l = check2(x, y, colors, h, w, o);
                    if (l > -1) {
                        x += o;
                        ret.add(new Rect(x, y, l,2));
                        break;
                    }
            }
        }
        for (int y = 10; y < mHeight-10; y++) {
            for (int x = 10; x < x1; x++) {
                int l = check3(x, y, colors, h, w, o);
                if (l > -1) {
                    y += o;
                    ret.add(new Point(y, l, 3));
                    break;
                } else {
                    l = check4(x, y, colors, h, w, o);
                    if (l > -1) {
                        y += o;
                        ret.add(new Point(y, l, 4));
                        break;
                    }
                }
            }
        }*/

        /*Collections.sort(ret, new Comparator<Rect>() {
            @Override
            public int compare(Rect o1, Rect o2) {
                if (o1.t < o2.t)
                    return -1;
                /*else  if (o1.y > o2.y)
                    return -1;
                else
                    return 1;
            }
        });*/
        return ret;
    }

    private int check(int x, int y, int[][] color, int h, int w, int o) {
        for (int i = 0; i < mHeight - y - h; i++) {
            if (!(color[x][y + i] == 1 && color[x + o][y + i] == 0 && color[x + o + w][y + i] == 0)) {
                if (i > h)
                    return i;
                else
                    return -1;
            }
        }
        return mHeight - y - h;
    }

    private int check2(int x, int y, int[][] color, int h, int w, int o) {
        for (int i = 0; i < mHeight - y - h; i++) {
            if (!(color[x][y + i] == 0 && color[x + o][y + i] == 1 && color[x + o + w][y + i] == 1)) {
                if (i > h)
                    return i;
                else
                    return -1;
            }
        }
        return mHeight - y - h;
    }

    private int check3(int x, int y, int[][] color, int h, int w, int o) {
        for (int i = 0; i < mWidth - x - h; i++) {
            if (!(color[x + i][y] == 1 && color[x + i][y + o] == 0 && color[x + i][y + o + w] == 0)) {
                if (i > h)
                    return i;
                else
                    return -1;
            }
        }
        return mHeight - y - h;
    }

    private int check4(int x, int y, int[][] color, int h, int w, int o) {
        for (int i = 0; i < mWidth - x - h; i++) {
            if (!(color[x + i][y] == 0 && color[x + i][y + o] == 1 && color[x + i][y + o + w] == 1)) {
                if (i > h)
                    return i;
                else
                    return -1;
            }
        }
        return mHeight - y - h;
    }
}
