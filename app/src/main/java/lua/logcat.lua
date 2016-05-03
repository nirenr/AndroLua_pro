require "import"
import "android.widget.*"
activity.setTheme(android.R.style.Theme_Holo_Light)
activity.setTitle("LogCat")
items={"All","Lua","Tcc","Error","Warning","Info","Debug","Verbose","Clear"}
function onCreateOptionsMenu(menu)
  for k,v in ipairs(items) do
    m=menu.add(v)
    items[v]=m
  end
end

function onMenuItemSelected(id,item)
  if func[item.getTitle()] then
    func[item.getTitle()]()
  else
    print(item,"功能开发中。。。")
  end
end

function readlog(s)
  p=io.popen("logcat -d -v long "..s)
  local s=p:read("*a")
  p:close()
  s=s:gsub("%-+ beginning of[^\n]*\n","")
  if #s==0 then
    s="<run the app to see its log output>"
    end
  return s
end

function clearlog()
  p=io.popen("logcat -c")
  local s=p:read("*a")
  p:close()
  return s
end

func={}
func.All=function()
  activity.setTitle("LogCat - All")
  task(readlog,"",show)
end
func.Lua=function()
  activity.setTitle("LogCat - Lua")
  task(readlog,"lua:* *:S",show)
end
func.Tcc=function()
  activity.setTitle("LogCat - Tcc")
  task(readlog,"tcc:* *:S",show)
end
func.Error=function()
  activity.setTitle("LogCat - Error")
  task(readlog,"*:E",show)
end
func.Warning=function()
  activity.setTitle("LogCat - Warning")
  task(readlog,"*:W",show)
end
func.Info=function()
  activity.setTitle("LogCat - Info")
  task(readlog,"*:I",show)
end
func.Debug=function()
  activity.setTitle("LogCat - Debug")
  task(readlog,"*:D",show)
end
func.Verbose=function()
  activity.setTitle("LogCat - Verbose")
  task(readlog,"*:V",show)
end
func.Clear=function()
  task(clearlog,show)
end

scroll=ScrollView(activity)
logview=TextView(activity)
logview.TextIsSelectable=true
scroll.addView(logview)
function show(s)
  logview.setText(s)
end

func.Lua()
activity.setContentView(scroll)