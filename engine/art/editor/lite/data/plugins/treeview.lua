local core = require "core"
local common = require "core.common"
local command = require "core.command"
local config = require "core.config"
local keymap = require "core.keymap"
local style = require "core.style"
local View = require "core.view"

config.treeview_size = 200 * SCALE

local function get_depth(filename)
  local n = 0
  for sep in filename:gmatch("[\\/]") do
    n = n + 1
  end
  return n
end


local TreeView = View:extend()

function TreeView:new()
  TreeView.super.new(self)
  self.scrollable = true
  self.visible = false --< @r-lyeh true>false
  self.init_size = true
  self.cache = {}
end


function TreeView:get_cached(item)
  local t = self.cache[item.filename]
  if not t then
    t = {}
    t.filename = item.filename
    t.abs_filename = system.absolute_path(item.filename)
    t.name = t.filename:match("[^\\/]+$")
    t.depth = get_depth(t.filename)
    t.type = item.type
    self.cache[t.filename] = t
  end
  return t
end


function TreeView:get_name()
  return "Project"
end


function TreeView:get_item_height()
  return style.font:get_height() + style.padding.y
end


function TreeView:check_cache()
  -- invalidate cache's skip values if project_files has changed
  if core.project_files ~= self.last_project_files then
    for _, v in pairs(self.cache) do
      v.skip = nil
    end
    self.last_project_files = core.project_files
  end
end


function TreeView:each_item()
  return coroutine.wrap(function()
    self:check_cache()
    local ox, oy = self:get_content_offset()
    local y = oy + style.padding.y
    local w = self.size.x
    local h = self:get_item_height()

    local i = 1
    while i <= #core.project_files do
      local item = core.project_files[i]
      local cached = self:get_cached(item)

      coroutine.yield(cached, ox, y, w, h)
      y = y + h
      i = i + 1

      if not cached.expanded then
        if cached.skip then
          i = cached.skip
        else
          local depth = cached.depth
          while i <= #core.project_files do
            local filename = core.project_files[i].filename
            if get_depth(filename) <= depth then break end
            i = i + 1
          end
          cached.skip = i
        end
      end
    end
  end)
end


function TreeView:on_mouse_moved(px, py)
  self.hovered_item = nil
  for item, x,y,w,h in self:each_item() do
    if px > x and py > y and px <= x + w and py <= y + h then
      self.hovered_item = item
      break
    end
  end
end


function TreeView:on_mouse_pressed(button, x, y)
  if not self.hovered_item then
    return
  elseif self.hovered_item.type == "dir" then
    self.hovered_item.expanded = not self.hovered_item.expanded
  else
    core.try(function()
      core.root_view:open_doc(core.open_doc(self.hovered_item.filename))
    end)
  end
end


function TreeView:update()
  -- update width
  local dest = self.visible and config.treeview_size or 0
  if self.init_size then
    self.size.x = dest
    self.init_size = false
  else
    self:move_towards(self.size, "x", dest)
  end

  TreeView.super.update(self)
end


function TreeView:draw()
  self:draw_background(style.background2)

  local icon_width = style.icon_font:get_width("D")
  local spacing = style.font:get_width(" ") * 2

  local doc = core.active_view.doc
  local active_filename = doc and system.absolute_path(doc.filename or "")

  for item, x,y,w,h in self:each_item() do
    local color = style.text

    -- highlight active_view doc
    if item.abs_filename == active_filename then
      color = style.accent
    end

    -- hovered item background
    if item == self.hovered_item then
      renderer.draw_rect(x, y, w, h, style.line_highlight)
      color = style.accent
    end

    -- icons
    x = x + item.depth * style.padding.x + style.padding.x
    if item.type == "dir" then
      local icon1 = item.expanded and "-" or "+"
      local icon2 = item.expanded and "D" or "d"
      common.draw_text(style.icon_font, color, icon1, nil, x, y, 0, h)
      x = x + style.padding.x
      common.draw_text(style.icon_font, color, icon2, nil, x, y, 0, h)
      x = x + icon_width
    else
      x = x + style.padding.x
      common.draw_text(style.icon_font, color, "f", nil, x, y, 0, h)
      x = x + icon_width
    end

    -- text
    x = x + spacing
    x = common.draw_text(style.font, color, item.name, nil, x, y, 0, h)
  end
end


-- init
local view = TreeView()
local node = core.root_view:get_active_node()
node:split("left", view, true)

-- register commands and keymap
command.add(nil, {
  ["treeview:toggle"] = function()
    view.visible = not view.visible
  end,
})

keymap.add { ["ctrl+t"] = "treeview:toggle" } --< @r-lyeh ctrl+// > ctrl+t

-- register some context menu items, if available
local has_menu, menu = core.try(require, "plugins.contextmenu")
local has_fsutils, fsutils = core.try(require, "plugins.fsutils")

if has_menu and has_fsutils then
  local function new_file_f(path)
    command.perform "core:new-doc"
  end

  local function new_file()
    new_file_f(view.hovered_item.abs_filename)
  end

  local function new_dir_f(path)
    core.command_view:enter("New directory name", function(dir)
      fsutils.mkdir(dir)
    end)
    core.command_view:set_text(path .. PATHSEP .. "New folder")
  end

  local function new_dir()
    new_dir_f(view.hovered_item.abs_filename)
  end

  local function delete_f(path)
    core.add_thread(function()
      local function wrap()
        return coroutine.wrap(function() fsutils.delete(path, true) end)
      end

      for n in wrap() do
        if n % 100 == 0 then
          core.log("Deleted %d items.", n)
          coroutine.yield(0)
        end
      end

      core.log("%q deleted.", path)
    end)
  end

  local function delete()
    local path = view.hovered_item.abs_filename
    if view.hovered_item.type == "dir"
      and system.show_confirm_dialog("Delete confirmation", string.format("Do you really want to delete %q ?", path)) then
      delete_f(path)
    else
      delete_f(path)
    end
  end

  local function dirname(path)
    local p = fsutils.split(path)
    table.remove(p)
    return table.concat(p, PATHSEP)
  end

  local function rename()
    local oldname = view.hovered_item.abs_filename
    core.command_view:enter("Rename to", function(newname)
      fsutils.move(oldname, newname)
      core.log("Moved %q to %q", oldname, newname)
    end, common.path_suggest)
    core.command_view:set_text(dirname(oldname))
  end

  local function copy_path()
    system.set_clipboard(view.hovered_item.abs_filename)
  end

  menu:register(function() return view.hovered_item and view.hovered_item.type == "dir" end, {
    { text = "New file", command = new_file },
    { text = "New folder", command = new_dir },
    menu.DIVIDER,
    { text = "Rename", command = rename },
    { text = "Delete", command = delete },
    menu.DIVIDER,
    { text = "Copy directory name", command = copy_path }
  })
  menu:register(function() return view.hovered_item and view.hovered_item.type == "file" end, {
    { text = "Rename", command = rename },
    { text = "Delete", command = delete },
    menu.DIVIDER,
    { text = "Copy filename", command = copy_path }
  })
  -- general region of the treeview
  menu:register(function(x, y)
    local x1, y1, x2, y2 = view:get_content_bounds()
    return not view.hovered_item and x > x1 and x <= x2 and y > y1 and y <= y2
  end, {
    { text = "New file", command = function() new_file_f(system.absolute_path('.')) end },
    { text = "New folder", command = function() new_dir_f(system.absolute_path('.')) end }
  })
end

return view --< @r-lyeh
