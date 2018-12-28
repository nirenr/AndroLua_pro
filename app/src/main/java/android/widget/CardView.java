/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.widget;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.os.Build;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.View;

/**
 * A FrameLayout with a rounded corner background and shadow.
 * <p>
 * CardView uses <code>elevation</code> property on L for shadows and falls back to a custom shadow
 * implementation on older platforms.
 * <p>
 * Due to expensive nature of rounded corner clipping, on platforms before L, CardView does not
 * clip its children that intersect with rounded corners. Instead, it adds padding to avoid such
 * intersection (See {@link #setPreventCornerOverlap(boolean)} to change this behavior).
 * <p>
 * Before L, CardView adds padding to its content and draws shadows to that area. This padding
 * amount is equal to <code>maxCardElevation + (1 - cos45) * cornerRadius</code> on the sides and
 * <code>maxCardElevation * 1.5 + (1 - cos45) * cornerRadius</code> on top and bottom.
 * <p>
 * Since padding is used to offset content for shadows, you cannot set padding on CardView.
 * Instead,
 * you can use content padding attributes in XML or {@link #setContentPadding(int, int, int, int)}
 * in code to set the padding between the edges of the Card and children of CardView.
 * <p>
 * Note that, if you specify exact dimensions for the CardView, because of the shadows, its content
 * area will be different between platforms before L and after L. By using api version specific
 * resource values, you can avoid these changes. Alternatively, If you want CardView to add inner
 * padding on platforms L and after as well, you can set {@link #setUseCompatPadding(boolean)} to
 * <code>true</code>.
 * <p>
 * To change CardView's elevation in a backward compatible way, use
 * {@link #setCardElevation(float)}. CardView will use elevation API on L and before L, it will
 * change the shadow size. To avoid moving the View while shadow size is changing, shadow size is
 * clamped by {@link #getMaxCardElevation()}. If you want to change elevation dynamically, you
 * should call {@link #setMaxCardElevation(float)} when CardView is initialized.
 *
 * @attr ref android.support.v7.cardview.R.styleable#CardView_cardBackgroundColor
 * @attr ref android.support.v7.cardview.R.styleable#CardView_cardCornerRadius
 * @attr ref android.support.v7.cardview.R.styleable#CardView_cardElevation
 * @attr ref android.support.v7.cardview.R.styleable#CardView_cardMaxElevation
 * @attr ref android.support.v7.cardview.R.styleable#CardView_cardUseCompatPadding
 * @attr ref android.support.v7.cardview.R.styleable#CardView_cardPreventCornerOverlap
 * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPadding
 * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingLeft
 * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingTop
 * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingRight
 * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingBottom
 */
public class CardView extends FrameLayout implements CardViewDelegate {

    private static final CardViewImpl IMPL;

	private DisplayMetrics dm;

    static {
        if (Build.VERSION.SDK_INT >= 21) {
            IMPL = new CardViewApi21();
        } else if (Build.VERSION.SDK_INT >= 17) {
            IMPL = new CardViewJellybeanMr1();
        } else {
            IMPL = new CardViewEclairMr1();
        }
        IMPL.initStatic();
    }

    private boolean mCompatPadding;

    private boolean mPreventCornerOverlap;

    private final Rect mContentPadding = new Rect();

    private final Rect mShadowBounds = new Rect();


    public CardView(Context context) {
        super(context);
        initialize(context, null, 0);
    }

    public CardView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize(context, attrs, 0);
    }

    public CardView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initialize(context, attrs, defStyleAttr);
    }

    @Override
    public void setPadding(int left, int top, int right, int bottom) {
        // NO OP
    }

    public void setPaddingRelative(int start, int top, int end, int bottom) {
        // NO OP
    }

    /**
     * Returns whether CardView will add inner padding on platforms L and after.
     *
     * @return True CardView adds inner padding on platforms L and after to have same dimensions
     * with platforms before L.
     */
    @Override
    public boolean getUseCompatPadding() {
        return mCompatPadding;
    }

    /**
     * CardView adds additional padding to draw shadows on platforms before L.
     * <p>
     * This may cause Cards to have different sizes between L and before L. If you need to align
     * CardView with other Views, you may need api version specific dimension resources to account
     * for the changes.
     * As an alternative, you can set this flag to <code>true</code> and CardView will add the same
     * padding values on platforms L and after.
     * <p>
     * Since setting this flag to true adds unnecessary gaps in the UI, default value is
     * <code>false</code>.
     *
     * @param useCompatPadding True if CardView should add padding for the shadows on platforms L
     *                         and above.
     * @attr ref android.support.v7.cardview.R.styleable#CardView_cardUseCompatPadding
     */
    public void setUseCompatPadding(boolean useCompatPadding) {
        if (mCompatPadding == useCompatPadding) {
            return;
        }
        mCompatPadding = useCompatPadding;
        IMPL.onCompatPaddingChanged(this);
    }

    /**
     * Sets the padding between the Card's edges and the children of CardView.
     * <p>
     * Depending on platform version or {@link #getUseCompatPadding()} settings, CardView may
     * update these values before calling {@link android.view.View#setPadding(int, int, int, int)}.
     *
     * @param left   The left padding in pixels
     * @param top    The top padding in pixels
     * @param right  The right padding in pixels
     * @param bottom The bottom padding in pixels
     * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPadding
     * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingLeft
     * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingTop
     * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingRight
     * @attr ref android.support.v7.cardview.R.styleable#CardView_contentPaddingBottom
     */
    public void setContentPadding(int left, int top, int right, int bottom) {
        mContentPadding.set(left, top, right, bottom);
        IMPL.updatePadding(this);
    }

    @SuppressLint("SwitchIntDef")
	@Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (!(IMPL instanceof CardViewApi21)) {
            final int widthMode = MeasureSpec.getMode(widthMeasureSpec);
            switch (widthMode) {
                case MeasureSpec.EXACTLY:
                case MeasureSpec.AT_MOST:
                    final int minWidth = (int) Math.ceil(IMPL.getMinWidth(this));
                    widthMeasureSpec = MeasureSpec.makeMeasureSpec(Math.max(minWidth,
                            MeasureSpec.getSize(widthMeasureSpec)), widthMode);
                    break;
            }

            final int heightMode = MeasureSpec.getMode(heightMeasureSpec);
            switch (heightMode) {
                case MeasureSpec.EXACTLY:
                case MeasureSpec.AT_MOST:
                    final int minHeight = (int) Math.ceil(IMPL.getMinHeight(this));
                    heightMeasureSpec = MeasureSpec.makeMeasureSpec(Math.max(minHeight,
                            MeasureSpec.getSize(heightMeasureSpec)), heightMode);
                    break;
            }
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        } else {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        }
    }

    private void initialize(Context context, AttributeSet attrs, int defStyleAttr) {
        dm=context.getResources().getDisplayMetrics();
		TypedArray array = context.getTheme().obtainStyledAttributes(new int[] {  
																		  android.R.attr.colorBackground, 
																		  }); 
		int backgroundColor = array.getColor(0, 0x00FFFAFAFA); 
		array.recycle();
		
		//TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.CardView, defStyleAttr,
        //        R.style.CardView_Light);
        //int backgroundColor = 0x00FFFAFAFA;
		
		//a.getColor(R.styleable.CardView_cardBackgroundColor, 0);
        float radius = dp(2);
		//a.getDimension(R.styleable.CardView_cardCornerRadius, 0);
        float elevation = dp(2);
		//a.getDimension(R.styleable.CardView_cardElevation, 0);
        float maxElevation =dp(2);
		//a.getDimension(R.styleable.CardView_cardMaxElevation, 0);
        mCompatPadding = false;
		//a.getBoolean(R.styleable.CardView_cardUseCompatPadding, false);
        mPreventCornerOverlap = true;
		//a.getBoolean(R.styleable.CardView_cardPreventCornerOverlap, true);
        int defaultPadding = 0;
		//a.getDimensionPixelSize(R.styleable.CardView_contentPadding, 0);
        mContentPadding.left = 0;
		//a.getDimensionPixelSize(R.styleable.CardView_contentPaddingLeft,
                //defaultPadding);
        mContentPadding.top = 0;
		//a.getDimensionPixelSize(R.styleable.CardView_contentPaddingTop,
                //defaultPadding);
        mContentPadding.right = 0;
		//a.getDimensionPixelSize(R.styleable.CardView_contentPaddingRight,
                //defaultPadding);
        mContentPadding.bottom = 0;
		//a.getDimensionPixelSize(R.styleable.CardView_contentPaddingBottom,
                //defaultPadding);
        if (elevation > maxElevation) {
            maxElevation = elevation;
        }
        //a.recycle();
        IMPL.initialize(this, context, backgroundColor, radius, elevation, maxElevation);
    }

	private float dp(float n) {
		// TODO: Implement this method
		return TypedValue.applyDimension(1,n,dm);
	}
	

    /**
     * Updates the background color of the CardView
     *
     * @param color The new color to set for the card background
     * @attr ref android.support.v7.cardview.R.styleable#CardView_cardBackgroundColor
     */
    public void setCardBackgroundColor(int color) {
        IMPL.setBackgroundColor(this, color);
    }
	
    public void setBackgroundColor(int color) {
        IMPL.setBackgroundColor(this, color);
    }
	
    /**
     * Returns the inner padding after the Card's left edge
     *
     * @return the inner padding after the Card's left edge
     */
    public int getContentPaddingLeft() {
        return mContentPadding.left;
    }

    /**
     * Returns the inner padding before the Card's right edge
     *
     * @return the inner padding before the Card's right edge
     */
    public int getContentPaddingRight() {
        return mContentPadding.right;
    }

    /**
     * Returns the inner padding after the Card's top edge
     *
     * @return the inner padding after the Card's top edge
     */
    public int getContentPaddingTop() {
        return mContentPadding.top;
    }

    /**
     * Returns the inner padding before the Card's bottom edge
     *
     * @return the inner padding before the Card's bottom edge
     */
    public int getContentPaddingBottom() {
        return mContentPadding.bottom;
    }

    /**
     * Updates the corner radius of the CardView.
     *
     * @param radius The radius in pixels of the corners of the rectangle shape
     * @attr ref android.support.v7.cardview.R.styleable#CardView_cardCornerRadius
     * @see #setRadius(float)
     */
    public void setRadius(float radius) {
        IMPL.setRadius(this, radius);
    }

    /**
     * Returns the corner radius of the CardView.
     *
     * @return Corner radius of the CardView
     * @see #getRadius()
     */
    public float getRadius() {
        return IMPL.getRadius(this);
    }

    /**
     * Internal method used by CardView implementations to update the padding.
     *
     * @hide
     */
    @Override
    public void setShadowPadding(int left, int top, int right, int bottom) {
        mShadowBounds.set(left, top, right, bottom);
        super.setPadding(left + mContentPadding.left, top + mContentPadding.top,
                right + mContentPadding.right, bottom + mContentPadding.bottom);
    }

    /**
     * Updates the backward compatible elevation of the CardView.
     *
     * @param radius The backward compatible elevation in pixels.
     * @attr ref android.support.v7.cardview.R.styleable#CardView_cardElevation
     * @see #getCardElevation()
     * @see #setMaxCardElevation(float)
     */
    public void setCardElevation(float radius) {
        IMPL.setElevation(this, radius);
    }

    /**
     * Returns the backward compatible elevation of the CardView.
     *
     * @return Elevation of the CardView
     * @see #setCardElevation(float)
     * @see #getMaxCardElevation()
     */
    public float getCardElevation() {
        return IMPL.getElevation(this);
    }

    /**
     * Updates the backward compatible elevation of the CardView.
     * <p>
     * Calling this method has no effect if device OS version is L or newer and
     * {@link #getUseCompatPadding()} is <code>false</code>.
     *
     * @param radius The backward compatible elevation in pixels.
     * @attr ref android.support.v7.cardview.R.styleable#CardView_cardElevation
     * @see #setCardElevation(float)
     * @see #getMaxCardElevation()
     */
    public void setMaxCardElevation(float radius) {
        IMPL.setMaxElevation(this, radius);
    }

    /**
     * Returns the backward compatible elevation of the CardView.
     *
     * @return Elevation of the CardView
     * @see #setMaxCardElevation(float)
     * @see #getCardElevation()
     */
    public float getMaxCardElevation() {
        return IMPL.getMaxElevation(this);
    }

    /**
     * Returns whether CardView should add extra padding to content to avoid overlaps with rounded
     * corners on API versions 20 and below.
     *
     * @return True if CardView prevents overlaps with rounded corners on platforms before L.
     *         Default value is <code>true</code>.
     */
    @Override
    public boolean getPreventCornerOverlap() {
        return mPreventCornerOverlap;
    }

    /**
     * On API 20 and before, CardView does not clip the bounds of the Card for the rounded corners.
     * Instead, it adds padding to content so that it won't overlap with the rounded corners.
     * You can disable this behavior by setting this field to <code>false</code>.
     * <p>
     * Setting this value on API 21 and above does not have any effect unless you have enabled
     * compatibility padding.
     *
     * @param preventCornerOverlap Whether CardView should add extra padding to content to avoid
     *                             overlaps with the CardView corners.
     * @attr ref android.support.v7.cardview.R.styleable#CardView_cardPreventCornerOverlap
     * @see #setUseCompatPadding(boolean)
     */
    public void setPreventCornerOverlap(boolean preventCornerOverlap) {
        if (preventCornerOverlap == mPreventCornerOverlap) {
            return;
        }
        mPreventCornerOverlap = preventCornerOverlap;
        IMPL.onPreventCornerOverlapChanged(this);
    }


	@SuppressLint("NewApi")
	static class CardViewApi21 implements CardViewImpl {

		@Override
		public void initialize(CardViewDelegate cardView, Context context, int backgroundColor,
							   float radius, float elevation, float maxElevation) {
			final RoundRectDrawable backgroundDrawable = new RoundRectDrawable(backgroundColor, radius);
			cardView.setBackgroundDrawable(backgroundDrawable);
			View view = (View) cardView;
			view.setClipToOutline(true);
			view.setElevation(elevation);
			setMaxElevation(cardView, maxElevation);
		}

		@Override
		public void setRadius(CardViewDelegate cardView, float radius) {
			((RoundRectDrawable) (cardView.getBackground())).setRadius(radius);
		}

		@Override
		public void initStatic() {
		}

		@Override
		public void setMaxElevation(CardViewDelegate cardView, float maxElevation) {
			((RoundRectDrawable) (cardView.getBackground())).setPadding(maxElevation,
																		cardView.getUseCompatPadding(), cardView.getPreventCornerOverlap());
			updatePadding(cardView);
		}

		@Override
		public float getMaxElevation(CardViewDelegate cardView) {
			return ((RoundRectDrawable) (cardView.getBackground())).getPadding();
		}

		@Override
		public float getMinWidth(CardViewDelegate cardView) {
			return getRadius(cardView) * 2;
		}

		@Override
		public float getMinHeight(CardViewDelegate cardView) {
			return getRadius(cardView) * 2;
		}

		@Override
		public float getRadius(CardViewDelegate cardView) {
			return ((RoundRectDrawable) (cardView.getBackground())).getRadius();
		}

		@Override
		public void setElevation(CardViewDelegate cardView, float elevation) {
			((View) cardView).setElevation(elevation);
		}

		@Override
		public float getElevation(CardViewDelegate cardView) {
			return ((View) cardView).getElevation();
		}

		@Override
		public void updatePadding(CardViewDelegate cardView) {
			if (!cardView.getUseCompatPadding()) {
				cardView.setShadowPadding(0, 0, 0, 0);
				return;
			}
			float elevation = getMaxElevation(cardView);
			final float radius = getRadius(cardView);
			int hPadding = (int) Math.ceil(RoundRectDrawableWithShadow
										   .calculateHorizontalPadding(elevation, radius, cardView.getPreventCornerOverlap()));
			int vPadding = (int) Math.ceil(RoundRectDrawableWithShadow
										   .calculateVerticalPadding(elevation, radius, cardView.getPreventCornerOverlap()));
			cardView.setShadowPadding(hPadding, vPadding, hPadding, vPadding);
		}

		@Override
		public void onCompatPaddingChanged(CardViewDelegate cardView) {
			setMaxElevation(cardView, getMaxElevation(cardView));
		}

		@Override
		public void onPreventCornerOverlapChanged(CardViewDelegate cardView) {
			setMaxElevation(cardView, getMaxElevation(cardView));
		}

		@Override
		public void setBackgroundColor(CardViewDelegate cardView, int color) {
			((RoundRectDrawable) (cardView.getBackground())).setColor(color);
		}
	}
	
	static class CardViewEclairMr1 implements CardViewImpl {

		final RectF sCornerRect = new RectF();

		@Override
		public void initStatic() {
			// Draws a round rect using 7 draw operations. This is faster than using
			// canvas.drawRoundRect before JBMR1 because API 11-16 used alpha mask textures to draw
			// shapes.
			RoundRectDrawableWithShadow.sRoundRectHelper
                = new RoundRectDrawableWithShadow.RoundRectHelper() {
				@Override
				public void drawRoundRect(Canvas canvas, RectF bounds, float cornerRadius,
										  Paint paint) {
					final float twoRadius = cornerRadius * 2;
					final float innerWidth = bounds.width() - twoRadius - 1;
					final float innerHeight = bounds.height() - twoRadius - 1;
					// increment it to account for half pixels.
					if (cornerRadius >= 1f) {
						cornerRadius += .5f;
						sCornerRect.set(-cornerRadius, -cornerRadius, cornerRadius, cornerRadius);
						int saved = canvas.save();
						canvas.translate(bounds.left + cornerRadius, bounds.top + cornerRadius);
						canvas.drawArc(sCornerRect, 180, 90, true, paint);
						canvas.translate(innerWidth, 0);
						canvas.rotate(90);
						canvas.drawArc(sCornerRect, 180, 90, true, paint);
						canvas.translate(innerHeight, 0);
						canvas.rotate(90);
						canvas.drawArc(sCornerRect, 180, 90, true, paint);
						canvas.translate(innerWidth, 0);
						canvas.rotate(90);
						canvas.drawArc(sCornerRect, 180, 90, true, paint);
						canvas.restoreToCount(saved);
						//draw top and bottom pieces
						canvas.drawRect(bounds.left + cornerRadius - 1f, bounds.top,
										bounds.right - cornerRadius + 1f, bounds.top + cornerRadius,
										paint);
						canvas.drawRect(bounds.left + cornerRadius - 1f,
										bounds.bottom - cornerRadius + 1f, bounds.right - cornerRadius + 1f,
										bounds.bottom, paint);
					}
////                center
					canvas.drawRect(bounds.left, bounds.top + Math.max(0, cornerRadius - 1f),
									bounds.right, bounds.bottom - cornerRadius + 1f, paint);
				}
			};
		}

		@Override
		public void initialize(CardViewDelegate cardView, Context context, int backgroundColor,
							   float radius, float elevation, float maxElevation) {
			RoundRectDrawableWithShadow background = createBackground(context, backgroundColor, radius,
																	  elevation, maxElevation);
			background.setAddPaddingForCorners(cardView.getPreventCornerOverlap());
			cardView.setBackgroundDrawable(background);
			updatePadding(cardView);
		}

		RoundRectDrawableWithShadow createBackground(Context context, int backgroundColor,
													 float radius, float elevation, float maxElevation) {
			return new RoundRectDrawableWithShadow(context.getResources(), backgroundColor, radius,
												   elevation, maxElevation);
		}

		@Override
		public void updatePadding(CardViewDelegate cardView) {
			Rect shadowPadding = new Rect();
			getShadowBackground(cardView).getMaxShadowAndCornerPadding(shadowPadding);
			((View) cardView).setMinimumHeight((int) Math.ceil(getMinHeight(cardView)));
			((View) cardView).setMinimumWidth((int) Math.ceil(getMinWidth(cardView)));
			cardView.setShadowPadding(shadowPadding.left, shadowPadding.top,
									  shadowPadding.right, shadowPadding.bottom);
		}

		@Override
		public void onCompatPaddingChanged(CardViewDelegate cardView) {
			// NO OP
		}

		@Override
		public void onPreventCornerOverlapChanged(CardViewDelegate cardView) {
			getShadowBackground(cardView).setAddPaddingForCorners(cardView.getPreventCornerOverlap());
			updatePadding(cardView);
		}

		@Override
		public void setBackgroundColor(CardViewDelegate cardView, int color) {
			getShadowBackground(cardView).setColor(color);
		}

		@Override
		public void setRadius(CardViewDelegate cardView, float radius) {
			getShadowBackground(cardView).setCornerRadius(radius);
			updatePadding(cardView);
		}

		@Override
		public float getRadius(CardViewDelegate cardView) {
			return getShadowBackground(cardView).getCornerRadius();
		}

		@Override
		public void setElevation(CardViewDelegate cardView, float elevation) {
			getShadowBackground(cardView).setShadowSize(elevation);
		}

		@Override
		public float getElevation(CardViewDelegate cardView) {
			return getShadowBackground(cardView).getShadowSize();
		}

		@Override
		public void setMaxElevation(CardViewDelegate cardView, float maxElevation) {
			getShadowBackground(cardView).setMaxShadowSize(maxElevation);
			updatePadding(cardView);
		}

		@Override
		public float getMaxElevation(CardViewDelegate cardView) {
			return getShadowBackground(cardView).getMaxShadowSize();
		}

		@Override
		public float getMinWidth(CardViewDelegate cardView) {
			return getShadowBackground(cardView).getMinWidth();
		}

		@Override
		public float getMinHeight(CardViewDelegate cardView) {
			return getShadowBackground(cardView).getMinHeight();
		}

		private RoundRectDrawableWithShadow getShadowBackground(CardViewDelegate cardView) {
			return ((RoundRectDrawableWithShadow) cardView.getBackground());
		}
	}
	
	
	static class CardViewJellybeanMr1 extends CardViewEclairMr1 {

		@Override
		public void initStatic() {
			RoundRectDrawableWithShadow.sRoundRectHelper
                = new RoundRectDrawableWithShadow.RoundRectHelper() {
				@Override
				public void drawRoundRect(Canvas canvas, RectF bounds, float cornerRadius,
										  Paint paint) {
					canvas.drawRoundRect(bounds, cornerRadius, cornerRadius, paint);
				}
			};
		}
	}
	
}
