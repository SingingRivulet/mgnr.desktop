path_font = "./res/font/font.ttf"
path_sf2 = "./res/soundfont/sndfnt.sf2"
function modulesInit(self)

    module_helloworld = require("modules.helloworld.helloworld")
    registerModule(self, module_helloworld)
    
end
