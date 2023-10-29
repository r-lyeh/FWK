local core = require "core"
local common = require "core.common"
local command = require "core.command"
local config = require "core.config"
local keymap = require "core.keymap"
local style = require "core.style"
local View = require "core.view"


local TodoTreeView = View:extend()

config.todo_tags = { --"TODO", "BUG", "FIX", "FIXME", "IMPROVEMENT",
  "@todo", "@fixme", "@testme", "@leak" } --< @r-lyeh

-- Paths or files to be ignored
config.todo_ignore_paths = {
   "tools/tcc",     --< @r-lyeh
   "tools\\tcc",    --< @r-lyeh
   "engine/fwk",    --< @r-lyeh
   "engine\\fwk",   --< @r-lyeh
   "engine/joint",  --< @r-lyeh
   "engine\\joint", --< @r-lyeh
}

-- 'tag' mode can be used to group the todos by tags
-- 'file' mode can be used to group the todos by files
config.todo_mode = "tag"

-- Tells if the plugin should start with the nodes expanded. default: true for tag mode
config.todo_expanded = config.todo_mode == "tag"

-- list of allowed extensions: items must start and end with a dot character
config.todo_allowed_extensions = '.h.c.m.hh.cc.hpp.cpp.cxx.lua.py.cs.vs.fs.bat.' --< @r-lyeh

-- whether the sidebar treeview is initially visible or not
config.todo_visible = false


function TodoTreeView:new()
  TodoTreeView.super.new(self)
  self.scrollable = true
  self.focusable = false
  self.visible = config.todo_visible
  self.times_cache = {}
  self.cache = {}
  self.cache_updated = false
  self.init_size = true

  -- Items are generated from cache according to the mode
  self.items = {}
end

local function is_file_ignored(filename)
  for _, path in ipairs(config.todo_ignore_paths) do
    local s, _ = filename:find(path)
    if s then
      return true
    end
  end

  return false
end

function TodoTreeView:refresh_cache()
  local items = {}
  if not next(self.items) then
    items = self.items
  end
  self.updating_cache = true

  core.add_thread(function()
    for _, item in ipairs(core.project_files) do
      local ignored = is_file_ignored(item.filename)
      if not ignored and item.type == "file" then
        local cached = self:get_cached(item)

        if config.todo_mode == "file" then
          items[cached.filename] = cached
        else
          for _, todo in ipairs(cached.todos) do
            local tag = todo.tag
            if not items[tag] then
              local t = {}
              t.expanded = config.todo_expanded
              t.type = "group"
              t.todos = {}
              t.tag = tag
              items[tag] = t
            end

            table.insert(items[tag].todos, todo)
          end
        end
      end
    end

    -- Copy expanded from old items
    if config.todo_mode == "tag" and next(self.items) then
      for tag, data in pairs(self.items) do
        if items[tag] then
          items[tag].expanded = data.expanded
        end
      end
    end

    self.items = items
    core.redraw = true
    self.cache_updated = true
    self.updating_cache = false
  end, self)
end


local function find_file_todos(t, filename)
  --< @r-lyeh
  local ext = (filename:match "[^.]+$") .. '.'
  if not string.find(config.todo_allowed_extensions,ext) then
    return
  end
  --<

  local fp = io.open(filename)
  if not fp then return t end

  --< @r-lyeh: optimized loops: early exit if quicksearch fails
  local function lines(str)
    local result = {}
    for line in string.gmatch(str, "(.-)%c") do -- line in str:gmatch '[^\n]+' do
      -- Add spaces at the start and end of line so the pattern will pick
      -- tags at the start and at the end of lines
      table.insert(result, " "..line.." ")
    end
    return result
  end
  local before = #t
  local content = fp:read("*all")
  for _, todo_tag in ipairs(config.todo_tags) do
    if string.find(content, todo_tag) then
      local n = 0
      for _, line in ipairs(lines(content)) do
        n = n + 1
        local match_str = todo_tag[1] == '@' and todo_tag or "[^a-zA-Z_\"'`]"..todo_tag.."[^a-zA-Z_\"'`]+"
        local s, e = line:find(match_str)
        if s then
          local d = {}
          d.tag = string.sub(string.upper(todo_tag), todo_tag:byte(1) == 64 and 2 or 1) .. 's'
          d.filename = filename
          d.text = line:sub(e+1)
          if d.text == "" then
            d.text = config.todo_mode == "tag" and filename:match("^.+[/\\](.+)$") or "blank"
          end
          d.line = n
          d.col = s
          table.insert(t, d)
        end
      end
    end
  end
  fp:close()
  if #t ~= before then
    coroutine.yield()
    core.redraw = true
  end
  --<
end


function TodoTreeView:get_cached(item)
  local t = self.cache[item.filename]
  if not t then
    t = {}
    t.expanded = config.todo_expanded
    t.filename = item.filename
    t.abs_filename = system.absolute_path(item.filename)
    t.type = item.type
    t.todos = {}
    find_file_todos(t.todos, t.filename)
    self.cache[t.filename] = t
  end
  return t
end


function TodoTreeView:get_name()
  return "Todo Tree"
end


function TodoTreeView:get_item_height()
  return style.font:get_height() + style.padding.y
end


function TodoTreeView:get_cached_time(doc)
  local t = self.times_cache[doc]
  if not t then
    local info = system.get_file_info(doc.filename)
    if not info then return nil end
    self.times_cache[doc] = info.modified
  end
  return t
end


function TodoTreeView:check_cache()
  for _, doc in ipairs(core.docs) do
    if doc.filename then
      local info = system.get_file_info(doc.filename)
      local cached = self:get_cached_time(doc)
      if not info and cached then
        -- document deleted
        self.times_cache[doc] = nil
        self.cache[doc.filename] = nil
        self.cache_updated = false
      elseif cached and cached ~= info.modified then
        -- document modified
        self.times_cache[doc] = info.modified
        self.cache[doc.filename] = nil
        self.cache_updated = false
      end
    end
  end

  if core.project_files ~= self.last_project_files then
    self.last_project_files = core.project_files
    self.cache_updated = false
  end
end

function TodoTreeView:each_item()
  self:check_cache()
  if not self.updating_cache and not self.cache_updated then
    self:refresh_cache()
  end

  return coroutine.wrap(function()
    local ox, oy = self:get_content_offset()
    local y = oy + style.padding.y
    local w = self.size.x
    local h = self:get_item_height()

    for _, item in pairs(self.items) do
      if #item.todos > 0 then
        coroutine.yield(item, ox, y, w, h)
        y = y + h

        for _, todo in ipairs(item.todos) do
          if item.expanded then
            coroutine.yield(todo, ox, y, w, h)
            y = y + h
          end
        end
      end
    end
  end)
end


function TodoTreeView:on_mouse_moved(px, py)
  self.hovered_item = nil
  for item, x,y,w,h in self:each_item() do
    if px > x and py > y and px <= x + w and py <= y + h then
      self.hovered_item = item
      break
    end
  end
end


function TodoTreeView:on_mouse_pressed(button, x, y)
  if not self.hovered_item then
    return
  elseif self.hovered_item.type == "file"
    or self.hovered_item.type == "group" then
    self.hovered_item.expanded = not self.hovered_item.expanded
  else
    core.try(function()
      local i = self.hovered_item
      local dv = core.root_view:open_doc(core.open_doc(i.filename))
      core.root_view.root_node:update_layout()
      dv.doc:set_selection(i.line, i.col)
      dv:scroll_to_line(i.line, false, true)
    end)
  end
end


function TodoTreeView:update()
  self.scroll.to.y = math.max(0, self.scroll.to.y)

  -- update width
  local dest = self.visible and config.treeview_size or 0
  if self.init_size then
    self.size.x = dest
    self.init_size = false
  else
    self:move_towards(self.size, "x", dest)
  end

  TodoTreeView.super.update(self)
end


function TodoTreeView:draw()
  self:draw_background(style.background2)

  --local h = self:get_item_height()
  local icon_width = style.icon_font:get_width("D")
  local spacing = style.font:get_width(" ") * 2
  local root_depth = 0

  for item, x,y,w,h in self:each_item() do
    local color = style.text

    -- hovered item background
    if item == self.hovered_item then
      renderer.draw_rect(x, y, w, h, style.line_highlight)
      color = style.accent
    end

    -- icons
    local item_depth = 0
    x = x + (item_depth - root_depth) * style.padding.x + style.padding.x
    if item.type == "file" then
      local icon1 = item.expanded and "-" or "+"
      common.draw_text(style.icon_font, color, icon1, nil, x, y, 0, h)
      x = x + style.padding.x
      common.draw_text(style.icon_font, color, "f", nil, x, y, 0, h)
      x = x + icon_width
    elseif item.type == "group" then
      local icon1 = item.expanded and "-" or "+"
      common.draw_text(style.icon_font, color, icon1, nil, x, y, 0, h)
      x = x + icon_width / 2
    else
      if config.todo_mode == "tag" then
        x = x + style.padding.x
      else
        x = x + style.padding.x * 1.5
      end
      common.draw_text(style.icon_font, color, "i", nil, x, y, 0, h)
      x = x + icon_width
    end

    -- text
    x = x + spacing
    if item.type == "file" then
      common.draw_text(style.font, color, item.filename, nil, x, y, 0, h)
    elseif item.type == "group" then
      common.draw_text(style.font, color, item.tag, nil, x, y, 0, h)
    else
      if config.todo_mode == "file" then
        common.draw_text(style.font, color, item.tag.." - "..item.text, nil, x, y, 0, h)
      else
        common.draw_text(style.font, color, item.text, nil, x, y, 0, h)
      end
    end
  end
end


-- init
local view = TodoTreeView()
local node = core.root_view:get_active_node()
view.size.x = config.treeview_size
node:split("right", view, true)

-- register commands and keymap
command.add(nil, {
  ["todotreeview:toggle"] = function()
    view.visible = not view.visible
  end,

  ["todotreeview:expand-items"] = function()
    for _, item in pairs(view.items) do
      item.expanded = true
    end
  end,

  ["todotreeview:hide-items"] = function()
    for _, item in pairs(view.items) do
      item.expanded = false
    end
  end,
})

keymap.add { ["ctrl+shift+t"] = "todotreeview:toggle" }
keymap.add { ["ctrl+shift+e"] = "todotreeview:expand-items" }
keymap.add { ["ctrl+shift+h"] = "todotreeview:hide-items" }

