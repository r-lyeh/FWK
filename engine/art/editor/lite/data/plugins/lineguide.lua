local config = require "core.config"
local style = require "core.style"
local DocView = require "core.docview"

local draw = DocView.draw

function DocView:draw(...)
  draw(self, ...)

  local offset = self:get_font():get_width("n") * config.line_limit
  local x = self:get_line_screen_position(1) + offset
  local y = self.position.y
  local w = math.ceil(SCALE * 1)
  local h = self.size.y

  local color = style.guide or style.selection
  renderer.draw_rect(x, y, w, h, color)
end
