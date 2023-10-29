local core = require "core"
local config = require "core.config"
local style = require "core.style"
local Doc = require "core.doc"
local DocView = require "core.docview"

config.motiontrail_steps = config.motiontrail_steps or 50

local function doc()
  return core.active_view.doc
end

local function lerp(a, b, t)
  return a + (b - a) * t
end


local function get_caret_rect(dv, idx)
  local line1, col1, line2, col2 = doc():get_selection_idx(idx)
  local x1, y1 = dv:get_line_screen_position(line1)
  x1 = x1 + dv:get_col_x_offset(line1, col1)
  return x1, y1, style.caret_width, dv:get_line_height()
end


local last_x = {}
local last_y = {}
local last_view = {}

local draw = DocView.draw

function DocView:draw(...)
  draw(self, ...)
  if self ~= core.active_view then return end

  for idx, line1, col1, line2, col2 in doc():get_selections(true, true) do
    --if line1 == line2 and col1 == col2 then return false end

    local x, y, w, h = get_caret_rect(self, idx)

    if last_view[idx] == self and (x ~= last_x[idx] or y ~= last_y[idx]) then
      local lx = x
      for i = 0, 1, 1 / config.motiontrail_steps do
        local ix = lerp(x, last_x[idx], i)
        local iy = lerp(y, last_y[idx], i)
        local iw = math.max(w, math.ceil(math.abs(ix - lx)))
        renderer.draw_rect(ix, iy, iw, h, style.caret)
        lx = ix
      end
      core.redraw = true
    end

    last_view[idx], last_x[idx], last_y[idx] = self, x, y
  end
end

