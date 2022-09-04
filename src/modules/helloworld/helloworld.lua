local libhelloworld = require("modules.helloworld.libhelloworld")
local index = 10
return {
    name = "helloworld",
    init = function()
        print("helloworld_init")
    end,
    shutdown = function()
        print("helloworld_shutdown")
    end,
    drawUI = function()
        index = index + 1
        imgui.ImGui.TextUnformatted("helloworld_draw" .. index)
        libhelloworld.drawUI()
    end,
    loop = function()
    end
}
