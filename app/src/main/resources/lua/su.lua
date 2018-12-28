local Runtime=luajava.bindClass("java.lang.Runtime")
local DataOutputStream=luajava.bindClass("java.io.DataOutputStream")
local DataInputStream=luajava.bindClass("java.io.DataInputStream")
function su(...)
    local args={...}
    local process = Runtime.getRuntime().exec("su")
    local dos = DataOutputStream(process.getOutputStream());
    local dis = DataInputStream(process.getInputStream());
    for k,v in ipairs(args) do
        dos.writeBytes(v.."\n");
        dos.flush();
        end
    dos.writeBytes("exit\n");
    dos.flush();
    process.waitFor();
    local r=""
    local t={}
    while r do
        r=dis.readLine()
        table.insert(t,r)
        end
    return table.concat(t,"\n")
    end
