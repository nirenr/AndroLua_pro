require "import"
import "java.util.zip.ZipOutputStream"
import "android.net.Uri"
import "java.io.File"
import "android.widget.Toast"
import "java.util.zip.CheckedInputStream"
import "java.io.FileInputStream"
import "android.content.Intent"
import "java.security.Signer"
import "java.util.ArrayList"
import "java.io.FileOutputStream"
import "java.io.BufferedOutputStream"
import "java.util.zip.ZipInputStream"
import "java.io.BufferedInputStream"
import "java.util.zip.ZipEntry"
import "android.app.ProgressDialog"
import "java.util.zip.CheckedOutputStream"
import "java.util.zip.Adler32"

local bin_dlg, error_dlg
local function update(s)
    bin_dlg.setMessage(s)
end

local function callback(s)
    LuaUtil.rmDir(File(activity.getLuaExtDir("bin/.temp")))
    bin_dlg.hide()
    bin_dlg.Message = ""
    if not s:find("成功") then
        error_dlg.Message = s
        error_dlg.show()
    end
end

local function create_bin_dlg()
    if bin_dlg then
        return
    end
    bin_dlg = ProgressDialog(activity);
    bin_dlg.setTitle("正在打包");
    bin_dlg.setMax(100);
end

local function create_error_dlg2()
    if error_dlg then
        return
    end
    error_dlg = AlertDialogBuilder(activity)
    error_dlg.Title = "出错"
    error_dlg.setPositiveButton("确定", nil)
end

local function binapk(luapath, apkpath)
    require "import"
    import "console"
    compile "mao"
    compile "sign"
    import "java.util.zip.*"
    import "java.io.*"
    import "mao.res.*"
    import "apksigner.*"
    local b = byte[2 ^ 16]
    local function copy(input, output)
        LuaUtil.copyFile(input, output)
        input.close()
        --[[local l=input.read(b)
      while l>1 do
        output.write(b,0,l)
        l=input.read(b)
      end]]
    end

    local function copy2(input, output)
        LuaUtil.copyFile(input, output)
    end

    local temp = File(apkpath).getParentFile();
    if (not temp.exists()) then

        if (not temp.mkdirs()) then

            error("create file " .. temp.getName() .. " fail");
        end
    end


    local tmp = luajava.luadir .. "/tmp.apk"
    local info = activity.getApplicationInfo()
    local ver = activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0).versionName
    local code = activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0).versionCode

    --local zip=ZipFile(info.publicSourceDir)
    local zipFile = File(info.publicSourceDir)
    local fis = FileInputStream(zipFile);
    --local checksum = CheckedInputStream(fis, Adler32());
    local zis = ZipInputStream(BufferedInputStream(fis));

    local fot = FileOutputStream(tmp)
    --local checksum2 = CheckedOutputStream(fot, Adler32());

    local out = ZipOutputStream(BufferedOutputStream(fot))
    local f = File(luapath)
    local errbuffer = {}
    local replace = {}
    local checked = {}
    local lualib = {}
    local md5s = {}
    local libs = File(activity.ApplicationInfo.nativeLibraryDir).list()
    libs = luajava.astable(libs)
    for k, v in ipairs(libs) do
        --libs[k]="lib/armeabi/"..libs[k]
        replace[v] = true
    end

    local mdp = activity.Application.MdDir
    local function getmodule(dir)
        local mds = File(activity.Application.MdDir .. dir).listFiles()
        mds = luajava.astable(mds)
        for k, v in ipairs(mds) do
            if mds[k].isDirectory() then
                getmodule(dir .. mds[k].Name .. "/")
            else
                mds[k] = "lua" .. dir .. mds[k].Name
                replace[mds[k]] = true
            end
        end
    end

    getmodule("/")

    local function checklib(path)
        if checked[path] then
            return
        end
        local cp, lp
        checked[path] = true
        local f = io.open(path)
        local s = f:read("*a")
        f:close()
        for m, n in s:gmatch("require *%(? *\"([%w_]+)%.?([%w_]*)") do
            cp = string.format("lib%s.so", m)
            if n ~= "" then
                lp = string.format("lua/%s/%s.lua", m, n)
                m = m .. '/' .. n
            else
                lp = string.format("lua/%s.lua", m)
            end
            if replace[cp] then
                replace[cp] = false
            end
            if replace[lp] then
                checklib(mdp .. "/" .. m .. ".lua")
                replace[lp] = false
                lualib[lp] = mdp .. "/" .. m .. ".lua"
            end
        end
        for m, n in s:gmatch("import *%(? *\"([%w_]+)%.?([%w_]*)") do
            cp = string.format("lib%s.so", m)
            if n ~= "" then
                lp = string.format("lua/%s/%s.lua", m, n)
                m = m .. '/' .. n
            else
                lp = string.format("lua/%s.lua", m)
            end
            if replace[cp] then
                replace[cp] = false
            end
            if replace[lp] then
                checklib(mdp .. "/" .. m .. ".lua")
                replace[lp] = false
                lualib[lp] = mdp .. "/" .. m .. ".lua"
            end
        end
    end

    replace["libluajava.so"] = false

    local function addDir(out, dir, f)
        local entry = ZipEntry("assets/" .. dir)
        out.putNextEntry(entry)
        local ls = f.listFiles()
        for n = 0, #ls - 1 do
            local name = ls[n].getName()
            if name==(".using") then
                checklib(luapath .. dir .. name)
            elseif name:find("%.apk$") or name:find("%.luac$") or name:find("^%.") then
            elseif name:find("%.lua$") then
                checklib(luapath .. dir .. name)
                local path, err = console.build(luapath .. dir .. name)
                if path then
                    if replace["assets/" .. dir .. name] then
                        table.insert(errbuffer, dir .. name .. "/.aly")
                    end
                    local entry = ZipEntry("assets/" .. dir .. name)
                    out.putNextEntry(entry)

                    replace["assets/" .. dir .. name] = true
                    copy(FileInputStream(File(path)), out)
                    table.insert(md5s, LuaUtil.getFileMD5(path))
                    os.remove(path)
                else
                    table.insert(errbuffer, err)
                end
            elseif name:find("%.aly$") then
                local path, err = console.build_aly(luapath .. dir .. name)
                if path then
                    name = name:gsub("aly$", "lua")
                    if replace["assets/" .. dir .. name] then
                        table.insert(errbuffer, dir .. name .. "/.aly")
                    end
                    local entry = ZipEntry("assets/" .. dir .. name)
                    out.putNextEntry(entry)

                    replace["assets/" .. dir .. name] = true
                    copy(FileInputStream(File(path)), out)
                    table.insert(md5s, LuaUtil.getFileMD5(path))
                    os.remove(path)
                else
                    table.insert(errbuffer, err)
                end
            elseif ls[n].isDirectory() then
                addDir(out, dir .. name .. "/", ls[n])
            else
                local entry = ZipEntry("assets/" .. dir .. name)
                out.putNextEntry(entry)
                replace["assets/" .. dir .. name] = true
                copy(FileInputStream(ls[n]), out)
                table.insert(md5s, LuaUtil.getFileMD5(ls[n]))
            end
        end
    end


    this.update("正在编译...");
    if f.isDirectory() then
        require "permission"
        dofile(luapath .. "init.lua")
        if user_permission then
            for k, v in ipairs(user_permission) do
                user_permission[v] = true
            end
        end


        local ss, ee = pcall(addDir, out, "", f)
        if not ss then
            table.insert(errbuffer, ee)
        end
        --print(ee,dump(errbuffer),dump(replace))


        local wel = File(luapath .. "icon.png")
        if wel.exists() then
            local entry = ZipEntry("res/drawable/icon.png")
            out.putNextEntry(entry)
            replace["res/drawable/icon.png"] = true
            copy(FileInputStream(wel), out)
        end
        local wel = File(luapath .. "welcome.png")
        if wel.exists() then
            local entry = ZipEntry("res/drawable/welcome.png")
            out.putNextEntry(entry)
            replace["res/drawable/welcome.png"] = true
            copy(FileInputStream(wel), out)
        end
    else
        return "error"
    end

    --print(dump(lualib))
    for name, v in pairs(lualib) do
        local path, err = console.build(v)
        if path then
            local entry = ZipEntry(name)
            out.putNextEntry(entry)
            copy(FileInputStream(File(path)), out)
            table.insert(md5s, LuaUtil.getFileMD5(path))
            os.remove(path)
        else
            table.insert(errbuffer, err)
        end
    end

    function touint32(i)
        local code = string.format("%08x", i)
        local uint = {}
        for n in code:gmatch("..") do
            table.insert(uint, 1, string.char(tonumber(n, 16)))
        end
        return table.concat(uint)
    end

    this.update("正在打包...");
    local entry = zis.getNextEntry();
    while entry do
        local name = entry.getName()
        local lib = name:match("([^/]+%.so)$")
        if replace[name] then
        elseif lib and replace[lib] then
        elseif name:find("^assets/") then
        elseif name:find("^lua/") then
        elseif name:find("META%-INF") then
        else
            local entry = ZipEntry(name)
            out.putNextEntry(entry)
            if entry.getName() == "AndroidManifest.xml" then
                if path_pattern and #path_pattern > 1 then
                    path_pattern = ".*\\\\." .. path_pattern:match("%w+$")
                end
                local list = ArrayList()
                local xml = AXmlDecoder.read(list, zis)
                local req = {
                    [activity.getPackageName()] = packagename,
                    [info.nonLocalizedLabel] = appname,
                    [ver] = appver,
                    [".*\\\\.alp"] = path_pattern or "",
                    [".*\\\\.lua"] = "",
                    [".*\\\\.luac"] = "",
                }
                for n = 0, list.size() - 1 do
                    local v = list.get(n)
                    if req[v] then
                        list.set(n, req[v])
                    elseif user_permission then
                        local p = v:match("%.permission%.([%w_]+)$")
                        if p and (not user_permission[p]) then
                            list.set(n, "")
                        end
                    end
                end
                local pt = activity.getLuaPath(".tmp")
                local fo = FileOutputStream(pt)
                xml.write(list, fo)
                local code = activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0).versionCode
                fo.close()
                local f = io.open(pt)
                local s = f:read("a")
                f:close()
                s = string.gsub(s, touint32(code), touint32(tointeger(appcode) or 1),1)
                s = string.gsub(s, touint32(18), touint32(tointeger(appsdk) or 18),1)

                local f = io.open(pt, "w")
                f:write(s)
                f:close()
                local fi = FileInputStream(pt)
                copy(fi, out)
                os.remove(pt)
            elseif not entry.isDirectory() then
                copy2(zis, out)
            end
        end
        entry = zis.getNextEntry()
    end
    out.setComment(table.concat(md5s))
    --print(table.concat(md5s,"/n"))
    zis.close();
    out.closeEntry()
    out.close()

    if #errbuffer == 0 then
        this.update("正在签名...");
        os.remove(apkpath)
        Signer.sign(tmp, apkpath)
        os.remove(tmp)
        activity.installApk(apkpath)
        --[[import "android.net.*"
        import "android.content.*"
        i = Intent(Intent.ACTION_VIEW);
        i.setDataAndType(activity.getUriForFile(File(apkpath)), "application/vnd.android.package-archive");
        i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        this.update("正在打开...");
        activity.startActivityForResult(i, 0);]]
        return "打包成功:" .. apkpath
    else
        os.remove(tmp)
        this.update("打包出错:\n " .. table.concat(errbuffer, "\n"));
        return "打包出错:\n " .. table.concat(errbuffer, "\n")
    end
end

--luabindir=activity.getLuaExtDir("bin")
--print(activity.getLuaExtPath("bin","a"))
local function bin(path)
    local p = {}
    local e, s = pcall(loadfile(path .. "init.lua", "bt", p))
    if e then
        create_error_dlg2()
        create_bin_dlg()
        bin_dlg.show()
        activity.newTask(binapk, update, callback).execute { path, activity.getLuaExtPath("bin", p.appname .. "_" .. p.appver .. ".apk") }
    else
        Toast.makeText(activity, "工程配置文件错误." .. s, Toast.LENGTH_SHORT).show()
    end
end

--bin(activity.getLuaExtDir("project/demo").."/")
return bin