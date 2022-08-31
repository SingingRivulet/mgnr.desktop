local index = 10
return {
    name = "helloworld",
    init = function()
        print("helloworld_init")
    end,
    shutdown = function()
        print("helloworld_shutdown")
    end,
    draw = function()
        index = index + 1
        imgui.ImGui.TextUnformatted("helloworld_draw" .. index)
    end
}
