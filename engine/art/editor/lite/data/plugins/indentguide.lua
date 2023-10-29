local style = require "core.style"
local config = require "core.config"
local DocView = require "core.docview"


local function get_line_spaces(doc, idx, dir)
  local text = doc.lines[idx]
  if not text then
    return 0
  end
  local s, e = text:find("^%s*")
  if e == #text then
    return get_line_spaces(doc, idx + dir, dir)
  end
  local n = 0
  for i = s, e do
    n = n + (text:byte(i) == 9 and config.indent_size or 1)
  end
  return n
end


local function get_line_indent_guide_spaces(doc, idx)
  if doc.lines[idx]:find("^%s*\n") then
    return math.max(
      get_line_spaces(doc, idx - 1, -1),
      get_line_spaces(doc, idx + 1,  1))
  end
  return get_line_spaces(doc, idx)
end


local draw_line_text = DocView.draw_line_text

function DocView:draw_line_text(idx, x, y)
  local spaces = get_line_indent_guide_spaces(self.doc, idx)
  local sw = self:get_font():get_width(" ")
  local w = math.ceil(1 * SCALE)
  local h = self:get_line_height()
  for i = 0, spaces - 1, config.indent_size do
    local color = style.guide or style.selection
    renderer.draw_rect(x + sw * i, y, w, h, color)
  end
  draw_line_text(self, idx, x, y)
end
