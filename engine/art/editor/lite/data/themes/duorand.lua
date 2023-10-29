local style = require "core.style"
local common = require "core.common"

math.randomseed(os.time())

local color = {
	math.random(90, 255),
	math.random(90, 255),
	math.random(90, 255)
}

style.background = { common.color "#151515" }
style.background2 = { common.color "#151515" }
style.background3 = { common.color "#151515" }
style.text = { common.color "#707070" }
style.caret = { common.color "#dfdfdf" }
style.accent = { common.color "#d0d0d0" }
style.dim = { common.color "#303030" }
style.divider = { common.color "#151515" }
style.selection = { common.color "#303030" }
style.line_number = { common.color "#252525" }
style.line_number2 = { common.color "#444444" }
style.line_highlight = { common.color "#101010" }
style.scrollbar = { common.color "#252525" }
style.scrollbar2 = { common.color "#444444" }

style.syntax = {}
style.syntax["normal"] = { common.color "#a0a0a0" }
style.syntax["symbol"] = { common.color "#a0a0a0" }
style.syntax["comment"] = { common.color "#404040" }
style.syntax["keyword"] = { common.color "#dfdfdf" }
style.syntax["keyword2"] = { common.color "#dfdfdf" }
style.syntax["number"] = { common.color "#dfdfdf" }
style.syntax["literal"] = { common.color "#dfdfdf" }
style.syntax["string"] = { common.color "#dfdfdf" }
style.syntax["operator"] = color
style.syntax["function"] = color
