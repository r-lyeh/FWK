-- Markers plugin for lite text editor
-- original implementation by Petri Häkkinen

local core = require "core"
local command = require "core.command"
local keymap = require "core.keymap"
local style = require "core.style"
local DocView = require "core.docview"
local Doc = require "core.doc"

local cache = {}  -- this table contains subtables for each document, each subtable is a set of line numbers
setmetatable(cache, {
  __mode = "k",
  __index = function(t, k)
    t[k] = {}
    return t[k]
  end,
})


local function shift_lines(doc, at, diff)
  if diff == 0 then return end
  local t = {}
  for line in pairs(cache[doc]) do
    line = line >= at and line + diff or line
    t[line] = true
  end
  cache[doc] = t
end


local raw_insert = Doc.raw_insert

function Doc:raw_insert(line, col, text, ...)
  raw_insert(self, line, col, text, ...)
  local line_count = 0
  for _ in text:gmatch("\n") do
    line_count = line_count + 1
  end
  shift_lines(self, line, line_count)
end


local raw_remove = Doc.raw_remove

function Doc:raw_remove(line1, col1, line2, col2, ...)
  raw_remove(self, line1, col1, line2, col2, ...)
  shift_lines(self, line2, line1 - line2)
end


local draw_line_gutter = DocView.draw_line_gutter

function DocView:draw_line_gutter(idx, x, y)
  if cache[self.doc] and cache[self.doc][idx] then
    local h = self:get_line_height()
    renderer.draw_rect(x, y, style.padding.x * 0.4, h, style.selection)
  end
  draw_line_gutter(self, idx, x, y)
end


command.add("core.docview", {
  ["markers:toggle-marker"] = function()
    local doc = core.active_view.doc
    local line = doc:get_selection()
    local markers = cache[doc]

    if markers[line] then
      markers[line] = nil
    else
      markers[line] = true
    end
  end,

  ["markers:go-to-next-marker"] = function()
    local doc = core.active_view.doc
    local line = doc:get_selection()
    local markers = cache[doc]

    local first_marker = math.huge
    local next_marker = math.huge
    for l, _ in pairs(markers) do
      if l > line and l < next_marker then
        next_marker = l
      end
      first_marker = math.min(first_marker, l)
    end
    if next_marker == math.huge then
      next_marker = first_marker
    end
    if next_marker ~= math.huge then
      doc:set_selection(next_marker, 1)
      core.active_view:scroll_to_line(next_marker, true)
    end
  end,
})


keymap.add {
  ["ctrl+f2"] = "markers:toggle-marker",
  ["f2"] = "markers:go-to-next-marker",
}
