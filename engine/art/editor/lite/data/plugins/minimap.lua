local command   = require "core.command"
local common    = require "core.common"
local config    = require "core.config"
local style     = require "core.style"
local DocView   = require "core.docview"

-- General plugin settings
config.minimap_enabled          = true
config.minimap_width            = 100
config.minimap_instant_scroll   = false
config.minimap_syntax_highlight = true
config.minimap_scale            = 1
config.minimap_draw_background  = true

-- Configure size for rendering each char in the minimap
local char_height  = 1 * SCALE * config.minimap_scale
local char_spacing = 0.8 * SCALE * config.minimap_scale
local line_spacing = 2 * SCALE * config.minimap_scale

-- Overloaded since the default implementation adds a extra x3 size of hotspot for the mouse to hit the scrollbar.
local prev_scrollbar_overlaps_point = DocView.scrollbar_overlaps_point
DocView.scrollbar_overlaps_point = function(self, x, y)
  if not config.minimap_enabled then return prev_scrollbar_overlaps_point(self, x, y) end

  local sx, sy, sw, sh = self:get_scrollbar_rect()
  return x >= sx and x < sx + sw and y >= sy and y < sy + sh
end

-- Helper function to determine if current file is too large to be shown fully inside the minimap area.
local function is_file_too_large(self)
    local line_count = #self.doc.lines
    local _, _, _, sh = self:get_scrollbar_rect()

    -- check if line count is too large to fit inside the minimap area
    local max_minmap_lines = math.floor(sh / line_spacing)
    return line_count > 1 and line_count > max_minmap_lines
end

-- Overloaded with an extra check if the user clicked inside the minimap to automatically scroll to that line.
local prev_on_mouse_pressed = DocView.on_mouse_pressed
DocView.on_mouse_pressed = function(self, button, x, y, clicks)
  if not config.minimap_enabled then return prev_on_mouse_pressed(self, button, x, y, clicks) end

  -- check if user clicked in the minimap area and jump directly to that line
  -- unless they are actually trying to perform a drag
  local minimap_hit = self:scrollbar_overlaps_point(x, y)
  if minimap_hit then
    local line_count = #self.doc.lines
    local minimap_height = line_count * line_spacing

    -- check if line count is too large to fit inside the minimap area
    local is_too_large = is_file_too_large(self)
    if is_too_large then
      local _, _, _, sh = self:get_scrollbar_rect()
      minimap_height = sh
    end

    -- calc which line to jump to
    local dy = y - self.position.y
    local jump_to_line = math.floor((dy / minimap_height) * line_count) + 1

    local _, cy, _, cy2 = self:get_content_bounds()
    local lh = self:get_line_height()
    local visible_lines_count = math.max(1, (cy2 - cy) / lh)
    local visible_lines_start = math.max(1, math.floor(cy / lh))

    -- calc if user hit the currently visible area
    local hit_visible_area = true
    if is_too_large then

      local visible_height = visible_lines_count * line_spacing
      local scroll_pos = (visible_lines_start-1) / (line_count - visible_lines_count - 1)
      scroll_pos = math.min(1.0, scroll_pos) -- 0..1
      local visible_y = self.position.y + scroll_pos * (minimap_height - visible_height)

      local t = (line_count - visible_lines_start) / visible_lines_count
      if t <= 1 then
        visible_y = visible_y + visible_height * (1.0 - t)
      end

      if y < visible_y or y > visible_y + visible_height then
        hit_visible_area = false
      end
    else

      -- If the click is on the currently visible line numbers,
      -- ignore it since then they probably want to initiate a drag instead.
      if jump_to_line < visible_lines_start or jump_to_line > visible_lines_start + visible_lines_count then
        hit_visible_area = false
      end
    end

    -- if user didn't click on the visible area (ie not dragging), scroll accordingly
    if not hit_visible_area then
      self:scroll_to_line(jump_to_line, false, config.minimap_instant_scroll)
      return
    end

  end

  return prev_on_mouse_pressed(self, button, x, y, clicks)
end

-- Overloaded with pretty much the same logic as original DocView implementation,
-- with the exception of the dragging scrollbar delta. We want it to behave a bit snappier
-- since the "scrollbar" essentially represents the lines visible in the content view.
local prev_on_mouse_moved = DocView.on_mouse_moved
DocView.on_mouse_moved = function(self, x, y, dx, dy)
  if not config.minimap_enabled then return prev_on_mouse_moved(self, x, y, dx, dy) end

  if self.dragging_scrollbar then
    local line_count = #self.doc.lines
    local lh = self:get_line_height()
    local delta = lh / line_spacing * dy

    if is_file_too_large(self) then
      local _, sy, _, sh = self:get_scrollbar_rect()
      delta = (line_count * lh) / sh * dy
    end

    self.scroll.to.y = self.scroll.to.y + delta
  end

  -- we need to "hide" that the scrollbar is dragging so that View doesnt does its own scrolling logic
  local t = self.dragging_scrollbar
  self.dragging_scrollbar = false
  local r = prev_on_mouse_moved(self, x, y, dx, dy)
  self.dragging_scrollbar = t
  return r
end

-- Overloaded since we want the mouse to interact with the full size of the minimap area,
-- not juse the scrollbar.
local prev_get_scrollbar_rect = DocView.get_scrollbar_rect
DocView.get_scrollbar_rect = function (self)
  if not config.minimap_enabled then return prev_get_scrollbar_rect(self) end

  return
    self.position.x + self.size.x - config.minimap_width * SCALE,
    self.position.y,
    config.minimap_width * SCALE,
    self.size.y
end

-- Overloaded so we can render the minimap in the "scrollbar area".
local prev_draw_scrollbar = DocView.draw_scrollbar
DocView.draw_scrollbar = function (self)
  if not config.minimap_enabled then return prev_draw_scrollbar(self) end

  local x, y, w, h = self:get_scrollbar_rect()

  local highlight = self.hovered_scrollbar or self.dragging_scrollbar
  local visual_color = highlight and style.scrollbar2 or style.scrollbar

  local _, cy, _, cy2 = self:get_content_bounds()
  local lh = self:get_line_height()
  local visible_lines_count = math.max(1, (cy2 - cy) / lh)
  local visible_lines_start = math.max(1, math.floor(cy / lh))
  local scroller_height = visible_lines_count * line_spacing
  local line_count = #self.doc.lines

  local visible_y = self.position.y + (visible_lines_start-1) * line_spacing


  -- check if file is too large to fit inside the minimap area
  local max_minmap_lines = math.floor(h / line_spacing)
  local minimap_start_line = 1
  if is_file_too_large(self) then

    local scroll_pos = (visible_lines_start-1) / (line_count - visible_lines_count - 1)
    scroll_pos = math.min(1.0, scroll_pos) -- 0..1, procent of visual area scrolled

    local scroll_pos_pixels = scroll_pos * (h - scroller_height)
    visible_y = self.position.y + scroll_pos_pixels

    -- offset visible area if user is scrolling past end
    local t = (line_count - visible_lines_start) / visible_lines_count
    if t <= 1 then
      visible_y = visible_y + scroller_height * (1.0 - t)
    end

    minimap_start_line = visible_lines_start - math.floor(scroll_pos_pixels / line_spacing)
    minimap_start_line = math.max(1, math.min(minimap_start_line, line_count - max_minmap_lines))
  end

  if config.minimap_draw_background then
    renderer.draw_rect(x, y, w, h, style.minimap_background or style.background)
  end
  -- draw visual rect
  renderer.draw_rect(x, visible_y, w, scroller_height, visual_color)

  -- time to draw the actual code, setup some local vars that are used in both highlighted and plain renderind.
  local line_y = y

  -- when not using syntax highlighted rendering, just use the normal color but dim it 50%.
  local color = style.syntax["normal"]
  color = { color[1],color[2],color[3],color[4] * 0.5 }

  -- we try to "batch" characters so that they can be rendered as just one rectangle instead of one for each.
  local batch_width = 0
  local batch_start = x
  local minimap_cutoff_x = x + config.minimap_width * SCALE

  -- render lines with syntax highlighting
  if config.minimap_syntax_highlight then

    -- keep track of the highlight type, since this needs to break batches as well
    local batch_syntax_type = nil

    local function flush_batch(type)
      if batch_width > 0 then
        -- fetch and dim colors
        color = style.syntax[batch_syntax_type]
        color = { color[1], color[2], color[3], color[4] * 0.5 }
        renderer.draw_rect(batch_start, line_y, batch_width, char_height, color)
      end
      batch_syntax_type = type
      batch_start = batch_start + batch_width
      batch_width = 0
    end

    -- per line
    local endidx = minimap_start_line + max_minmap_lines
    endidx = math.min(endidx, line_count)
    for idx=minimap_start_line,endidx do
      batch_syntax_type = nil
      batch_start = x
      batch_width = 0

      -- per token
      for _, type, text in self.doc.highlighter:each_token(idx) do
        -- flush prev batch
        if not batch_syntax_type then batch_syntax_type = type end
        if batch_syntax_type ~= type then
          flush_batch(type)
        end

        -- per character
        for char in common.utf8_chars(text) do
          if char == " " or char == "\n" then
            flush_batch(type)
            batch_start = batch_start + char_spacing
          elseif batch_start + batch_width > minimap_cutoff_x then
            flush_batch(type)
            break
          else
            batch_width = batch_width + char_spacing
          end

        end
      end
      flush_batch(nil)
      line_y = line_y + line_spacing
    end

  else  -- render lines without syntax highlighting

    local function flush_batch()
      if batch_width > 0 then
        renderer.draw_rect(batch_start, line_y, batch_width, char_height, color)
      end
      batch_start = batch_start + batch_width
      batch_width = 0
    end

    for idx=1,line_count-1 do
      batch_start = x
      batch_width = 0

      for char in common.utf8_chars(self.doc.lines[idx]) do
        if char == " " or char == "\n" then
          flush_batch()
          batch_start = batch_start + char_spacing
        elseif batch_start + batch_width > minimap_cutoff_x then
            flush_batch()
        else
          batch_width = batch_width + char_spacing
        end
      end
      flush_batch()
      line_y = line_y + line_spacing
    end

  end

end

command.add(nil, {
  ["minimap:toggle-visibility"] = function()
    config.minimap_enabled = not config.minimap_enabled
  end,
  ["minimap:toggle-syntax-highlighting"] = function()
    config.minimap_syntax_highlight = not config.minimap_syntax_highlight
  end,
})
