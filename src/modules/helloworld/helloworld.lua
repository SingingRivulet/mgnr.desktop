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
        -- libhelloworld.drawUI()
    end,
    loop = function()
    end,
    vscript = {
        input = {{
            name = "输入0",
            type = "字符串"
        }, {
            name = "输入1",
            type = "字符串"
        }},
        output = {},
        menu = "测试节点",
        exec = function(global, self)
            local i0 = vscript.getInput_string(self, 0)
            local i1 = vscript.getInput_string(self, 1)
            vscript.print(self, "exec helloworld")
            vscript.print(self, "i0=" .. i0)
            vscript.print(self, "i1=" .. i1)
        end
    }
}
