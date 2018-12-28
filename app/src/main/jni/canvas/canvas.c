#if defined __WIN32__ || defined WIN32
# include <windows.h>
# define _EXPORT __declspec(dllexport)
#else
# define _EXPORT
#endif
#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* Constant that is used to index the JNI Environment */
#define LUAJAVAJNIENVTAG      "_JNIEnv"
/* Defines wheter the metatable is of a java Object */
#define LUAJAVAOBJECTIND      "__IsJavaObject"
/* Index metamethod name */
#define LUAINDEXMETAMETHODTAG "__index"
/* Call metamethod name */
#define LUACALLMETAMETHODTAG  "__call"

/*
**************************************
*全局静态类变量，为了缓存
**************************************
*/
static jclass canvas_class = NULL;
static jclass rect_class = NULL;
static jclass rectf_class = NULL;
static jclass charsequence_class = NULL;
static jclass path_class = NULL;

static jmethodID clippath1_method = NULL;
static jmethodID clippath2_method = NULL;

static jmethodID cliprect1_method = NULL;
static jmethodID cliprect2_method = NULL;
static jmethodID cliprect3_method = NULL;
static jmethodID cliprect4_method = NULL;
static jmethodID cliprect5_method = NULL;
static jmethodID cliprect6_method = NULL;

static jmethodID clipregion1_method = NULL;
static jmethodID clipregion2_method = NULL;

static jmethodID concat_method = NULL;
static jmethodID drawargb_method = NULL;
static jmethodID drawarc_method = NULL;

static jmethodID drawbitmap1_method = NULL;
static jmethodID drawbitmap2_method = NULL;
static jmethodID drawbitmap3_method = NULL;
static jmethodID drawbitmap4_method = NULL;
static jmethodID drawbitmap5_method = NULL;

static jmethodID drawbitmapmesh_method = NULL;
static jmethodID drawcircle_method = NULL;

static jmethodID drawcolor1_method = NULL;
static jmethodID drawcolor2_method = NULL;

static jmethodID drawline_method = NULL;

static jmethodID drawlines1_method = NULL;
static jmethodID drawlines2_method = NULL;

static jmethodID drawoval_method = NULL;
static jmethodID drawpaint_method = NULL;
static jmethodID drawpath_method = NULL;

static jmethodID drawpicture1_method = NULL;
static jmethodID drawpicture2_method = NULL;
static jmethodID drawpicture3_method = NULL;

static jmethodID drawpoint_method = NULL;

static jmethodID drawpoints1_method = NULL;
static jmethodID drawpoints2_method = NULL;

static jmethodID drawpostext1_method = NULL;
static jmethodID drawpostext2_method = NULL;

static jmethodID drawrgb_method = NULL;
static jmethodID drawroundrect_method = NULL;

static jmethodID drawrect1_method = NULL;
static jmethodID drawrect2_method = NULL;
static jmethodID drawrect3_method = NULL;

static jmethodID drawtext1_method = NULL;
static jmethodID drawtext2_method = NULL;
static jmethodID drawtext3_method = NULL;
static jmethodID drawtext4_method = NULL;

static jmethodID drawtextonpath1_method = NULL;
static jmethodID drawtextonpath2_method = NULL;

static jmethodID drawvertices_method = NULL;

static jmethodID getclipbounds1_method = NULL;
static jmethodID getclipbounds2_method = NULL;

static jmethodID getdensity_method = NULL;
static jmethodID getheight_method = NULL;

static jmethodID getmatrix1_method = NULL;
static jmethodID getmatrix2_method = NULL;

static jmethodID getmaximumbitmapheight_method = NULL;
static jmethodID getmaximumbitmapwidth_method = NULL;
static jmethodID getsavecount_method = NULL;

static jmethodID getwidth_method = NULL;
static jmethodID ishardwareaccelerated_method = NULL;
static jmethodID isopaque_method = NULL;

static jmethodID quickreject1_method = NULL;
static jmethodID quickreject2_method = NULL;
static jmethodID quickreject3_method = NULL;

static jmethodID restore_method = NULL;
static jmethodID restoretocount_method = NULL;

static jmethodID rotate1_method = NULL;
static jmethodID rotate2_method = NULL;

static jmethodID save1_method = NULL;
static jmethodID save2_method = NULL;

static jmethodID savelayer1_method = NULL;
static jmethodID savelayer2_method = NULL;

static jmethodID savelayeralpha1_method = NULL;
static jmethodID savelayeralpha2_method = NULL;

static jmethodID scale1_method = NULL;
static jmethodID scale2_method = NULL;

static jmethodID setbitmap_method = NULL;
static jmethodID setdensity_method = NULL;
static jmethodID setdrawfilter_method = NULL;
static jmethodID setmatrix_method = NULL;
static jmethodID skew_method = NULL;
static jmethodID translate_method = NULL;

static jmethodID lockcanvas_method = NULL;
static jmethodID unlockcanvas_method = NULL;

static jclass holder_class = NULL;

static JNIEnv *getEnvFromState(lua_State * L);
static int isJavaObject(lua_State * L, int idx);
/* 
 **************************************
 *
 *boolean clipPath
 *
 *2 Method
 *
 **************************************
 */
int clipPath(lua_State * L)
{
	jobject canvas, path, region_Op;
	jboolean ret;
	JNIEnv *javaEnv;
	int statement_num;

	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		if (clippath1_method == NULL)
			clippath1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipPath",
										"(Landroid/graphics/Path;)Z");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		path = (jobject) * (jobject *) lua_touserdata(L, 2);

		ret =
			(*javaEnv)->CallBooleanMethod(javaEnv, canvas, clippath1_method,
										  path);
		lua_pushboolean(L, ret);
	}
	else if (statement_num == 3)
	{
		
		if (clippath2_method == NULL)
			clippath2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipPath",
										"(Landroid/graphics/Path;Landroid/graphics/Region.Op;)Z");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		path = (jobject) * (jobject *) lua_touserdata(L, 2);
		region_Op = (jobject) * (jobject *) lua_touserdata(L, 3);

		ret =
			(*javaEnv)->CallBooleanMethod(javaEnv, canvas, clippath2_method,
										  path, region_Op);
		lua_pushboolean(L, ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}

/* 
 **************************************
 *
 *boolean clipRect
 *
 *7 Method(其中一个已合并到4个float参数的方法,它的参数是4个int)
 *
 **************************************
 */
int clipRect(lua_State * L)
{
	jclass clazz;
	/*临时类，用完释放掉,下同*/
	jobject canvas;
	jobject who, region_Op;
	/*who是获取栈中的值，用来判断类型,下同*/
	jboolean ret;
	lua_Number left, top, right, bottom;
	JNIEnv *javaEnv;
	int statement_num;

	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	/*获取传入的参数个数,下同*/
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	/*根据参数个数判断调用，下同*/
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		who = (jobject) * (jobject *) lua_touserdata(L, 2);
		
		/*如果类没有被使用过则新建类的引用，为了缓存，下同*/
		if (rect_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/Rect");
			rect_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}

		if (rectf_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/RectF");
			rectf_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		/*判断传进来的参数，根据类型调用方法，下同*/
		if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rect_class) == JNI_TRUE)
		{
			if (cliprect1_method == NULL)
				cliprect1_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRect",
											"(Landroid/graphics/Rect;)Z");

			ret =
				(*javaEnv)->CallBooleanMethod(javaEnv, canvas, cliprect1_method,
											  who);
			//lua_pushboolean(L, ret);
		}
		else if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rectf_class) == JNI_TRUE)
		{
			if (cliprect2_method == NULL)
				cliprect2_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRect",
											"(Landroid/graphics/RectF;)Z");

			ret =
				(*javaEnv)->CallBooleanMethod(javaEnv, canvas, cliprect2_method,
											  who);
			//lua_pushboolean(L, ret);
		}
		lua_pushboolean(L, ret);
	}

	else if (statement_num == 3)
	{
		
			canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
			who = (jobject) * (jobject *) lua_touserdata(L, 2);
			region_Op = (jobject) * (jobject *) lua_touserdata(L, 3);
		
		if (rect_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/Rect");
			rect_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}

		if (rectf_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/RectF");
			rectf_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		
		if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rect_class) == JNI_TRUE)
		{
			if (cliprect3_method == NULL)
				cliprect3_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRect",
											"(Landroid/graphics/Rect;Landroid/graphics/Region.Op;)Z");

			ret =
				(*javaEnv)->CallBooleanMethod(javaEnv, canvas, cliprect3_method,
											  who, region_Op);
			//lua_pushboolean(L, ret);
		}
		else if ((*javaEnv)->IsInstanceOf
				 (javaEnv,who,rectf_class) == JNI_TRUE)
		{
			if (cliprect4_method == NULL)
				cliprect4_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRect",
											"(Landroid/graphics/RectF;Landroid/graphics/Region.Op;)Z");

			ret =
				(*javaEnv)->CallBooleanMethod(javaEnv, canvas, cliprect4_method,
											  who, region_Op);
			//lua_pushboolean(L, ret);
		}

		lua_pushboolean(L, ret);
	}

	else if (statement_num == 5)
	{
		if (cliprect5_method == NULL)
			cliprect5_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRect",
										"(FFFF)Z");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);

		left = lua_tonumber(L, 2);
		top = lua_tonumber(L, 3);
		right = lua_tonumber(L, 4);
		bottom = lua_tonumber(L, 5);
		ret =
			(*javaEnv)->CallBooleanMethod(javaEnv, canvas, cliprect5_method,
										  (jfloat) left, (jfloat) top,
										  (jfloat) right, (jfloat) bottom);
		lua_pushboolean(L, ret);
	}
	else if (statement_num == 6)
	{
		if (cliprect6_method == NULL)
			cliprect6_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipPath",
										"(FFFFLandroid/graphics/Region.Op;)Z");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		left = lua_tonumber(L, 2);
		top = lua_tonumber(L, 3);
		right = lua_tonumber(L, 4);
		bottom = lua_tonumber(L, 5);
		region_Op = (jobject) * (jobject *) lua_touserdata(L, 6);

		ret =
			(*javaEnv)->CallBooleanMethod(javaEnv, canvas, cliprect6_method,
										  (jfloat) left, (jfloat) top,
										  (jfloat) right, (jfloat) bottom,
										  region_Op);
		lua_pushboolean(L, ret);
	}
	/*传入的参数个数不对，抛出错误,下同*/
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	/*return 1;表示这个方法有1个返回值*/
	return 1;
}

/* 
 **************************************
 *
 *boolean clipRegion
 *
 *2 Method
 *
 **************************************
 */
int clipRegion(lua_State * L)
{
	jobject canvas, region, region_Op;
	jboolean ret;
	JNIEnv *javaEnv;
	int statement_num;

	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		if (clipregion1_method == NULL)
			clipregion1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRegion",
										"(Landroid/graphics/Region;)Z");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		region_Op = (jobject) * (jobject *) lua_touserdata(L, 2);
		ret =
			(*javaEnv)->CallBooleanMethod(javaEnv, canvas, clipregion1_method,
										  region);
		lua_pushboolean(L, ret);
	}
	else if (statement_num == 3)
	{
		if (clipregion2_method == NULL)
			clipregion2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "clipRegion",
										"(Landroid/graphics/Region;Landroid/graphics/Region.Op;)Z");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		region = (jobject) * (jobject *) lua_touserdata(L, 2);
		region_Op = (jobject) * (jobject *) lua_touserdata(L, 3);
		ret =
			(*javaEnv)->CallBooleanMethod(javaEnv, canvas, clipregion2_method,
										  region, region_Op);
		lua_pushboolean(L, ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}

/* 
 **************************************
 *
 *void concat
 *
 *1 Method
 *
 **************************************
 */
int concat(lua_State * L)
{
	jobject canvas, matrix;
	JNIEnv *javaEnv;
	int statement_num;

	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		if (concat_method == NULL)
			concat_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "concat",
										"(Landroid/graphics/Matrix;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		matrix = (jobject) * (jobject *) lua_touserdata(L, 2);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, concat_method, matrix);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawARGB
 *
 *1 Method
 *
 **************************************
 */
int drawARGB(lua_State * L)
{
	lua_Number a;
	lua_Number r;
	lua_Number g;
	lua_Number b;
	jobject canvas;
	int statement_num;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 5)
	{
		if (drawargb_method == NULL)
			drawargb_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawARGB",
										"(IIII)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		a = lua_tonumber(L, 2);
		r = lua_tonumber(L, 3);
		g = lua_tonumber(L, 4);
		b = lua_tonumber(L, 5);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawargb_method, (jint) a,
								   (jint) r, (jint) g, (jint) b);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawArc
 *
 *1 Method
 *
 **************************************
 */
int drawArc(lua_State * L)
{
	jobject canvas, oval, paint;
	lua_Number startAngle, sweepAngle;
	jboolean useCenter;
	int statement_num;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 6)
	{
		if (drawarc_method == NULL)
			drawarc_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawArc",
										"(Landroid/graphics/RectF;FFZLandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		oval = (jobject) * (jobject *) lua_touserdata(L, 2);
		startAngle = lua_tonumber(L, 3);
		sweepAngle = lua_tonumber(L, 4);
		useCenter = lua_toboolean(L, 5);
		paint = (jobject) * (jobject *) lua_touserdata(L, 6);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawarc_method, oval,
								   (jfloat) startAngle, (jfloat) sweepAngle,
								   (jboolean) useCenter, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}

	return 0;
}

/* 
 **************************************
 *
 *void drawBitmap 无法使用，原因不详
 *
 *6 Method(有10个参数的2个函数已合并)
 *
 **************************************
 */
int drawBitmap(lua_State * L)
{
	jclass clazz;
	jintArray colors;
	jobject canvas, bitmap, matrix, paint, src, who;
	lua_Number left, top, x, y, width, height, offset, stride;
	jboolean useCenter, hasAlpha;
	int statement_num;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 4)
	{
		if (drawbitmap1_method == NULL)
			drawbitmap1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawBitmap",
										"(Landroid/graphics/Bitmap;Landroid/graphics/Matrix;Landroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bitmap = (jobject) * (jobject *) lua_touserdata(L, 2);
		matrix = (jobject) * (jobject *) lua_touserdata(L, 3);
		paint = (jobject) * (jobject *) lua_touserdata(L, 4);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawbitmap1_method, bitmap,
								   matrix, paint);
	}
	else if (statement_num == 5)
	{
		
		/*下面这个方法最常用，第四个参数是数值时被调用*/
		if(lua_isnumber(L,4))
		{
			if (drawbitmap4_method == NULL)
				drawbitmap4_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class,
											"drawBitmap",
											"(Landroid/graphics/Bitmap;FFLandroid/graphics/Paint;)V");
			canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
			bitmap = (jobject) * (jobject *) lua_touserdata(L, 2);
			left = lua_tonumber(L, 3);
			top = lua_tonumber(L, 4);
			paint = (jobject) * (jobject *) lua_touserdata(L, 5);

			(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawbitmap4_method,
									   bitmap, (jfloat) left, (jfloat) top,
									   paint);

		}
		else if(lua_isuserdata(L,4))	
		{		
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bitmap = (jobject) * (jobject *) lua_touserdata(L, 2);
		src = (jobject) * (jobject *) lua_touserdata(L, 3);
		who = (jobject) * (jobject *) lua_touserdata(L, 4);
		paint = (jobject) * (jobject *) lua_touserdata(L, 5);
		
		
		/*如果类没有被使用过则新建类的引用，为了缓存，下同*/
		if (rect_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/Rect");
			rect_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}

		if (rectf_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/RectF");
			rectf_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		
		if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rectf_class) == JNI_TRUE)
		{
			if (drawbitmap2_method == NULL)
				drawbitmap2_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class,
											"drawBitmap",
											"(Landroid/graphics/Bitmap;Landroid/graphics/Rect;Landroid/graphics/RectF;Landroid/graphics/Paint;)V");

			(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawbitmap2_method,
									   bitmap, src, who, paint);
		}
		else if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rect_class) == JNI_TRUE)
		{
			if (drawbitmap3_method == NULL)
				drawbitmap3_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class,
											"drawBitmap",
											"(Landroid/graphics/Bitmap;Landroid/graphics/Rect;Landroid/graphics/Rect;Landroid/graphics/Paint;)V");

			(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawbitmap3_method,
									   bitmap, src, who, paint);
		}
	}
	}
	else if (statement_num == 10)
	{

		if (drawbitmap5_method == NULL)
			drawbitmap5_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class,
										"drawBitmap",
										"([IIIFFIIZ;Landroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		colors = (jintArray) * (jintArray *) lua_touserdata(L, 2);
		offset = lua_tonumber(L, 3);
		stride = lua_tonumber(L, 4);
		x = lua_tonumber(L, 5);
		y = lua_tonumber(L, 6);
		width = lua_tonumber(L, 7);
		height = lua_tonumber(L, 8);
		hasAlpha = lua_toboolean(L, 9);
		paint = (jobject) * (jobject *) lua_touserdata(L, 10);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawbitmap5_method,
								   colors, (jint) offset, (jint) stride,
								   (jfloat) x, (jfloat) y, (jint) width,
								   (jint) height, hasAlpha, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}

	return 0;
}

/* 
 **************************************
 *
 *void drawBitmapMesh
 *
 *1 Method
 *
 **************************************
 */
int drawBitmapMesh(lua_State * L)
{
	JNIEnv *javaEnv;
	jobject canvas;
	jobject bitmap;
	jobject paint;
	lua_Number meshWidth, meshHeight, vertOffset, colorOffset;
	jfloatArray verts;
	jfloatArray colors;
	int statement_num;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 9)
	{
		if (drawbitmapmesh_method == NULL)
			drawbitmapmesh_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class,
										"drawBitmapMesh",
										"(Landroid/graphics/Bitmap;II[FI[IILandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bitmap = (jobject) * (jobject *) lua_touserdata(L, 2);
		meshWidth = lua_tonumber(L, 3);
		meshHeight = lua_tonumber(L, 4);
		verts = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 5);
		vertOffset = lua_tonumber(L, 6);
		colors = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 7);
		colorOffset = lua_tonumber(L, 8);
		paint = (jobject) * (jobject *) lua_touserdata(L, 9);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawbitmapmesh_method,
								   bitmap, (jint) meshWidth,
								   (jint) meshHeight, verts, (jint) vertOffset,
								   colors, (jint) colorOffset, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawCircle
 *
 *1 Method
 *
 **************************************
 */
int drawCircle(lua_State * L)
{
	JNIEnv *javaEnv;
	jobject canvas;
	jobject paint;
	lua_Number cx, cy, radius;
	int statement_num;
	/* Gets the JNI Environment */
	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 5)
	{
		if (drawcircle_method == NULL)
			drawcircle_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawCircle",
										"(FFFLandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		cx = lua_tonumber(L, 2);
		cy = lua_tonumber(L, 3);
		radius = lua_tonumber(L, 4);
		paint = (jobject) * (jobject *) lua_touserdata(L, 5);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawcircle_method,
								   (jfloat) cx, (jfloat) cy, (jfloat) radius,
								   paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawColor
 *
 *2 Method
 *
 **************************************
 */
int drawColor(lua_State * L)
{
	JNIEnv *javaEnv;
	jobject canvas;
	jobject mode;
	int statement_num;
	lua_Number color;

	/* Gets the JNI Environment */
	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		if (drawcolor1_method == NULL)
			drawcolor1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawColor",
										"(I)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		color = lua_tonumber(L, 2);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawcolor1_method,
								   (jint) color);
	}
	else if (statement_num == 3)
	{
		if (drawcolor2_method == NULL)
			drawcolor2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawColor",
										"(ILandroid/graphics/PorterDuff.Mode)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		color = lua_tonumber(L, 2);
		mode = (jobject) * (jobject *) lua_touserdata(L, 3);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawcolor2_method,
								   (jint) color, mode);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawLine
 *
 *1 Method
 *
 **************************************
 */
int drawLine(lua_State * L)
{
	lua_Number startX;
	lua_Number startY;
	lua_Number stopX;
	lua_Number stopY;
	jobject canvas;
	jobject paint;
	JNIEnv *javaEnv;
	int statement_num;

	/* Gets the JNI Environment */
	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);

	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 6)
	{
		if (drawline_method == NULL)
			drawline_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawLine",
										"(FFFFLandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		startX = lua_tonumber(L, 2);
		startY = lua_tonumber(L, 3);
		stopX = lua_tonumber(L, 4);
		stopY = lua_tonumber(L, 5);
		paint = (jobject) * (jobject *) lua_touserdata(L, 6);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawline_method,
								   (jfloat) startX, (jfloat) startY,
								   (jfloat) stopX, (jfloat) stopY, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawLines
 *
 *2 Method
 *
 **************************************
 */
int drawLines(lua_State * L)
{
	lua_Number offset;
	lua_Number count;
	jobject canvas;
	jfloatArray pts;
	jobject paint;
	JNIEnv *javaEnv;
	int statement_num;
	/* Gets the JNI Environment */
	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);

	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		if (drawlines1_method == NULL)
			drawlines1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawLines",
										"([FLandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);

		pts = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 2);

		paint = (jobject) * (jobject *) lua_touserdata(L, 3);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawlines1_method,
								   pts, paint);
	}
	else if (statement_num == 5)
	{
		if (drawlines2_method == NULL)
			drawlines2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawLines",
										"([FIILandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		pts = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 2);
		offset = lua_tonumber(L, 3);
		count = lua_tonumber(L, 4);
		paint = (jobject) * (jobject *) lua_touserdata(L, 5);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawlines2_method,
								   pts, (jint) offset, (jint) count, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

/* 
 **************************************
 *
 *void drawOval
 *
 *1 Method
 *
 **************************************
 */
int drawOval(lua_State * L)
{
	jobject canvas;
	jobject paint;
	jobject rect;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		if (drawoval_method == NULL)
			drawoval_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawOval",
										"(Landroid/graphics/RectF;Landroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		rect = (jobject) * (jobject *) lua_touserdata(L, 2);
		paint = (jobject) * (jobject *) lua_touserdata(L, 3);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawoval_method, rect,
								   paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

 /* 
  **************************************
  *
  *void drawPaint
  *
  *1 Method
  *
  **************************************
  */
int drawPaint(lua_State * L)
{
	jobject canvas;
	jobject paint;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		if (drawpaint_method == NULL)
			drawpaint_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPaint",
										"(Landroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		paint = (jobject) * (jobject *) lua_touserdata(L, 2);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpaint_method, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

  /* 
   **************************************
   *
   *void drawPath
   *
   *1 Method
   *
   **************************************
   */
int drawPath(lua_State * L)
{
	jobject canvas;
	jobject path, paint;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		if (drawpath_method == NULL)
			drawpath_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPath",
										"(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		path = (jobject) * (jobject *) lua_touserdata(L, 2);
		paint = (jobject) * (jobject *) lua_touserdata(L, 3);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpath_method, path,
								   paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

  /* 
   **************************************
   *
   *void drawPicture 未测试
   *
   *3 Method
   *
   **************************************
   */
int drawPicture(lua_State * L)
{
	jclass clazz;
	jobject canvas;
	jobject picture, who;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		if (drawpicture1_method == NULL)
			drawpicture1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPicture",
										"(Landroid/graphics/Picture;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		picture = (jobject) * (jobject *) lua_touserdata(L, 2);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpicture1_method,
								   picture);
	}
	else if (statement_num == 3)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		picture = (jobject) * (jobject *) lua_touserdata(L, 2);
		who = (jobject) * (jobject *) lua_touserdata(L, 3);
		
		/*如果类没有被使用过则新建类的引用，为了缓存，下同*/
		if (rect_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/Rect");
			rect_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}

		if (rectf_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/RectF");
			rectf_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		
		if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rectf_class) == JNI_TRUE)
			{
		    if (drawpicture2_method == NULL)
			    drawpicture2_method =
				    (*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPicture",
										"(Landroid/graphics/Picture;Landroid/graphics/RectF;)V");
		        (*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpicture2_method,
								          picture, who);
			}
		else if ((*javaEnv)->IsInstanceOf
			(javaEnv, who,rect_class) == JNI_TRUE)
			{
		    if (drawpicture3_method == NULL)
			    drawpicture3_method =
				    (*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPicture",
										"(Landroid/graphics/Picture;Landroid/graphics/Rect;)V");
		        (*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpicture3_method,
								          picture, who);
			}
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

  /* 
   **************************************
   *
   *void drawPoint
   *
   *1 Method
   *
   **************************************
   */
int drawPoint(lua_State * L)
{
	jobject canvas;
	jobject paint;
	lua_Number x, y;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 4)
	{
		if (drawpoint_method == NULL)
			drawpoint_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPoint",
										"(FFLandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		x = lua_tonumber(L, 2);
		y = lua_tonumber(L, 3);
		paint = (jobject) * (jobject *) lua_touserdata(L, 4);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpoint_method,
								   (jfloat) x, (jfloat) y, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}

	return 0;
}

 /* 
  **************************************
  *
  *void drawPoints
  *
  *2  Method
  *
  **************************************
  */
int drawPoints(lua_State * L)
{
	jobject canvas;
	jfloatArray pts;
	jobject paint;
	lua_Number offset, count;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		if (drawpoints1_method == NULL)
			drawpoints1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPoints",
										"([FLandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		pts = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 2);
		paint = (jobject) * (jobject *) lua_touserdata(L, 3);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpoints1_method,
								   pts, paint);
	}
	else if (statement_num == 5)
	{
		if (drawpoints2_method == NULL)
			drawpoints2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPoints",
										"([FIILandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		pts = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 2);
		offset = lua_tonumber(L, 3);
		count = lua_tonumber(L, 4);
		paint = (jobject) * (jobject *) lua_touserdata(L, 5);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpoints2_method,
								   pts, (jfloat) offset, (jfloat) count,
								   paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

  /* 
   **************************************
   *
   *void drawPosText
   *
   *2 Method
   *
   **************************************
   */
int drawPosText(lua_State * L)
{
	jobject canvas;
	jstring text;
	const char *tempText;
	jcharArray char_text;
	jfloatArray pos;
	jobject paint;
	lua_Number index, count;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 4)
	{
		if (drawpostext1_method == NULL)
			drawpostext1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPosText",
										"(Ljava/lang/String;[FLandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		tempText = lua_tostring(L, 2);
		text = (*javaEnv)->NewStringUTF(javaEnv, tempText);
		pos = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 3);
		paint = (jobject) * (jobject *) lua_touserdata(L, 4);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpostext1_method,
								   text, pos, paint);
		(*javaEnv)->DeleteLocalRef(javaEnv, text);
	}
	else if (statement_num == 6)
	{
		if (drawpostext2_method == NULL)
			drawpostext2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawPosText",
										"([CII[FLandroid/graphics/Paint;)V");
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		char_text = (jcharArray) * (jcharArray *) lua_touserdata(L, 2);
		index = lua_tonumber(L, 3);
		count = lua_tonumber(L, 4);
		pos = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 5);
		paint = (jobject) * (jobject *) lua_touserdata(L, 6);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawpostext2_method,
								   char_text, (jint) index,
								   (jint) count,  pos, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

 /* 
  **************************************
  *
  *void drawRGB
  *
  *1 Method
  *
  **************************************
  */
int drawRGB(lua_State * L)
{
	lua_Number r;
	lua_Number g;
	lua_Number b;
	jobject canvas;
	int statement_num;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 4)
	{
		if (drawrgb_method == NULL)
			drawrgb_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawRGB",
										"(III)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		r = lua_tonumber(L, 2);
		g = lua_tonumber(L, 3);
		b = lua_tonumber(L, 4);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawrgb_method,
								   (jint) r, (jint) g, (jint) b);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}


/* 
 **************************************
 *
 *void drawRect
 *
 *3 Method
 *
 **************************************
 */
int drawRect(lua_State * L)
{
	lua_Number left;
	lua_Number top;
	lua_Number right;
	lua_Number bottom;
	jobject canvas;
	jobject paint;

	jobject who;

	jclass clazz;
	int statement_num;
	JNIEnv *javaEnv;

	/* lua_pcall Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	// 获取参数个数
	statement_num = lua_gettop(L);

	if (statement_num == 6)
	{
		if (drawrect1_method == NULL)
			drawrect1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawRect",
										"(FFFFLandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);

		left = lua_tonumber(L, 2);
		top = lua_tonumber(L, 3);
		right = lua_tonumber(L, 4);
		bottom = lua_tonumber(L, 5);
		paint = (jobject) * (jobject *) lua_touserdata(L, 6);
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawrect1_method,
								   (jfloat) left, (jfloat) top, (jfloat) right,
								   (jfloat) bottom, paint);
	}
	else if (statement_num == 3)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		who = (jobject) * (jobject *) lua_touserdata(L, 2);
		paint = (jobject) * (jobject *) lua_touserdata(L, 3);
		
		if (rect_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/Rect");
			rect_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}

		if (rectf_class == NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/RectF");
			rectf_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		
		if ((*javaEnv)->IsInstanceOf
			(javaEnv,who ,rectf_class) == JNI_TRUE)
		{
			if (drawrect2_method == NULL)
				drawrect2_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawRect",
											"(Landroid/graphics/RectF;Landroid/graphics/Paint;)V");

			(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawrect2_method, who,
									   paint);

		}
		else if ((*javaEnv)->IsInstanceOf
				 (javaEnv,who, rect_class) == JNI_TRUE)
		{
			if (drawrect3_method == NULL)
				drawrect3_method =
					(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawRect",
											"(Landroid/graphics/Rect;Landroid/graphics/Paint;)V");
			(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawrect3_method, who,
									   paint);
		}
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

 /* 
  **************************************
  *
  *void drawRoundRect
  *
  *1 Method
  *
  **************************************
  */
int drawRoundRect(lua_State * L)
{
	lua_Number rx;
	lua_Number ry;
	jobject rect;
	jobject canvas, paint;
	int statement_num;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 5)
	{
		if (drawroundrect_method == NULL)
			drawroundrect_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawRoundRect",
										"(Landroid/graphics/RectF;FFLandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		rect = (jobject) * (jobject *) lua_touserdata(L, 2);
		rx = lua_tonumber(L, 3);
		ry = lua_tonumber(L, 4);
		paint = (jobject) * (jobject *) lua_touserdata(L, 5);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawroundrect_method,
								   rect, (jfloat) rx, (jfloat) ry, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
  /* 
   **************************************
   *
   *void drawText
   *
   *4 Method
   *
   **************************************
   */
int drawText(lua_State * L)
{
	jclass clazz;
	jobject canvas;
	jstring text;
	jobject text2;
	jcharArray text3;
	const char *tempText;
	jfloatArray pos;
	jobject who,paint;
	lua_Number x, y,start,end,index,count;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 5)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		tempText = lua_tostring(L, 2);
		text = (*javaEnv)->NewStringUTF(javaEnv, tempText);
		x= lua_tonumber(L, 3);
		y= lua_tonumber(L, 4);
		paint = (jobject) * (jobject *) lua_touserdata(L, 5);
		if (drawtext1_method == NULL)
			drawtext1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawText",
										"(Ljava/lang/String;FFLandroid/graphics/Paint;)V");
		

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawtext1_method,
								   text, (jfloat)x,(jfloat)y, paint);
		(*javaEnv)->DeleteLocalRef(javaEnv, text);
	}
	else if (statement_num == 7)
	{
		
		if(charsequence_class==NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "java/lang/CharSequence");
			charsequence_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		
		if(lua_isstring(L,2))
		{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		tempText = lua_tostring(L, 2);
		text = (*javaEnv)->NewStringUTF(javaEnv, tempText);
		index = lua_tonumber(L, 3);
		count = lua_tonumber(L, 4);
		x = lua_tonumber(L, 5);
		y = lua_tonumber(L, 6);
		paint = (jobject) * (jobject *) lua_touserdata(L, 7);
		
		    if (drawtext2_method == NULL)
			   drawtext2_method =
				  (*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawText",
										"(Ljava/lang/String;IIFFLandroid/graphics/Paint;)V");
										
		       (*javaEnv)->CallVoidMethod(javaEnv, canvas, drawtext2_method,
								   text, (jint) index,
								   (jint) count, (jfloat) x,(jfloat)y, paint);
			   ( *javaEnv )->DeleteLocalRef( javaEnv , text);
	   }
	   else if((*javaEnv)->IsInstanceOf
		(javaEnv,(jobject) * (jobject *) lua_touserdata(L, 2) ,charsequence_class) == JNI_TRUE){
		   
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		text2 =(jobject) * (jobject *) lua_touserdata(L, 2);
		start = lua_tonumber(L, 3);
		end = lua_tonumber(L, 4);
		x = lua_tonumber(L, 5);
		y = lua_tonumber(L, 6);
		paint = (jobject) * (jobject *) lua_touserdata(L, 7);
		  if (drawtext3_method == NULL)
			   drawtext3_method =
				  (*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawText",
										"(Ljava/lang/CharSequence;IIFFLandroid/graphics/Paint;)V");
				  (*javaEnv)->CallVoidMethod(javaEnv, canvas, drawtext3_method,
								   text2, (jint) start,
								   (jint) end, (jfloat) x,(jfloat)y, paint);
		
	   }
	   else{
	    canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		text3 =(jcharArray) * (jcharArray *) lua_touserdata(L, 2);
		index = lua_tonumber(L, 3);
		count = lua_tonumber(L, 4);
		x = lua_tonumber(L, 5);
		y = lua_tonumber(L, 6);
		paint = (jobject) * (jobject *) lua_touserdata(L, 7);
		  if (drawtext4_method == NULL)
			   drawtext4_method =
				  (*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawText",
										"([CIIFFLandroid/graphics/Paint;)V");
				  (*javaEnv)->CallVoidMethod(javaEnv, canvas, drawtext4_method,
								   text3, (jint) index,
								   (jint) count, (jfloat) x,(jfloat)y, paint);
	   }
			   
	}
	
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
  /* 
   **************************************
   *
   *void drawTextOnPath 未测试
   *
   *2 Method
   *
   **************************************
   */
int drawTextOnPath(lua_State * L)
{
	jobject canvas;
	jstring text;
	jcharArray text2;
	const char *tempText;
	jobject path;
	jobject paint;
	lua_Number hOffset,vOffset,index,count;
	JNIEnv *javaEnv;
	int statement_num;

	statement_num = lua_gettop(L);
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 6)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		tempText = lua_tostring(L, 2);
		text = (*javaEnv)->NewStringUTF(javaEnv, tempText);
		path = (jobject) * (jobject *) lua_touserdata(L, 3);
		hOffset= lua_tonumber(L, 4);
		vOffset= lua_tonumber(L, 5);
		paint = (jobject) * (jobject *) lua_touserdata(L, 6);
		if (drawtextonpath1_method == NULL)
			drawtextonpath1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawTextOnPath",
										"(Ljava/lang/String;Landroid/graphics/Path;FFLandroid/graphics/Paint;)V");
		

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawtextonpath1_method,
								   text,path, (jfloat)hOffset,(jfloat)vOffset, paint);
		(*javaEnv)->DeleteLocalRef(javaEnv, text);
	}
	else if(statement_num == 8)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		text2 = (jcharArray) * (jcharArray *) lua_touserdata(L, 2);
		index=lua_tonumber(L, 3);
		count=lua_tonumber(L, 4);
		path = (jobject) * (jobject *) lua_touserdata(L, 5);
		hOffset= lua_tonumber(L, 6);
		vOffset= lua_tonumber(L, 7);
		paint = (jobject) * (jobject *) lua_touserdata(L, 8);
		if (drawtextonpath2_method == NULL)
			drawtextonpath2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawTextOnPath",
										"([CIILandroid/graphics/Path;FFLandroid/graphics/Paint;)V");
										
		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawtextonpath2_method,
								   text2,(jint)index,(jint)count,path, (jfloat)hOffset,(jfloat)vOffset, paint);
		
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void drawVertices 未测试
  *
  *1 Method(参数最多的方法)
  *
  **************************************
  */
int drawVertices(lua_State * L)
{
	lua_Number vertexCount,vertOffset,texOffset,colorOffset,indexOffset,indexCount;
	jfloatArray verts,texs;
	jintArray colors;
	jshortArray indices;
	jobject canvas, mode,paint;
	int statement_num;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 13)
	{
		if (drawvertices_method == NULL)
			drawvertices_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawVertices",
										"(Landroid/graphics/Canvas.VertexMode;I[FI[FI[II[SIILandroid/graphics/Paint;)V");

		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		mode = (jobject) * (jobject *) lua_touserdata(L, 2);
		vertexCount = lua_tonumber(L, 3);
		verts = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 4);
		vertOffset = lua_tonumber(L, 5);
		texs = (jfloatArray) * (jfloatArray *) lua_touserdata(L, 6);
		texOffset = lua_tonumber(L, 7);
		colors = (jintArray) * (jintArray *) lua_touserdata(L, 8);
		colorOffset = lua_tonumber(L, 9);
		indices = (jshortArray) * (jshortArray *) lua_touserdata(L, 10);
		indexOffset = lua_tonumber(L, 11);
		indexCount = lua_tonumber(L, 12);
		paint = (jobject) * (jobject *) lua_touserdata(L, 13);

		(*javaEnv)->CallVoidMethod(javaEnv, canvas, drawvertices_method,
								   mode, (jint) vertexCount, verts,(jint)vertOffset,texs,(jint)texOffset,colors,
								  (jint)colorOffset,indices,(jint)indexOffset,(jint)indexCount, paint);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

 /* 
  **************************************
  *
  *final Rect getClipBounds()
  *boolean getClipBounds(Rect bounds)
  *
  *2 Method
  *
  **************************************
  */
int getClipBounds(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,bounds;
	jboolean ret2;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getclipbounds1_method == NULL)
			getclipbounds1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getClipBounds","()Landroid/graphics/Rect;");
				const jobject ret=(*javaEnv)->CallObjectMethod(javaEnv, canvas,getclipbounds1_method);
				lua_pushlightuserdata(L,ret);
	}
    else if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bounds = (jobject) * (jobject *) lua_touserdata(L, 2);
		if (getclipbounds2_method == NULL)
			getclipbounds2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getClipBounds","(Landroid/graphics/Rect;)Z");
				 ret2=(*javaEnv)->CallBooleanMethod(javaEnv, canvas,getclipbounds2_method,bounds);
				lua_pushboolean(L,ret2);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *int getDensity
  *
  *1 Method
  *
  **************************************
  */
int getDensity(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getdensity_method == NULL)
			getdensity_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getDensity","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,getdensity_method);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *int getHeight()
  *
  *1 Method
  *
  **************************************
  */
int getHeight(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jint ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getheight_method == NULL)
			getheight_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getHeight","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,getheight_method);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
  /* 
  **************************************
  *
  *未测试
  *final Matrix getMatrix()
  *void getMatrix(Matrix ctm)
  *
  *2 Method
  *
  **************************************
  */
int getMatrix(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,ctm;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getmatrix1_method == NULL)
			getmatrix1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getMatrix","()Landroid/graphics/Matrix;");
				const jobject ret=(*javaEnv)->CallObjectMethod(javaEnv, canvas,getmatrix1_method);
				lua_pushlightuserdata(L,ret);
			return 1;
	}
    else if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		ctm = (jobject) * (jobject *) lua_touserdata(L, 2);
		if (getmatrix2_method == NULL)
			getmatrix2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getMatrix","(Landroid/graphics/Matrix;)V");
				 (*javaEnv)->CallBooleanMethod(javaEnv, canvas,getmatrix2_method,ctm);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *int getMaximumBitmapHeight()
  *
  *1 Method
  *
  **************************************
  */
int getMaximumBitmapHeight(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jint ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getmaximumbitmapheight_method == NULL)
			getmaximumbitmapheight_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getMaximumBitmapHeight","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,getmaximumbitmapheight_method);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *int getMaximumBitmapWidth()
  *
  *1 Method
  *
  **************************************
  */
int getMaximumBitmapWidth(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jint ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getmaximumbitmapwidth_method == NULL)
			getmaximumbitmapwidth_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getMaximumBitmapWidth","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,getmaximumbitmapwidth_method);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *int getSaveCount()
  *
  *1 Method
  *
  **************************************
  */
int getSaveCount(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jint ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getsavecount_method == NULL)
			getsavecount_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getSaveCount","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,getsavecount_method);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
/* 
  **************************************
  *
  *int getWidth()
  *
  *1 Method
  *
  **************************************
  */
int getWidth(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jint ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (getwidth_method == NULL)
			getwidth_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "getWidth","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,getwidth_method);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *boolean isHardwareAccelerated
  *
  *1 Method
  *
  **************************************
  */
int isHardwareAccelerated(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jboolean ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (ishardwareaccelerated_method == NULL)
			ishardwareaccelerated_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "isHardwareAccelerated","()Z");
				ret =(*javaEnv)->CallBooleanMethod(javaEnv, canvas,ishardwareaccelerated_method);
				lua_pushboolean(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *boolean isOpaque
  *
  *1 Method
  *
  **************************************
  */
int isOpaque(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jboolean ret;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (isopaque_method == NULL)
			isopaque_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "isOpaque","()Z");
				ret =(*javaEnv)->CallBooleanMethod(javaEnv, canvas,isopaque_method);
				lua_pushboolean(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *boolean quickReject 未测试
  *
  *3 Method
  *
  **************************************
  */
int quickReject(lua_State * L)
{
	jclass clazz;
	jobject who,type;
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	jboolean ret;
	lua_Number left,top,right,bottom;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		if(path_class==NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "java/lang/path");
			path_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		if(rectf_class==NULL)
		{
			clazz = (*javaEnv)->FindClass(javaEnv, "java/lang/RectF");
			rectf_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
			(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
		}
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		who = (jobject) * (jobject *) lua_touserdata(L, 2);
		type = (jobject) * (jobject *) lua_touserdata(L, 3);
		if((*javaEnv)->IsInstanceOf(javaEnv,who,path_class)==JNI_TRUE)
		{
		  if (quickreject1_method == NULL)
		      quickreject1_method =
		         (*javaEnv)->GetMethodID(javaEnv, canvas_class, "quickReject","(Landroid/graphics/Path;Landroid/graphics/Canvas.EdgeType;)Z");
			   ret =(*javaEnv)->CallBooleanMethod(javaEnv, canvas,quickreject1_method,who,type);
		}
		else if((*javaEnv)->IsInstanceOf(javaEnv,who,rect_class)==JNI_TRUE)
		{
		  if (quickreject2_method == NULL)
			  quickreject2_method =
		      (*javaEnv)->GetMethodID(javaEnv, canvas_class, "quickReject","(Landroid/graphics/RectF;Landroid/graphics/Canvas.EdgeType;)Z");
			   ret =(*javaEnv)->CallBooleanMethod(javaEnv, canvas,quickreject2_method,who,type);
		}
		lua_pushboolean(L,ret);
			
	}
	else if(statement_num == 6)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		left =  lua_tonumber(L, 2);
		top =  lua_tonumber(L, 3);
		right =  lua_tonumber(L, 4);
		bottom =  lua_tonumber(L, 5);
		type = (jobject) * (jobject *) lua_touserdata(L, 6);
		if (quickreject3_method == NULL)
			  quickreject3_method =
		      (*javaEnv)->GetMethodID(javaEnv, canvas_class, "quickReject","(FFFFLandroid/graphics/Canvas.EdgeType;)Z");
			   ret =(*javaEnv)->CallBooleanMethod(javaEnv, canvas,quickreject3_method,
			        (jfloat)left,(jfloat)top,(jfloat)right,(jfloat)bottom,type);
		lua_pushboolean(L,ret);
		
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *void restore
  *
  *1 Method
  *
  **************************************
  */
int restore(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		if (restore_method == NULL)
			restore_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "restore","()V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,restore_method);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void restoreToCount
  *
  *1 Method
  *
  **************************************
  */
int restoreToCount(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number saveCount;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		saveCount=lua_tonumber(L,2);
		if (restoretocount_method == NULL)
			restoretocount_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "restoreToCount","()V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,restoretocount_method,(jint)saveCount);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void rotate
  *
  *2 Method
  *
  **************************************
  */
int rotate(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number degrees,px,py;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		degrees=lua_tonumber(L,2);
		
		if (rotate1_method == NULL)
			rotate1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "rotate","(F)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,rotate1_method,
				   (jfloat)degrees);
	}
	else if (statement_num == 4)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		degrees=lua_tonumber(L,2);
		px=lua_tonumber(L,3);
		py=lua_tonumber(L,4);
		if (rotate2_method == NULL)
			rotate2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "rotate","(FFF)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,rotate2_method,
				    (jfloat)degrees,(jfloat)px,(jfloat)py);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /*
  **************************************
  *
  *void save
  *
  *2 Method
  *
  **************************************
  */
int save(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number saveFlags;
	lua_Number ret;
	
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 1)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		
		if (save1_method == NULL)
			save1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "save","()I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,save1_method);
				lua_pushnumber(L,ret);
	}
	else if (statement_num == 4)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		saveFlags=lua_tonumber(L,2);
		
		if (save2_method == NULL)
			save2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "save","(I)I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,save2_method,
				    (jint)saveFlags);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /*
  **************************************
  *
  *int saveLayer
  *
  *2 Method
  *
  **************************************
  */
int saveLayer(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,bounds,paint;
	lua_Number saveFlags,left,top,right,bottom;
	lua_Number ret;
	
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 4)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bounds = (jobject) * (jobject *) lua_touserdata(L, 2);
		paint = (jobject) * (jobject *) lua_touserdata(L, 3);
		saveFlags = lua_tonumber(L,4);
		if (savelayer1_method == NULL)
			savelayer1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "saveLayer","(Landroid/graphics/RectF;Landroid/graphics/Paint;I)I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,savelayer1_method,
				bounds,paint,(jint)saveFlags);
				lua_pushnumber(L,ret);
	}
	else if (statement_num == 7)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		left=lua_tonumber(L,2);
		top=lua_tonumber(L,3);
		right=lua_tonumber(L,4);
		bottom=lua_tonumber(L,5);
		paint = (jobject) * (jobject *) lua_touserdata(L, 6);
		saveFlags=lua_tonumber(L,7);
		
		if (savelayer2_method == NULL)
			savelayer2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "saveLayer","(FFFFLandroid/graphics/Paint;I)I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,savelayer2_method,
				    (jfloat)left,(jfloat)top,(jfloat)right,(jfloat)bottom,
					paint,(jint)saveFlags);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
/*
  **************************************
  *
  *int saveLayerAlpha
  *
  *2 Method
  *
  **************************************
  */
int saveLayerAlpha(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,bounds;
	lua_Number saveFlags,alpha,left,top,right,bottom;
	lua_Number ret;
	
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 4)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bounds = (jobject) * (jobject *) lua_touserdata(L, 2);
        alpha = lua_tonumber(L,3);
		saveFlags = lua_tonumber(L,4);
		if (savelayeralpha1_method == NULL)
			savelayeralpha1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "saveLayerAlpha","(Landroid/graphics/RectF;II)I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,savelayeralpha1_method,
				bounds,(jint)alpha,(jint)saveFlags);
				lua_pushnumber(L,ret);
	}
	else if (statement_num == 7)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		left=lua_tonumber(L,2);
		top=lua_tonumber(L,3);
		right=lua_tonumber(L,4);
		bottom=lua_tonumber(L,5);
		alpha=lua_tonumber(L,6);
		saveFlags=lua_tonumber(L,7);
		
		if (savelayeralpha2_method == NULL)
			savelayeralpha2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "saveLayerAlpha","(FFFFII)I");
				ret=(*javaEnv)->CallIntMethod(javaEnv, canvas,savelayeralpha2_method,
				    (jfloat)left,(jfloat)top,(jfloat)right,(jfloat)bottom,
					(jint)alpha,(jint)saveFlags);
				lua_pushnumber(L,ret);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}

/*
  **************************************
  *
  *void scale
  *
  *2 Method
  *
  **************************************
  */
int scale(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number sx,sy,px,py;
	
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		sx = lua_tonumber(L,2);
        sy = lua_tonumber(L,3);
		if (scale1_method == NULL)
			scale1_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "scale","(FF)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,scale1_method,
				(jfloat)sx,(jfloat)sy);
	}
	else if (statement_num == 5)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		sx = lua_tonumber(L,2);
        sy = lua_tonumber(L,3);
		px = lua_tonumber(L,4);
        py = lua_tonumber(L,5);
		
		if (scale2_method == NULL)
			scale2_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "scale","(FFFF)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,scale2_method,
				    (jfloat)sx,(jfloat)sy,(jfloat)px,(jfloat)py);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 1;
}
 /* 
  **************************************
  *
  *void setBitmap
  *
  *1 Method
  *
  **************************************
  */
int setBitmap(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,bitmap;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		bitmap = (jobject) * (jobject *) lua_touserdata(L, 2);
		if (setbitmap_method == NULL)
			setbitmap_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "setBitmap","(Landroid/graphics/Bitmap;)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,setbitmap_method,bitmap);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void setDensity
  *
  *1 Method
  *
  **************************************
  */
int setDensity(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number density;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		density = lua_tonumber(L, 2);
		if (setdensity_method == NULL)
			setdensity_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "setDensity","(I)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,setdensity_method,(jint)density);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void setDrawFilter 未测试
  *
  *1 Method
  *
  **************************************
  */
int setDrawFilter(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,filter;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		filter = (jobject) * (jobject *) lua_touserdata(L, 2);
		if (setdrawfilter_method == NULL)
			setdrawfilter_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "setDrawFilter","(Landroid/graphics/DrawFilter;)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,setdrawfilter_method,filter);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void setMatrix
  *
  *1 Method
  *
  **************************************
  */
int setMatrix(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas,matrix;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 2)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		matrix = (jobject) * (jobject *) lua_touserdata(L, 2);
		if (setmatrix_method == NULL)
			setmatrix_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "setMatrix","(Landroid/graphics/Matrix;)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,setmatrix_method,matrix);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void skew
  *
  *1 Method
  *
  **************************************
  */
int skew(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number sx,sy;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		sx = lua_tonumber(L, 2);
		sy = lua_tonumber(L, 3);
		if (skew_method == NULL)
			skew_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "skew","(FF)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,skew_method,
				(jfloat)sx,(jfloat)sy);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}
 /* 
  **************************************
  *
  *void translate
  *
  *1 Method
  *
  **************************************
  */
int translate(lua_State * L)
{
	int statement_num;
	JNIEnv *javaEnv;
	jobject canvas;
	lua_Number dx,dy;
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	statement_num = lua_gettop(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	if (statement_num == 3)
	{
		canvas = (jobject) * (jobject *) lua_touserdata(L, 1);
		dx = lua_tonumber(L, 2);
		dy = lua_tonumber(L, 3);
		if (translate_method == NULL)
			translate_method =
				(*javaEnv)->GetMethodID(javaEnv, canvas_class, "translate","(FF)V");
				(*javaEnv)->CallVoidMethod(javaEnv, canvas,translate_method,
				(jfloat)dx,(jfloat)dy);
	}
	else
	{
		lua_pushstring(L, "Error. Invalid number of parameters.");
		lua_error(L);
	}
	return 0;
}

int getCanvas(lua_State * L)
{
	jclass clazz;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	canvas_class = (*javaEnv)->FindClass(javaEnv, "android/graphics/Canvas");
	drawline_method =
		(*javaEnv)->GetMethodID(javaEnv, canvas_class, "drawLine",
								"(FFFFLandroid/graphics/Paint;)V");
	lua_getmetatable(L, 1);
	lua_pushstring(L, LUAINDEXMETAMETHODTAG);
	lua_newtable(L);
	
	lua_pushstring(L, "clipPath");
	lua_pushcfunction(L, &clipPath);
	lua_settable(L, -3);

	lua_pushstring(L, "clipRect");
	lua_pushcfunction(L, &clipRect);
	lua_settable(L, -3);

	lua_pushstring(L, "clipRegion");
	lua_pushcfunction(L, &clipRegion);
	lua_settable(L, -3);

	lua_pushstring(L, "concat");
	lua_pushcfunction(L, &concat);
	lua_settable(L, -3);

	lua_pushstring(L, "drawArc");
	lua_pushcfunction(L, &drawArc);
	lua_settable(L, -3);

	lua_pushstring(L, "drawARGB");
	lua_pushcfunction(L, &drawARGB);
	lua_settable(L, -3);

	lua_pushstring(L, "drawBitmap");
	lua_pushcfunction(L, &drawBitmap);
	lua_settable(L, -3);

	lua_pushstring(L, "drawBitmapMesh");
	lua_pushcfunction(L, &drawBitmap);
	lua_settable(L, -3);

	lua_pushstring(L, "drawCircle");
	lua_pushcfunction(L, &drawCircle);
	lua_settable(L, -3);

	lua_pushstring(L, "drawColor");
	lua_pushcfunction(L, &drawColor);
	lua_settable(L, -3);

	lua_pushstring(L, "drawLine");
	lua_pushcfunction(L, &drawLine);
	lua_settable(L, -3);

	lua_pushstring(L, "drawLines");
	lua_pushcfunction(L, &drawLines);
	lua_settable(L, -3);

	lua_pushstring(L, "drawOval");
	lua_pushcfunction(L, &drawOval);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPaint");
	lua_pushcfunction(L, &drawPaint);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPath");
	lua_pushcfunction(L, &drawPath);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPicture");
	lua_pushcfunction(L, &drawPicture);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPoint");
	lua_pushcfunction(L, &drawPoint);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPoints");
	lua_pushcfunction(L, &drawPoints);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPosText");
	lua_pushcfunction(L, &drawPosText);
	lua_settable(L, -3);

	lua_pushstring(L, "drawRGB");
	lua_pushcfunction(L, &drawRGB);
	lua_settable(L, -3);

	lua_pushstring(L, "drawRect");
	lua_pushcfunction(L, &drawRect);
	lua_settable(L, -3);

	lua_pushstring(L, "drawRoundRect");
	lua_pushcfunction(L, &drawRoundRect);
	lua_settable(L, -3);

	lua_pushstring(L, "drawText");
	lua_pushcfunction(L, &drawText);
	lua_settable(L, -3);
	
	lua_pushstring(L, "drawTextOnPath");
	lua_pushcfunction(L, &drawTextOnPath);
	lua_settable(L, -3);
	
	lua_pushstring(L, "drawVertices");
	lua_pushcfunction(L, &drawVertices);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getClipBounds");
	lua_pushcfunction(L, &getClipBounds);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getDensity");
	lua_pushcfunction(L, &getDensity);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getHeight");
	lua_pushcfunction(L, &getHeight);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getMatrix");
	lua_pushcfunction(L, &getMatrix);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getMaximumBitmapHeight");
	lua_pushcfunction(L, &getMaximumBitmapHeight);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getMaximumBitmapWidth");
	lua_pushcfunction(L, &getMaximumBitmapWidth);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getSaveCount");
	lua_pushcfunction(L, &getSaveCount);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getWidth");
	lua_pushcfunction(L, &getWidth);
	lua_settable(L, -3);
	
	lua_pushstring(L, "isHardwareAccelerated");
	lua_pushcfunction(L, &isHardwareAccelerated);
	lua_settable(L, -3);
	
	lua_pushstring(L, "isOpaque");
	lua_pushcfunction(L, &isOpaque);
	lua_settable(L, -3);
	
	lua_pushstring(L, "quickReject");
	lua_pushcfunction(L, &quickReject);
	lua_settable(L, -3);
	
	lua_pushstring(L, "restore");
	lua_pushcfunction(L, &restore);
	lua_settable(L, -3);
	
	lua_pushstring(L, "restoreToCount");
	lua_pushcfunction(L, &restoreToCount);
	lua_settable(L, -3);
	
	lua_pushstring(L, "rotate");
	lua_pushcfunction(L, &rotate);
	lua_settable(L, -3);
	
	lua_pushstring(L, "save");
	lua_pushcfunction(L, &save);
	lua_settable(L, -3);
	
	lua_pushstring(L, "saveLayer");
	lua_pushcfunction(L, &saveLayer);
	lua_settable(L, -3);
	
	lua_pushstring(L, "saveLayerAlpha");
	lua_pushcfunction(L, &saveLayerAlpha);
	lua_settable(L, -3);
	
	lua_pushstring(L, "scale");
	lua_pushcfunction(L, &scale);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setBitmap");
	lua_pushcfunction(L, &setBitmap);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setDensity");
	lua_pushcfunction(L, &setDensity);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setDrawFilter");
	lua_pushcfunction(L, &setDrawFilter);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setMatrix");
	lua_pushcfunction(L, &setMatrix);
	lua_settable(L, -3);
	
	lua_pushstring(L, "skew");
	lua_pushcfunction(L, &skew);
	lua_settable(L, -3);
	
	lua_pushstring(L, "translate");
	lua_pushcfunction(L, &translate);
	lua_settable(L, -3);
	
	lua_rawset(L, -3);
	lua_pop(L, 1);
	return 1;
}

int lockCanvas(lua_State * L)
{
	jclass clazz;
	jobject holder, obj, canvas;
	jobject *userData;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	userData = (jobject *) lua_touserdata(L, 1);
	holder = (jobject) * userData;

	if (holder_class == NULL)
	{
		clazz = (*javaEnv)->GetObjectClass(javaEnv, holder);
		holder_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
		(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
	}

	if (lockcanvas_method == NULL)
		lockcanvas_method =
			(*javaEnv)->GetMethodID(javaEnv, holder_class, "lockCanvas",
									"()Landroid/graphics/Canvas;");

	if (canvas_class == NULL)
	{
		clazz = (*javaEnv)->FindClass(javaEnv, "android/graphics/Canvas");
		canvas_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
		(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
	}

	obj = (*javaEnv)->CallObjectMethod(javaEnv, holder, lockcanvas_method);
	canvas = (*javaEnv)->NewGlobalRef(javaEnv, obj);
	(*javaEnv)->DeleteLocalRef(javaEnv, obj);


	lua_settop(L, 0);

	userData = (jobject *) lua_newuserdata(L, sizeof(jobject));
	*userData = canvas;

	lua_newtable(L);
	lua_pushstring(L, LUAINDEXMETAMETHODTAG);
	lua_newtable(L);

	lua_pushstring(L, "clipPath");
	lua_pushcfunction(L, &clipPath);
	lua_settable(L, -3);

	lua_pushstring(L, "clipRect");
	lua_pushcfunction(L, &clipRect);
	lua_settable(L, -3);

	lua_pushstring(L, "clipRegion");
	lua_pushcfunction(L, &clipRegion);
	lua_settable(L, -3);

	lua_pushstring(L, "concat");
	lua_pushcfunction(L, &concat);
	lua_settable(L, -3);

	lua_pushstring(L, "drawARGB");
	lua_pushcfunction(L, &drawARGB);
	lua_settable(L, -3);

	lua_pushstring(L, "drawArc");
	lua_pushcfunction(L, &drawArc);
	lua_settable(L, -3);

	lua_pushstring(L, "drawBitmap");
	lua_pushcfunction(L, &drawBitmap);
	lua_settable(L, -3);

	lua_pushstring(L, "drawBitmapMesh");
	lua_pushcfunction(L, &drawBitmap);
	lua_settable(L, -3);

	lua_pushstring(L, "drawCircle");
	lua_pushcfunction(L, &drawCircle);
	lua_settable(L, -3);

	lua_pushstring(L, "drawColor");
	lua_pushcfunction(L, &drawColor);
	lua_settable(L, -3);

	lua_pushstring(L, "drawLine");
	lua_pushcfunction(L, &drawLine);
	lua_settable(L, -3);

	lua_pushstring(L, "drawLines");
	lua_pushcfunction(L, &drawLines);
	lua_settable(L, -3);

	lua_pushstring(L, "drawOval");
	lua_pushcfunction(L, &drawOval);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPaint");
	lua_pushcfunction(L, &drawPaint);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPath");
	lua_pushcfunction(L, &drawPath);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPicture");
	lua_pushcfunction(L, &drawPicture);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPoint");
	lua_pushcfunction(L, &drawPoint);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPoints");
	lua_pushcfunction(L, &drawPoints);
	lua_settable(L, -3);

	lua_pushstring(L, "drawPosText");
	lua_pushcfunction(L, &drawPosText);
	lua_settable(L, -3);

	lua_pushstring(L, "drawRGB");
	lua_pushcfunction(L, &drawRGB);
	lua_settable(L, -3);

	lua_pushstring(L, "drawRect");
	lua_pushcfunction(L, &drawRect);
	lua_settable(L, -3);

	lua_pushstring(L, "drawRoundRect");
	lua_pushcfunction(L, &drawRoundRect);
	lua_settable(L, -3);

	lua_pushstring(L, "drawText");
	lua_pushcfunction(L, &drawText);
	lua_settable(L, -3);
	
	lua_pushstring(L, "drawTextOnPath");
	lua_pushcfunction(L, &drawTextOnPath);
	lua_settable(L, -3);
	
	lua_pushstring(L, "drawVertices");
	lua_pushcfunction(L, &drawVertices);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getClipBounds");
	lua_pushcfunction(L, &getClipBounds);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getDensity");
	lua_pushcfunction(L, &getDensity);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getHeight");
	lua_pushcfunction(L, &getHeight);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getMatrix");
	lua_pushcfunction(L, &getMatrix);
	lua_settable(L, -3);
		
	lua_pushstring(L, "getMaximumBitmapHeight");
	lua_pushcfunction(L, &getMaximumBitmapHeight);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getMaximumBitmapWidth");
	lua_pushcfunction(L, &getMaximumBitmapWidth);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getSaveCount");
	lua_pushcfunction(L, &getSaveCount);
	lua_settable(L, -3);
	
	lua_pushstring(L, "getWidth");
	lua_pushcfunction(L, &getWidth);
	lua_settable(L, -3);
	
	lua_pushstring(L, "isHardwareAccelerated");
	lua_pushcfunction(L, &isHardwareAccelerated);
	lua_settable(L, -3);
	
	lua_pushstring(L, "isOpaque");
	lua_pushcfunction(L, &isOpaque);
	lua_settable(L, -3);
	
	lua_pushstring(L, "quickReject");
	lua_pushcfunction(L, &quickReject);
	lua_settable(L, -3);
	
	lua_pushstring(L, "restore");
	lua_pushcfunction(L, &restore);
	lua_settable(L, -3);
	
	lua_pushstring(L, "restoreToCount");
	lua_pushcfunction(L, &restoreToCount);
	lua_settable(L, -3);
	
	lua_pushstring(L, "rotate");
	lua_pushcfunction(L, &rotate);
	lua_settable(L, -3);
	
	lua_pushstring(L, "save");
	lua_pushcfunction(L, &save);
	lua_settable(L, -3);
	
	lua_pushstring(L, "saveLayer");
	lua_pushcfunction(L, &saveLayer);
	lua_settable(L, -3);
	
	lua_pushstring(L, "saveLayerAlpha");
	lua_pushcfunction(L, &saveLayerAlpha);
	lua_settable(L, -3);
	
	lua_pushstring(L, "scale");
	lua_pushcfunction(L, &scale);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setBitmap");
	lua_pushcfunction(L, &setBitmap);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setDensity");
	lua_pushcfunction(L, &setDensity);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setDrawFilter");
	lua_pushcfunction(L, &setDrawFilter);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setMatrix");
	lua_pushcfunction(L, &setMatrix);
	lua_settable(L, -3);
	
	lua_pushstring(L, "skew");
	lua_pushcfunction(L, &skew);
	lua_settable(L, -3);
	
	lua_pushstring(L, "translate");
	lua_pushcfunction(L, &translate);
	lua_settable(L, -3);
	
	lua_rawset(L, -3);
	lua_pushstring(L, LUAJAVAOBJECTIND);
	lua_pushboolean(L, 1);
	lua_rawset(L, -3);
	lua_setmetatable(L, -2);
	// lua_pop( L , 1 );
	return 1;
}

int unlockCanvasAndPost(lua_State * L)
{
	jclass clazz;
	jobject holder, canvas;
	jobject *userData;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	userData = (jobject *) lua_touserdata(L, 1);
	holder = (jobject) * userData;
	userData = (jobject *) lua_touserdata(L, 2);
	canvas = (jobject) * userData;

	if (holder_class == NULL)
	{
		clazz = (*javaEnv)->GetObjectClass(javaEnv, holder);
		holder_class = (*javaEnv)->NewGlobalRef(javaEnv, clazz);
		(*javaEnv)->DeleteLocalRef(javaEnv, clazz);
	}

	if (unlockcanvas_method == NULL)
		unlockcanvas_method =
			(*javaEnv)->GetMethodID(javaEnv, holder_class,
									"unlockCanvasAndPost",
									"(Landroid/graphics/Canvas;)V");

	(*javaEnv)->CallVoidMethod(javaEnv, holder, unlockcanvas_method, canvas);

	return 0;
}

JNIEnv *getEnvFromState(lua_State * L)
{
	JNIEnv **udEnv;

	lua_pushstring(L, LUAJAVAJNIENVTAG);
	lua_rawget(L, LUA_REGISTRYINDEX);

	if (!lua_isuserdata(L, -1))
	{
		lua_pop(L, 1);
		return NULL;
	}

	udEnv = (JNIEnv **) lua_touserdata(L, -1);
	lua_pop(L, 1);
	return *udEnv;
}

int isJavaObject(lua_State * L, int idx)
{
	if (!lua_isuserdata(L, idx))
		return 0;

	if (lua_getmetatable(L, idx) == 0)
		return 0;

	lua_pushstring(L, LUAJAVAOBJECTIND);
	lua_rawget(L, -2);

	if (lua_isnil(L, -1))
	{
		lua_pop(L, 2);
		return 0;
	}
	lua_pop(L, 2);
	return 1;
}

int _EXPORT luaopen_canvas(lua_State * L)
{
	static const struct luaL_reg funcs[] = {
		{"get", getCanvas},
		{"clipPath", clipPath},
		{"clipRect", clipRect},
		{"clipRegion", clipRegion},
		{"concat", concat},
		{"drawARGB", drawARGB},
		{"drawArc", drawArc},
		{"drawBitmap", drawBitmap},
		{"drawBitmapMesh", drawBitmapMesh},
		{"drawCircle", drawCircle},
		{"drawColor", drawColor},
		{"drawLine", drawLine},
		{"drawLines", drawLines},
		{"drawOval", drawOval},
		{"drawPaint", drawPaint},
		{"drawPath", drawPath},
		{"drawPicture", drawPicture},
		{"drawPoint", drawPoint},
		{"drawPoints", drawPoints},
		{"drawPosText", drawPosText},
		{"drawRGB", drawRGB},
		{"drawRect", drawRect},
		{"drawRoundRect", drawRoundRect},
        {"drawText", drawText},
		{"drawTextOnPath",drawTextOnPath},
		{"drawVertices",drawVertices},
		{"getClipBounds",getClipBounds},
		{"getDensity",getDensity},
		{"getHeight",getHeight},
		{"getMatrix",getMatrix},
		{"getMaximumBitmapHeight",getMaximumBitmapHeight},
		{"getMaximumBitmapWidth",getMaximumBitmapWidth},
		{"getSaveCount",getSaveCount},
		{"getWidth",getWidth},
		{"isHardwareAccelerated",isHardwareAccelerated},
		{"isOpaque",isOpaque},
		{"quickReject",quickReject},
		{"restore",restore},
		{"restoreToCount",restoreToCount},
		{"rotate",rotate},
		{"save",save},
		{"saveLayer",saveLayer},
		{"saveLayerAlpha",saveLayerAlpha},
		{"scale",scale},
		{"setBitmap",setBitmap},
		{"setDensity",setDensity},
		{"setDrawFilter",setDrawFilter},
		{"setMatrix",setMatrix},
		{"skew",skew},
		{"translate",translate},
		
		{"lockCanvas", lockCanvas},
		{"unlockCanvasAndPost", unlockCanvasAndPost},
		{NULL, NULL}
	};
	luaL_newlib(L, funcs);
	return 1;
}


