path_font = "./res/font/font.ttf"
path_sf2 = "./res/soundfont/sndfnt.sf2"
path_imgui_ini = "imgui.ini"
path_imgui_log = "imgui_log.txt"

if env_mgnr_path ~= nil then
    path_font = env_mgnr_path .. "/res/font/font.ttf"
    path_sf2 = env_mgnr_path .. "/res/soundfont/sndfnt.sf2"
    path_imgui_ini = env_mgnr_path .. "/imgui.ini"
    path_imgui_log = env_mgnr_path .. "/imgui_log.txt"
    package.path = env_mgnr_path..'/?.lua;/?/init.lua;' -- 搜索lua模块
    package.cpath = env_mgnr_path..'/?.so;' -- 搜索so模块
end
print("path_font:" .. path_font)
print("path_sf2:" .. path_sf2)

function modulesInit(self)

    module_helloworld = require("modules.helloworld.helloworld")
    registerModule(self, module_helloworld)

end
