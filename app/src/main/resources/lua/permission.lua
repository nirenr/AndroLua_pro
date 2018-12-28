require "import"
import "android.content.pm.PackageManager"
local pm = activity.getPackageManager();
local pi = pm.getPackageInfo(activity.getPackageName(), 0);
-- 得到自己的包名
local pkgName = pi.packageName;

local pkgInfo = pm.getPackageInfo(pkgName,
    PackageManager.GET_PERMISSIONS); --通过包名，返回包信息
local sharedPkgList = pkgInfo.requestedPermissions; --得到权限列表
permission = {}
permission_info = {}
permission_info["BIND_ACCESSIBILITY_SERVICE"] = "辅助功能"
for i = 0, #sharedPkgList - 1 do
    local permName = sharedPkgList[i];
    --print(permName)
    local s, tmpPermInfo = pcall(pm.getPermissionInfo, permName, 0); --通过permName得到该权限的详细信息

    --local pgi = pm.getPermissionGroupInfo( tmpPermInfo.group, 0);--权限分为不同的群组，通过权限名，我们得到该权限属于什么类型的权限。
    --print(i , "-" , permName );
    --print(tmpPermInfo)
    -- print(i , "-" , pgi.loadLabel(pm) );
    --print(i , "-" , tmpPermInfo.loadLabel(pm));
    --print(i , "-" , tmpPermInfo.loadDescription(pm));
    permName = permName:match("%.([%w_]+)$")
    table.insert(permission, permName)
    if s then
        permission_info[permName] = tmpPermInfo.loadLabel(pm)
    else
        permission_info[permName] = permName
    end
end
