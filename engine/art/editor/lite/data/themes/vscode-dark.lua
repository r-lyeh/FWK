-- Most of the colors are taken from:
-- https://github.com/microsoft/vscode/tree/master/extensions/theme-defaults/themes

local style = require "core.style"
local common = require "core.common"

style.background = { common.color "#1E1E1E" }
style.background2 = { common.color "#252526" }
style.background3 = { common.color "#252526" }
style.text = { common.color "#D4D4D4" }
style.caret = { common.color "#FFFFFF" }
style.accent = { common.color "#76BCFF" } -- Text in autocomplete and command, col(>80) in satusbar
style.dim = { common.color "#7A7A7A" } -- Text of nonactive tabs, prefix in log
style.divider = { common.color "#1E1E1E" }
style.selection = { common.color "#264F78" }
style.line_number = { common.color "#707070" }
style.line_number2 = { common.color "#A0A0A0" } -- Number on line with caret
style.line_highlight = { common.color "#333A40"}
style.scrollbar = { common.color "#404040" }
style.scrollbar2 = { common.color "#707070" } -- Hovered

style.syntax["normal"] = { common.color "#D4D4D4" }
style.syntax["symbol"] = { common.color "#D4D4D4" }
style.syntax["comment"] = { common.color "#6A9955" }
style.syntax["keyword"] = { common.color "#569CD6" }  -- local function end, if case
style.syntax["keyword2"] = { common.color "#C586C0" } -- self, int float
style.syntax["number"] = { common.color "#B5CEA8" }
style.syntax["literal"] = { common.color "#569CD6" }
style.syntax["string"] = { common.color "#CE9178" }
style.syntax["operator"] = { common.color "#8590A5"}  -- = + - / < >
style.syntax["function"] = { common.color "#DCDCAA" }

-- PLUGINS
style.linter_warning = { common.color "#B89500" }     -- linter
style.bracketmatch_color = { common.color "#76BCFF" } -- bracketmatch
style.guide = { common.color "#404040" }              -- indentguide
style.guide_width = 1                                 -- indentguide
