local core = require "core"
local command = require "core.command"
local config = require "core.config"
local DocView = require "core.docview"
local Doc = require "core.doc"

local cache = setmetatable({}, { __mode = "k" })


local function detect_indent(doc)
  for _, text in ipairs(doc.lines) do
    local str = text:match("^  +")
    if str then return "soft", #str end
    local str = text:match("^\t+")
    if str then return "hard" end
  end
end


local function update_cache(doc)
  local type, size = detect_indent(doc)
  if type then
    cache[doc] = { type = type, size = size }
  end
end


local new = Doc.new
function Doc:new(...)
  new(self, ...)
  update_cache(self)
end

local clean = Doc.clean
function Doc:clean(...)
  clean(self, ...)
  update_cache(self)
end


local function with_indent_override(doc, fn, ...)
  local c = cache[doc]
  if not c then
    return fn(...)
  end
  local type, size = config.tab_type, config.indent_size
  config.tab_type, config.indent_size = c.type, c.size or config.indent_size
  local r1, r2, r3 = fn(...)
  config.tab_type, config.indent_size = type, size
  return r1, r2, r3
end


local perform = command.perform
function command.perform(...)
  return with_indent_override(core.active_view.doc, perform, ...)
end


local draw = DocView.draw
function DocView:draw(...)
  return with_indent_override(self.doc, draw, self, ...)
end
