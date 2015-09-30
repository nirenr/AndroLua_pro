require "import"
import "android.widget.*"
activity.setTheme(android.R.style.Theme_Holo_Light)
activity.setTitle("LogCat")
items={"A","L","T","E","W","I","D","V"}
function onCreateOptionsMenu(menu)
    for k,v in ipairs(items) do
        m=menu.add(v)
        items[v]=m
        --m.setShowAsActionFlags(1)
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
    p=io.popen("logcat  -d -v long "..s)
    t={}
    for s in p:lines() do
        table.insert(t,s)
        end
    p:close()
    return table.concat(t,"\n")
    end


func={}
func.A=function()
    task(readlog,"",show)
    end
func.L=function()
    task(readlog,"lua:* *:S",show)
    end
func.T=function()
    task(readlog,"tcc:* *:S",show)
    end

func.E=function()
    task(readlog,"*:E",show)
    end

func.W=function()
    task(readlog,"*:W",show)
    end
func.I=function()
    task(readlog,"*:I",show)
    end
func.D=function()
    task(readlog,"*:D",show)
    end
func.V=function()
    task(readlog,"*:V",show)
    end

scroll=ScrollView(activity)
logview=TextView(activity)
logview.TextIsSelectable=true
scroll.addView(logview)
function show(s)
    logview.setText(s)
    end
--print(1)

task([[
p=io.popen("logcat  -d -v long")
t={}
for s in p:lines() do
    table.insert(t,s)
    end
p:close()
return table.concat(t,"\n")
]],show)
activity.setContentView(scroll)
