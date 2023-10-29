local core = require "core"
local command = require "core.command"
local config = require "core.config"
local search = require "core.doc.search"
local DocView = require "core.docview"

local max_previous_finds = 50


local function doc()
  return core.active_view.doc
end


local previous_finds
local last_doc
local last_fn, last_text


local function push_previous_find(doc, sel)
  if last_doc ~= doc then
    last_doc = doc
    previous_finds = {}
  end
  if #previous_finds >= max_previous_finds then
    table.remove(previous_finds, 1)
  end
  table.insert(previous_finds, sel or { doc:get_selection() })
end


local function find(label, search_fn)
  local dv = core.active_view
  local sel = { dv.doc:get_selection() }
  local text = dv.doc:get_text(table.unpack(sel))
  local found = false

  core.command_view:set_text(text, true)

  core.command_view:enter(label, function(text)
    if found then
      last_fn, last_text = search_fn, text
      previous_finds = {}
      push_previous_find(dv.doc, sel)
    else
      core.error("Couldn't find %q", text)
      dv.doc:set_selection(table.unpack(sel))
      dv:scroll_to_make_visible(sel[1], sel[2])
    end

  end, function(text)
    local ok, line1, col1, line2, col2 = pcall(search_fn, dv.doc, sel[1], sel[2], text)
    if ok and line1 and text ~= "" then
      dv.doc:set_selection(line2, col2, line1, col1)
      dv:scroll_to_line(line2, true)
      found = true
    else
      dv.doc:set_selection(table.unpack(sel))
      found = false
    end

  end, function(explicit)
    if explicit then
      dv.doc:set_selection(table.unpack(sel))
      dv:scroll_to_make_visible(sel[1], sel[2])
    end
  end)
end


local function replace(kind, default, fn)
  core.command_view:set_text(default, true)

  core.command_view:enter("Find To Replace " .. kind, function(old)
    core.command_view:set_text(old, true)

    local s = string.format("Replace %s %q With", kind, old)
    core.command_view:enter(s, function(new)
      local n = doc():replace(function(text)
        return fn(text, old, new)
      end)
      core.log("Replaced %d instance(s) of %s %q with %q", n, kind, old, new)
    end)
  end)
end


local function has_selection()
  return core.active_view:is(DocView)
     and core.active_view.doc:has_selection()
end

local function has_unique_selection()
  if not doc() then return false end
  local text = nil
  for idx, line1, col1, line2, col2 in doc():get_selections(true, true) do
    if line1 == line2 and col1 == col2 then return false end
    local selection = doc():get_text(line1, col1, line2, col2)
    if text ~= nil and text ~= selection then return false end
    text = selection
  end
  return text ~= nil
end

local function is_in_selection(line, col, l1, c1, l2, c2)
  if line < l1 or line > l2 then return false end
  if line == l1 and col <= c1 then return false end
  if line == l2 and col > c2 then return false end
  return true
end

local function is_in_any_selection(line, col)
  for idx, l1, c1, l2, c2 in doc():get_selections(true, false) do
    if is_in_selection(line, col, l1, c1, l2, c2) then return true end
  end
  return false
end

local function select_add_next(all)
  local il1, ic1 = doc():get_selection(true)
  for idx, l1, c1, l2, c2 in doc():get_selections(true, true) do
    local text = doc():get_text(l1, c1, l2, c2)
    repeat
      l1, c1, l2, c2 = search.find(doc(), l2, c2, text, { wrap = true })
      if l1 == il1 and c1 == ic1 then break end
      if l2 and (all or not is_in_any_selection(l2, c2)) then
        doc():add_selection(l2, c2, l1, c1)
        if not all then
          core.active_view:scroll_to_make_visible(l2, c2)
          return
        end
      end
    until not all or not l2
    if all then break end
  end
end

local function select_next(reverse)
  local l1, c1, l2, c2 = doc():get_selection(true)
  local text = doc():get_text(l1, c1, l2, c2)
  if reverse then
    l1, c1, l2, c2 = search.find(doc(), l1, c1, text, { wrap = true, reverse = true })
  else
    l1, c1, l2, c2 = search.find(doc(), l2, c2, text, { wrap = true })
  end
  if l2 then doc():set_selection(l2, c2, l1, c1) end
end

---@param in_selection? boolean whether to replace in the selections only, or in the whole file.
local function find_replace(in_selection)
  local l1, c1, l2, c2 = doc():get_selection()
  local selected_text = ""
  if not in_selection then
    selected_text = doc():get_text(l1, c1, l2, c2)
    doc():set_selection(l2, c2, l2, c2)
  end
  replace("Text", l1 == l2 and selected_text or "", function(text, old, new)
    if not find_regex then
      return text:gsub(old:gsub("%W", "%%%1"), new:gsub("%%", "%%%%"), nil)
    end
    local result, matches = regex.gsub(regex.compile(old, "m"), text, new)
    return result, matches
  end)
end

command.add(has_unique_selection, {
  ["find-replace:select-next"] = select_next,
  ["find-replace:select-previous"] = function() select_next(true) end,
  ["find-replace:select-add-next"] = select_add_next,
  ["find-replace:select-add-all"] = function() select_add_next(true) end
})

command.add("core.docview", {
  ["find-replace:find"] = function()
    find("Find Text", function(doc, line, col, text)
      local opt = { wrap = true, no_case = true }
      return search.find(doc, line, col, text, opt)
    end)
  end,

  ["find-replace:find-pattern"] = function()
    find("Find Text Pattern", function(doc, line, col, text)
      local opt = { wrap = true, no_case = true, pattern = true }
      return search.find(doc, line, col, text, opt)
    end)
  end,

  ["find-replace:repeat-find"] = function()
    if not last_fn then
      core.error("No find to continue from")
    else
      local line, col = doc():get_selection()
      local line1, col1, line2, col2 = last_fn(doc(), line, col, last_text)
      if line1 then
        push_previous_find(doc())
        doc():set_selection(line2, col2, line1, col1)
        core.active_view:scroll_to_line(line2, true)
      end
    end
  end,

  ["find-replace:previous-find"] = function()
    local sel = table.remove(previous_finds)
    if not sel or doc() ~= last_doc then
      core.error("No previous finds")
      return
    end
    doc():set_selection(table.unpack(sel))
    core.active_view:scroll_to_line(sel[3], true)
  end,

  ["find-replace:replace"] = function()
    replace("Text", "", function(text, old, new)
      return text:gsub(old:gsub("%W", "%%%1"), new:gsub("%%", "%%%%"), nil)
    end)
  end,

  ["find-replace:replace-pattern"] = function()
    replace("Pattern", "", function(text, old, new)
      return text:gsub(old, new)
    end)
  end,

  ["find-replace:replace-symbol"] = function()
    local first = ""
    if doc():has_selection() then
      local text = doc():get_text(doc():get_selection())
      first = text:match(config.symbol_pattern) or ""
    end
    replace("Symbol", first, function(text, old, new)
      local n = 0
      local res = text:gsub(config.symbol_pattern, function(sym)
        if old == sym then
          n = n + 1
          return new
        end
      end)
      return res, n
    end)
  end,
})
