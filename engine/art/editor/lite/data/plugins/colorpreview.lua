local common = require "core.common"
local DocView = require "core.docview"


local white = { common.color "#ffffff" }
local black = { common.color "#000000" }
local tmp = {}


local function draw_color_previews(self, idx, x, y, ptn, base, nibbles)
  local text = self.doc.lines[idx]
  local s, e = 0, 0

  while true do
    s, e = text:find(ptn, e + 1)
    if not s then break end

    local str = text:sub(s, e)
    local r, g, b = str:match(ptn)
    r, g, b = tonumber(r, base), tonumber(g, base), tonumber(b, base)

    -- #123 becomes #112233
    if nibbles then
      r = r * 16
      g = g * 16
      b = b * 16
    end

    local x1 = x + self:get_col_x_offset(idx, s)
    local x2 = x + self:get_col_x_offset(idx, e + 1)
    local oy = self:get_line_text_y_offset()

    local text_color = math.max(r, g, b) < 128 and white or black
    tmp[1], tmp[2], tmp[3] = r, g, b

    local l1, _, l2, _ = self.doc:get_selection(true)

    if not (self.doc:has_selection() and idx >= l1 and idx <= l2) then
      renderer.draw_rect(x1, y, x2 - x1, self:get_line_height(), tmp)
      renderer.draw_text(self:get_font(), str, x1, y + oy, text_color)
    end
  end
end


local draw_line_text = DocView.draw_line_text

function DocView:draw_line_text(idx, x, y)
  draw_line_text(self, idx, x, y)
  draw_color_previews(self, idx, x, y, "#(%x%x)(%x%x)(%x%x)%f[%W]",        16)
  draw_color_previews(self, idx, x, y, "#(%x)(%x)(%x)%f[%W]",              16, true) -- support #fff css format
  draw_color_previews(self, idx, x, y, "rgb?%((%d+)%D+(%d+)%D+(%d+).-%)",  10)
  draw_color_previews(self, idx, x, y, "rgba?%((%d+)%D+(%d+)%D+(%d+).-%)", 10)
end
