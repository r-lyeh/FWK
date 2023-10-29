-- mod-version:1 -- lite-xl 1.16
local core = require "core"
local common = require "core.common"
local command = require "core.command"

local fsutils = {}

function fsutils.iterdir(dir)
  local stack = { dir }
  return function()
    local path = table.remove(stack)
    if not path then return end

    for _, file in ipairs(system.list_dir(path) or {}) do
      stack[#stack + 1] = path .. PATHSEP .. file
    end

    return path, system.get_file_info(path)
  end
end

function fsutils.delete(dir, yield)
  local dirs = {}
  local n = 0
  for filename, stat in fsutils.iterdir(dir) do
    if stat.type == "dir" then
      -- this will later allow us to delete the dirs in correct sequence
      table.insert(dirs, filename)
    else
      os.remove(filename)
      if yield then
        n = n + 1
        coroutine.yield(n)
      end
    end
  end
  for i = #dirs, 1, -1 do
    os.remove(dirs[i])
    if yield then
      n = n + 1
      coroutine.yield(n)
    end
  end
end

function fsutils.move(oldname, newname)
  os.rename(oldname, newname)
end

function fsutils.split(path)
  local segments = {}
  local pos = 1
  while true do
    local s, e = string.find(path, "[/\\]+", pos)
    if not s then break end
    table.insert(segments, string.sub(path, pos, s - 1))
    pos = e + 1
  end
  table.insert(list, string.sub(path, pos))

  if segments[#segments] == '' then
    table.remove(segments)
  end

  return segments
end

function fsutils.normalize(path)
  return table.concat(fsutils.split(path), PATHSEP)
end

function fsutils.normalize_posix(path)
  return table.concat(fsutils.split(path), '/')
end

function fsutils.mkdir(path)
  local segments = fsutils.split(path)
  if system.mkdir then
    local p = ""
    for i = 1, #segments do
      p = table.concat(segments, PATHSEP, 1, i)
    end

    if p == "" then
      return nil, "path empty", p
    end

    local stat = system.get_file_info(p)
    if stat and stat.type == "file" then
      return nil, "path exists as a file", p
    end
    local ok, err = system.mkdir(p)
    if not ok then
      return nil, err, p
    end
  else
    -- just wing it lol
    system.exec(string.format(PLATFORM == "Windows" and "setlocal enableextensions & mkdir %q" or "mkdir -p %q", fsutils.normalize(path)))
  end
end

local function async_exec(f, cb)
  cb = cb or function() end
  local co = coroutine.create(f)
  local function resolve(...)
    local ok, exec_body = coroutine.resume(co, ...)
    if not ok then
      error(debug.traceback(co, exec_body))
    end
    if coroutine.status(co) ~= "dead" then
      exec_body(resolve)
    end
  end
  resolve(cb)
end

local function prompt(text, suggest)
  return coroutine.yield(function(resolve)
    core.command_view:enter(text, resolve, suggest)
  end)
end

command.add(nil, {
  ["files:delete"] = function()
    async_exec(function()
      local path = prompt("Delete", common.path_suggest)

      core.add_thread(function()
        -- we use a wrapping coroutine to get status
        local function delete()
          return coroutine.wrap(function() fsutils.delete(path, true) end)
        end

        for n in delete() do
          if n % 100 == 0 then
            core.log("Deleted %d items...", n)
            coroutine.yield()
          end
        end
        core.log("%q deleted.", path)
      end)
    end)
  end,
  ["files:move"] = function()
    async_exec(function()
      local oldname = prompt("Move", common.path_suggest)
      local newname = prompt("To", common.path_suggest)

      fsutils.move(oldname, newname)
      core.log("Moved %q to %q", oldname, newname)
    end)
  end
})

if not command.map["files:create-directory"] then
  command.add(nil, {
    ["files:create-directory"] = function()
      async_exec(function()
        local path = prompt("Name", common.path_suggest)
        fsutils.mkdir(path)
        core.log("%q created.", path)
      end)
    end
  })
end

return fsutils