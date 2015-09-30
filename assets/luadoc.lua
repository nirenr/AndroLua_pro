require "import"
import "android.widget.*"
import "android.view.*"
import "com.androlua.*"
import "android.app.*"

activity.setTheme(android.R.style.Theme_Holo_Light)
activity.setTitle("Lua参考手册")

items={"目录","返回",}
function onCreateOptionsMenu(menu)
    for k,v in ipairs(items) do
        m=menu.add(v)
        m.setShowAsActionFlags(1)
        end
    end

function onMenuItemSelected(id,item)
    func[item.getTitle()]()
    end
func={}
func["目录"]=function()
    doc_web.loadUrl("file:/android_asset/luadoc/contents.html#contents")
    end

func["返回"]=function()
    --luajava.clear(doc_web)
    activity.finish()
    end

doc_web=LuaWebView(activity)
doc_web.loadUrl("file:/android_asset/luadoc/manual.html")
doc_web.setOnKeyListener(View.OnKeyListener{
    onKey=function (view,keyCode,event)
        if ((keyCode == event.KEYCODE_BACK) and view.canGoBack()) then
            view.goBack();
            return true;
            end
        return false
        end
    })


activity.setContentView(doc_web)





