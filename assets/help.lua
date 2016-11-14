require "import"
import "android.widget.*"
import "android.view.*"
import "android.app.*"
import "android.net.*"
import "android.content.*"
import "autotheme"

help=[===[
@关于@
@AndroLua是基于LuaJava开发的安卓平台轻量级脚本编程语言工具，既具有Lua简洁优雅的特质，又支持绝大部分安卓API，可以使你在手机上快速编写小型应用。
官方QQ群：236938279(已满)
http://jq.qq.com/?_wv=1027&k=dcofRr
官方QQ2群：148389676
http://jq.qq.com/?_wv=1027&k=2Gqxcak

百度贴吧：
http://c.tieba.baidu.com/mo/m?kw=androlua
项目地址：
https://github.com/nirenr/AndroLua_pro
点击链接支持我的工作，一块也可以哦：
https://qr.alipay.com/apt7ujjb4jngmu3z9a

本程序使用了以下开源项目部分代码

bson,crypt,md5
https://github.com/cloudwu/skynet

cjson
https://sourceforge.net/projects/cjson/

zlib
https://github.com/brimworks/lua-zlib

xml
https://github.com/chukong/quick-cocos2d-x

luv
https://github.com/luvit/luv
https://github.com/clibs/uv

zip
https://github.com/brimworks/lua-zip
https://github.com/julienr/libzip-android

luagl
http://luagl.sourceforge.net/

luasocket
https://github.com/diegonehab/luasocket

sensor
https://github.com/ddlee/AndroidLuaActivity

canvas
由落叶似秋开发

jni
由nirenr开发

@
@与标准Lua5.3的不同@
@打开了部分兼容选项，module，unpack，bit32
添加string.gfind函数，用于递归返回匹配位置
增加tointeger函数，强制将数值转为整数
修改tonumber支持转换Java对象
@
@1，参考链接@
@关于lua的语法和Android API请参考以下网页。
Lua官网：
http://www.lua.org
Android 中文API：
http://android.toolib.net/reference/packages.html
@
@2，导入模块@
@require "import" 
以导入import模块，简化写代码的难度。
目前程序还内置bmob,bson,canvas,cjson,crypt,ftp,gl,http,import,md5,smtp,socket,sensor,xml,zip,zlib等模块。
一般模块导入形式
local http=require "http"
这样导入的是局部变量
导入import后也可以使用
import "http"
的形式，导入为全局变量
@
@3，导入包或类@
@在使用Java类之前需要导入相应的包或者类，
可以用包名.*的形式导入导入包
import "android.widget.*"
或者用完整的类名导入类
import "android.widget.Button"
导入内部类
import "android.view.View_OnClickListener"
或者在导入类后直接使用内部类
View.OnClickListene
包名和类名必须用引号包围。
导入的类为全局变量，你可以使用
local Burton=import "android.widget.Button"
的形式保存为局部变量，以解决类名冲突问题。
@
@4，创建布局与组件@
@安卓使用布局与视图管理和显示用户界面。
布局负责管理视图如何显示，如LinearLayout以线性排列视图，FrameLayout则要求自行指定停靠与位置。
视图则显示具体内容，如TextView可以向用户展示文字内容，Button可以响应用户点击事件。

创建一个线性布局
layout=LinearLayout(activity)
创建一个按钮视图
button=Button(activity)
将按钮添加到布局
layout.addView(button)
将刚才的内容设置为活动内容视图
activity.setContentView(layout)

注.activity是当前窗口的Context对象，如果你习惯也可以使用this
button=Button(this)
@
@5，使用方法@
@button.setText("按钮")

getter/setter
Java的getxxx方法没有参数与setxxx方法只有一个参数时可以简写，
button.Text="按钮"
x=button.Text
@
@6，使用事件@
@创建事件处理函数
function click(s)
    print("点击")
    end
把函数添加到事件接口
listener=View.OnClickListener{onClick = click}
把接口注册到组件
button.setOnClickListener(listener)

也可以使用匿名函数
button.setOnClickListener(View.OnClickListener {onClick = function(s)
        print("点击")
        end
    })
    
onxxx事件可以简写
button.onClick=function(v)
    print(v)
    end
@
@7，回调方法@
@在活动文件添加以下函数，这些函数可以在活动的特定状态执行。
function main(...)
    --...：newActivity传递过来的参数。
    print("入口函数",...)
    end

function onCreate()
    print("窗口创建")
    end

function onStart()
    print("活动开始")
    end
  
function onResume()
    print("返回程序")
    end

function onPause()
    print("活动暂停")
    end

function onStop()
    print("活动停止")
    end

function onDestroy()
    print("程序已退出")
    end

function onResult(name,...)
  --name：返回的活动名称
  --...：返回的参数
  print("返回活动",name,...)
  end

function onCreateOptionsMenu(menu)
    --menu：选项菜单。
    menu.add("菜单")
    end

function onOptionsItemSelected(item)
    --item：选中的菜单项
    print(item.Title)
    end

function onConfigurationChanged(config)
    --config：配置信息
    print("屏幕方向关闭")
    end
  
function onKeyDown(keycode,event)
    --keycode：键值
    --event：事件
    print("按键按下",keycode)
    end

function onKeyUp(keycode,event)
    --keycode：键值
    --event：事件
    print("按键抬起",keycode)
    end

function onKeyLongPress(keycode,event)
    --keycode：键值
    --event：事件
    print("按键长按",keycode)
    end

function onTouchEvent(event)
    --event：事件
    print("触摸事件",event)
    end
  @
@8，按键与触控@
@function onKeyDown(code,event)
    print(code event)
    end
function onTouchEvent(event)
    print(event)
    end
支持onKeyDown,onKeyUp,onKeyLongPress,onTouchEvent
函数必须返布尔值
@
@9，使用数组@
@array=float{1,2,3}
或者
array=int[10]
a=array[0]
array[0]=4
@
@10，使用线程@
@需导入import模块，参看thread,timer与task函数说明。
线程中使用独立环境运行，不能使用外部变量与函数，需要使用参数和回调与外部交互。
任务

task(str,args,callback)

str 为任务执行代码，args 为参数，callback 为回调函数，任务返回值将传递到回调方法
线程

t=thread(str,args)

str 为线程中执行的代码，args 为初始传入参数
调用线程中方法
call(t,fn,args)
t 为线程，fn 为方法名称，args 为参数
设置线程变量
set(t,fn,arg)
t 为线程，fn 为变量名称，arg 为变量值
线程调用主线程中方法
call(fn,args)
fn 为方法名称，args 为参数
线程设置主线程变量
set(fn,arg)
fn 为变量名称，arg 为变量值

注. 参数类型为 字符串，数值，Java对象，布尔值与nil
线程要使用quit结束线程。

t=timer(func,delay,period,args)

func 为定时器执行的函数，delay 为定时器延时，period 为定时器间隔，args 为初始化参数
t.Enable=false 暂停定时器
t.Enable=true 启动定时器
t.stop() 停止定时器

注意：定时器函数定义run函数时定时器重复执行run函数，否则重复执行构建时的func函数
@
@11，使用布局表@
@使用布局表须导入android.view与android.widget包。
require "import"
import "android.widget.*"
import "android.view.*"
布局表格式
layout={
    控件类名称,
    id=控件名称,
    属性=值,
    {
        子控件类名称,
        id=控件名称,
        属性=值,
        }
    }
  
例如：
layout={
  LinearLayout,--视图类名称
  id="linear",--视图ID，可以在loadlayout后直接使用
  orientation="vertical",--属性与值
  {
    TextView,--子视图类名称
    text="hello AndroLua+",--属性与值
    layout_width="fill"--布局属性
  },
}
使用loadlayout函数解析布局表生成布局。
activity.setContentView(loadlayout(layout))
也可以简化为：
activity.setContentView(layout)
如果使用单独文件布局(比如有个layout.aly布局文件)也可以简写为：
activity.setContentView("layout")
此时不用导入布局文件。

布局表支持大全部安卓控件属性，
与安卓XML布局文件的不同点：
id表示在Lua中变量的名称，而不是安卓的可以findbyid的数字id。
ImageView的src属性是当前目录图片名称或绝对文件路径图片或网络上的图片，
layout_width与layout_height的值支持fill与wrap简写，
onClick值为lua函数或java onClick接口或他们的全局变量名称，
背景background支持背景图片，背景色与LuaDrawable自绘制背景，背景图片参数为是当前目录图片名称或绝对文件路径图片或网络上的图片，颜色同backgroundColor，自绘制背景参数为绘制函数或绘制函数的全局变量名称，
控件背景色使用backgroundColor设置，值为"十六进制颜色值"。
尺寸单位支持 px，dp，sp，in，mm，%w，%h。
其他参考loadlayout与loadbitmap
@
@12，2D绘图@
@require "import"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "android.graphics.*"
activity.setTitle('AndroLua')

paint=Paint()
paint.setARGB(100,0,250,0)
paint.setStrokeWidth(20)
paint.setTextSize(28)

sureface = SurfaceView(activity);
callback=SurfaceHolder_Callback{
    surfaceChanged=function(holder,format,width,height)
        end,
    surfaceCreated=function(holder)
        ca=holder.lockCanvas()
        if (ca~=nil) then
            ca.drawRGB(0,79,90);
            ca.drawRect(0,0,200,300,paint)
            end
        holder.unlockCanvasAndPost(ca)
        end,
    surfaceDestroyed=function(holder)
        end
    }
holder=sureface.getHolder()
holder.addCallback(callback)
activity.setContentView(sureface)
@
@13，Lua类型与Java类型@
@在大多数情况下androlua可以很好的处理Lua与Java类型之间的自动转换，但是Java的数值类型有多种(double,float,long,int,short,byte)，而Lua只有number，在必要的情况下可以使用类型的强制转换。
i=int(10)
i就是一个Java的int类型数据
d=double(10)
d是一个Java的double类型
在调用Java方法时androlua可以自动将Lua的table转换成Java的array，Map或interface
Map类型可以像使用Lua表一样简便。
map=HashMap{a=1,b=2}
print(map.a)
map.a=3
取长度运算符#可以获取Java中array，List,Map,Set，String的长度。

@

@14.1 canvas模块@
@require "import"
import "canvas"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "android.graphics.*"
activity.setTitle('AndroLua')

paint=Paint()
paint.setARGB(100,0,250,0)
paint.setStrokeWidth(20)
paint.setTextSize(28)

sureface = SurfaceView(activity);
callback=SurfaceHolder_Callback{
    surfaceChanged=function(holder,format,width,height)
        end,
    surfaceCreated=function(holder)
        ca=canvas.lockCanvas(holder)
        if (ca~=nil) then
            ca:drawRGB(0,79,90)
            ca:drawRect(0,0,200,300,paint)
            end
        canvas.unlockCanvasAndPost(holder,ca)
        end,
    surfaceDestroyed=function(holder)
        end
    }
holder=sureface.getHolder()
holder.addCallback(callback)
activity.setContentView(sureface)
@
@14.2 OpenGL模块@
@require "import"
import "gl"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "android.opengl.*"
activity.setTitle('AndroLua')
--activity.setTheme( android.R.style.Theme_Holo_Light_NoActionBar_Fullscreen)

mTriangleData ={
    0.0, 0.6, 0.0,
    -0.6, 0.0, 0.0,
    0.6, 0.0, 0.0,
    };
mTriangleColor = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    };

sr=GLSurfaceView.Renderer{
    onSurfaceCreated=function(gl2, config)
        gl.glDisable(gl.GL_DITHER);
        gl.glHint(gl.GL_PERSPECTIVE_CORRECTION_HINT, gl.GL_FASTEST);
        gl.glClearColor(0, 0, 0, 0);
        gl.glShadeModel(gl.GL_SMOOTH);
        gl.glClearDepth(1.0)
        gl.glEnable(gl.GL_DEPTH_TEST);
        gl.glDepthFunc(gl.GL_LEQUAL);
        end,
    onDrawFrame=function(gl2, config)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT);
        gl.glMatrixMode(gl.GL_MODELVIEW);
        gl.glLoadIdentity();
        gl.glRotate(0,1,1,1)
        gl.glTranslate(0, 0,0);
        gl.glEnableClientState(gl.GL_VERTEX_ARRAY);
        gl.glEnableClientState(gl.GL_COLOR_ARRAY);
        gl.glVertexPointer( mTriangleData,3);
        gl.glColorPointer(mTriangleColor,4);
        gl.glDrawArrays( gl.GL_TRIANGLE_STRIP , 0, 3);
        gl.glFinish();
        gl.glDisableClientState(gl.GL_VERTEX_ARRAY);
        gl.glDisableClientState(gl.GL_COLOR_ARRAY);
        end,
    onSurfaceChanged= function (gl2, w, h)
        gl.glViewport(0, 0, w, h);
        gl.glLoadIdentity();
        ratio =  w / h;
        gl.glFrustum(-rautio, ratio, -1, 1, 1, 10);
        end
    }

glSurefaceView = GLSurfaceView(activity);
glSurefaceView.setRenderer(sr);
activity.setContentView(glSurefaceView);

@
@14.3 http 同步网络模块@
@body,cookie,code,headers=http.get(url [,cookie,ua,header])
body,cookie,code,headers=http.post(url ,postdata [,cookie,ua,header])
code,headers=http.download(url [,cookie,ua,ref,header])
body,cookie,code,headers=http.upload(url ,datas ,files [,cookie,ua,header])
参数说明
url 网址
postdata post的字符串或字符串数据组表
datas upload的字符串数据组表
files upload的文件名数据表
cookie 网页要求的cookie
ua 浏览器识别
ref 来源页网址
header http请求头

require "import"
import "http"

--get函数以get请求获取网页，参数为请求的网址与cookie
body,cookie,code,headers=http.get("http://www.androlua.com")

--post函数以post请求获取网页，通常用于提交表单，参数为请求的网址，要发送的内容与cookie
body,cookie,code,headers=http.post("http://androlua.com/Login.Asp?Login=Login&Url=http://androlua.com/bbs/index.asp","name=用户名&pass=密码&ki=1")

--download函数和get函数类似，用于下载文件，参数为请求的网址，保存文件的路径与cookie
http.download("http://androlua.com","/sdcard/a.txt")

--upload用于上传文件，参数是请求的网址，请求内容字符串部分，格式为以key=value形式的表，请求文件部分，格式为key=文件路径的表，最后一个参数为cookie
http.upload("http://androlua.com",{title="标题",msg="内容"},{file1="/sdcard/1.txt",file2="/sdcard/2.txt"})
@
@14.4 import模块@
@require "import"
import "android.widget.*"
import "android.view.*"
layout={
    LinearLayout,
    orientation="vertical",
    {
        EditText,
        id="edit",
        layout_width="fill"
        },
    {
        Button,
        text="按钮",
        layout_width="fill",
        onClick="click"
        }
    }

function click()
    Toast.makeText(activity, edit.getText().toString(), Toast.LENGTH_SHORT ).show()
    end
activity.setContentView(loadlayout(layout))
@
@14.5 Http 异步网络模块@
@获取内容 get函数
Http.get(url,cookie,charset,header,callback)
url 网络请求的链接网址
cookie 使用的cookie，也就是服务器的身份识别信息
charset 内容编码
header 请求头
callback 请求完成后执行的函数

除了url和callback其他参数都不是必须的

回调函数接受四个参数值分别是
code 响应代码，2xx表示成功，4xx表示请求错误，5xx表示服务器错误，-1表示出错
content 内容，如果code是-1，则为出错信息
cookie 服务器返回的用户身份识别信息
header 服务器返回的头信息

向服务器发送数据 post函数
Http.post(url,data,cookie,charset,header,callback)
除了增加了一个data外，其他参数和get完全相同
data 向服务器发送的数据

下载文件 download函数
Http.download(url,path,cookie,header,callback)
参数中没有编码参数，其他同get，
path 文件保存路径

需要特别注意一点，只支持同时有127个网络请求，否则会出错


Http其实是对Http.HttpTask的封装，Http.HttpTask使用的更加通用和灵活的形式
参数格式如下
Http.HttpTask( url, String method, cookie, charset, header,  callback) 
所有参数都是必选，没有则传入nil

url 请求的网址
method 请求方法可以是get，post，put，delete等
cookie 身份验证信息
charset 内容编码
header 请求头
callback 回调函数

该函数返回的是一个HttpTask对象，
需要调用execute方法才可以执行，
t=Http.HttpTask(xxx)
t.execute{data}

注意调用的括号是花括号，内容可以是字符串或者byte数组，
使用这个形式可以自己封装异步上传函数

@
@14.6 bmob网络数据库@
@b=bmob(id,key)
id 用户id，key 应用key。

b:insert(key,data,callback)
新建数据表，key 表名称，data 数据，callback 回调函数。

b:update(key,id,data,callback)
更新数据表，key 表名称id 数据id，data 数据，callback 回调函数。

b:query(key,data,callback)
查询数据表，key 表名称，data 查询规则，callback 回调函数。

b:increment(key,id,k,v,c)
原子计数，key 表名称，id 数据id，k 数据key，v 计数增加量。

b:delete(key,id,callback)
删除数据，key 表名称,id 数据id，callback 回调函数。

b:sign(user,pass,mail,callback)
注册用户，user 用户名，pass 密码，mail 电子邮箱，callback 回调函数。

b:login(user or mail,pass,callback)
登录用户，user 用户名，pass 密码，mail 电子邮箱，callback 回调函数。

b:upload(path,callback)
上传文件，path 文件路径，callback 回调函数。

b:remove(url,callback)
删除文件，url 文件路径，callback 回调函数。


注：
1，查询规则支持表或者json格式，具体用法参考官方api
2，回调函数的第一个参数为状态码，-1 出错，其他状态码参考http状态码，第二个参数为返回内容。
@
@15.1 LuaUtil 辅助库@
@copyDir(from,to)
复制文件或文件夹，from 源路径，to 目标路径。

zip(from,dir,name)
压缩文件或文件夹，from 源路径，dir 目标文件夹，name zip文件名称。

unZip(from,to)
解压文件，from zip文件路径，to 目标路径。

getFileMD5(path)
获取文件MD5值， path 文件路径。

getFileSha1(path)
获取文件Sha1值， path 文件路径。

@
@关于打包@
@新建工程或在脚本目录新建init.lua文件。
写入以下内容，即可将文件夹下所有lua文件打包，main.lua为程序人口。
appname="demo"
appver="1.0"
packagename="com.androlua.demo"
目录下icon.png替换图标，welcome.png替换启动图。
打包使用debug签名。
@
@部分函数参考@
@
[a]表示参数a可选，(...)表示不定参数。函数调用在只有一个参数且参数为字符串或表时可以省略括号。
AndroLua库函数在import模块，为便于使用都是全局变量。
s 表示string类型，i 表示整数类型，n 表示浮点数或整数类型，t 表示表类型，b 表示布尔类型，o 表示Java对象类型，f为Lua函数。
--表示注释。

each(o)
参数：o 实现Iterable接口的Java对象
返回：用于Lua迭代的闭包
作用：Java集合迭代器


enum(o)
参数：o 实现Enumeration接口的Java对象
返回：用于Lua迭代的闭包
作用：Java集合迭代器

import(s)
参数：s 要载入的包或类的名称
返回：载入的类或模块
作用：载入包或类或Lua模块
import "http" --载入http模块
import "android.widget.*" --载入android.widget包
import "android.widget.Button" --载入android.widget.Button类
import "android.view.View$OnClickListener" --载入android.view.View.OnClickListener内部类

loadlayout(t [,t2])
参数：t 要载入的布局表，t2 保存view的表
返回：布局最外层view
作用：载入布局表，生成view
layout={
    LinearLayout,
    layout_width="fill",
    {
        TextView,
        text="Androlua",
        id="tv"
        }
    }
main={}
activity.setContentView(loadlayout(layout,main))
print(main.tv.getText())

loadbitmap(s)
参数：s 要载入图片的地址，支持相对地址，绝对地址与网址
返回：bitmap对象
作用：载入图片
注意：载入网络图片需要在线程中进行

task(s [,...], f)
参数：s 任务中运行的代码或函数，... 任务传入参数，f 回调函数
返回：无返回值
作用：在异步线程运行Lua代码，执行完毕在主线程调用回调函数
注意：参数类型包括 布尔，数值，字符串，Java对象，不允许Lua对象
function func(a,b)
    require "import"
    print(a,b)
    return a+b
    end
task(func,1,2,print)

thread(s[,...])
参数：s 线程中运行的lua代码或脚本的相对路径(不加扩展名)或函数，... 线程初始化参数
返回：返回线程对象
作用：开启一个线程运行Lua代码
注意：线程需要调用quit方法结束线程
func=[[
a,b=...
function add()
    call("print",a+b)
    end
]]
t=thread(func,1,2)
t.add()

timer(s,i1,i2[,...])
参数：s 定时器运行的代码或函数，i1 前延时，i2 定时器间隔，... 定时器初始化参数
返回：定时器对象
作用：创建定时器重复执行函数
function f(a)
    function run()
        print(a)
        a=a+1
        end
    end

t=timer(f,0,1000,1)
t.Enabled=false--暂停定时器
t.Enabled=true--重新定时器
t.stop()--停止定时器

luajava.bindClass(s)
参数：s class的完整名称，支持基本类型
返回：Java class对象
作用：载入Java class
Button=luajava.bindClass("android.widget.Button")
int=luajava.bindClass("int")

luajava.createProxy(s,t)
参数：s 接口的完整名称，t 接口函数表
返回：Java接口对象
作用：创建Java接口
onclick=luajava.createProxy("android.view.View$OnClickListener",{onClick=function(v)print(v)end})

luajava.createArray(s,t)
参数：s 类的完整名称，支持基本类型，t 要转化为Java数组的表
返回：创建的Java数组对象
作用：创建Java数组
arr=luajava.createArray("int",{1,2,3,4})

luajava.newInstance(s [,...])
参数：s 类的完整名称，... 构建方法的参数
作用：创建Java类的实例
b=luajava.newInstance("android.widget.Button",activity)

luajava.new(o[,...])
参数：o Java类对象，... 参数
返回：类的实例或数组对象或接口对象
作用：创建一个类实例或数组对象或接口对象
注意：当只有一个参数且为表类型时，如果类对象为interface创建接口，为class创建数组，参数为其他情况创建实例
b=luajava.new(Button,activity)
onclick=luajava.new(OnClickListener,{onClick=function(v)print(v)end})
arr=luajava.new(int,{1,2,3})
(示例中假设已载入相关类)

luajava.coding(s [,s2 [, s3]])
参数：s 要转换编码的Lua字符串，s2 字符串的原始编码，s3 字符串的目标编码
返回：转码后的Lua字符串
作用：转换字符串编码
注意：默认进行GBK转UTF8

luajava.clear(o)
参数：o Java对象
返回：无
作用：销毁Java对象
注意：仅用于销毁临时对象

luajava.astable(o)
参数：o Java对象
返回：Lua表
作用：转换Java的Array List或Map为Lua表

luajava.tostring(o)
参数：o Java对象
返回：Lua字符串
作用：相当于 o.toString()
@
@activity部分API参考@
@setContentView(layout, env)
设置布局表layout为当前activity的主视图，env是保存视图ID的表，默认是_G
getLuaDir()
返回脚本当前目录
getLuaDir(name)
返回脚本当前目录的子目录
getLuaExtDir()
返回Androlua在SD的工作目录
getLuaExtDir(name)
返回Androlua在SD的工作目录的子目录
getWidth()
返回屏幕宽度
getHeight()
返回屏幕高度，不包括状态栏与导航栏
loadDex(path)
加载当前目录dex或jar，返回DexClassLoader
loadLib(path)
加载当前目录c模块，返回载入后模块的返回值(通常是包含模块函数的包)
registerReceiver(filter)
注册一个广播接收者，当再次调用该方法时将移除上次注册的过滤器
newActivity(req, path, enterAnim, exitAnim, arg)
打开一个新activity，运行路径为path的Lua文件，其他参数为可选，arg为表，接受脚本为变长参数
result{...}
向来源activity返回数据，在源activity的onResult回调
newTask(func[, update], callback)
新建一个Task异步任务，在线程中执行func函数，其他两个参数可选，执行结束回调callback，在任务调用update函数时在UI线程回调该函数
新建的Task在调用execute{}时通过表传入参数，在func以unpack形式接收，执行func可以返回多个值
newThread(func, arg)
新建一个线程，在线程中运行func函数，可以以表的形式传入arg，在func以unpack形式接收
新建的线程调用start()方法运行，线程为含有loop线程，在当前activity结束后自动结束loop
newTimer(func, arg)
新建一个定时器，在线程中运行func函数，可以以表的形式传入arg，在func以unpack形式接收
调用定时器的start(delay, period)开始定时器，stop()停止定时器，Enabled暂停恢复定时器，Period属性改变定时器间隔
@
@布局表字符串常量@
@布局表支持属性字符串常量
    -- android:drawingCacheQuality
    auto=0,
    low=1,
    high=2,

    -- android:importantForAccessibility
    auto=0,
    yes=1,
    no=2,

    -- android:layerType
    none=0,
    software=1,
    hardware=2,

    -- android:layoutDirection
    ltr=0,
    rtl=1,
    inherit=2,
    locale=3,

    -- android:scrollbarStyle
    insideOverlay=0x0,
    insideInset=0x01000000,
    outsideOverlay=0x02000000,
    outsideInset=0x03000000,

    -- android:visibility
    visible=0,
    invisible=1,
    gone=2,

    wrap_content=-2,
    fill_parent=-1,
    match_parent=-1,
    wrap=-2,
    fill=-1,
    match=-1,

    -- android:orientation
    vertical=1,
    horizontal= 0,

    -- android:gravity
    axis_clip = 8,
    axis_pull_after = 4,
    axis_pull_before = 2,
    axis_specified = 1,
    axis_x_shift = 0,
    axis_y_shift = 4,
    bottom = 80,
    center = 17,
    center_horizontal = 1,
    center_vertical = 16,
    clip_horizontal = 8,
    clip_vertical = 128,
    display_clip_horizontal = 16777216,
    display_clip_vertical = 268435456,
    --fill = 119,
    fill_horizontal = 7,
    fill_vertical = 112,
    horizontal_gravity_mask = 7,
    left = 3,
    no_gravity = 0,
    relative_horizontal_gravity_mask = 8388615,
    relative_layout_direction = 8388608,
    right = 5,
    start = 8388611,
    top = 48,
    vertical_gravity_mask = 112,
    end = 8388613,

    -- android:textAlignment
    inherit=0,
    gravity=1,
    textStart=2,
    textEnd=3,
    textCenter=4,
    viewStart=5,
    viewEnd=6,
    
    -- android:inputType
    none=0x00000000,
    text=0x00000001,
    textCapCharacters=0x00001001,
    textCapWords=0x00002001,
    textCapSentences=0x00004001,
    textAutoCorrect=0x00008001,
    textAutoComplete=0x00010001,
    textMultiLine=0x00020001,
    textImeMultiLine=0x00040001,
    textNoSuggestions=0x00080001,
    textUri=0x00000011,
    textEmailAddress=0x00000021,
    textEmailSubject=0x00000031,
    textShortMessage=0x00000041,
    textLongMessage=0x00000051,
    textPersonName=0x00000061,
    textPostalAddress=0x00000071,
    textPassword=0x00000081,
    textVisiblePassword=0x00000091,
    textWebEditText=0x000000a1,
    textFilter=0x000000b1,
    textPhonetic=0x000000c1,
    textWebEmailAddress=0x000000d1,
    textWebPassword=0x000000e1,
    number=0x00000002,
    numberSigned=0x00001002,
    numberDecimal=0x00002002,
    numberPassword=0x00000012,
    phone=0x00000003,
    datetime=0x00000004,
    date=0x00000014,
    time=0x00000024,
    
    --android:ellipsize
    end　　  
    start 　　
    middle     
    marquee

相对布局rule
    layout_above=2,
    layout_alignBaseline=4,
    layout_alignBottom=8,
    layout_alignEnd=19,
    layout_alignLeft=5,
    layout_alignParentBottom=12,
    layout_alignParentEnd=21,
    layout_alignParentLeft=9,
    layout_alignParentRight=11,
    layout_alignParentStart=20,
    layout_alignParentTop=10,
    layout_alignRight=7,
    layout_alignStart=18,
    layout_alignTop=6,
    layout_alignWithParentIfMissing=0,
    layout_below=3,
    layout_centerHorizontal=14,
    layout_centerInParent=13,
    layout_centerVertical=15,
    layout_toEndOf=17,
    layout_toLeftOf=0,
    layout_toRightOf=1,
    layout_toStartOf=16
    


尺寸单位
    px=0,
    dp=1,
    sp=2,
    pt=3,
    in=4,
    mm=5

@
]===]
activity.setTitle("帮助")
activity.setTheme(autotheme())


list={}
for t,c in help:gmatch("(%b@@)\n*(%b@@)") do
    --print(t)
    t=t:sub(2,-2)
    c=c:sub(2,-2)
    list[t]=c
    list[#list+1]=t
    end

function show(v)
    local s=v.getText()
    local c=list[s]
    if c then
        help_dlg.setTitle(s)
        help_tv.setText(c)
        help_dlg.show()
        --  local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String({c}))
        -- listview.setAdapter(adapter)
        end
    end



listview=ListView(activity)
listview.setOnItemClickListener(AdapterView.OnItemClickListener{
    onItemClick=function(parent, v, pos,id)
        show(v)
        end
    })
local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(list))
listview.setAdapter(adapter)
activity.setContentView(listview)

help_dlg=Dialog(activity,autotheme())
help_sv=ScrollView(activity)
help_tv=TextView(activity)
help_tv.setTextSize(20)
help_tv.TextIsSelectable=true
help_sv.addView(help_tv)
help_dlg.setContentView(help_sv)

func={}
func["捐赠"]=function()
    intent = Intent();
    intent.setAction("android.intent.action.VIEW");
    content_url = Uri.parse("https://qr.alipay.com/apt7ujjb4jngmu3z9a");
    intent.setData(content_url);
    activity.startActivity(intent);
    end
func["返回"]=function()
    activity.finish()
    end

items={"捐赠","返回"}
function onCreateOptionsMenu(menu)
    for k,v in ipairs(items) do
        m=menu.add(v)
        m.setShowAsActionFlags(1)
        end
    end

function onMenuItemSelected(id,item)
    func[item.getTitle()]()
    end




