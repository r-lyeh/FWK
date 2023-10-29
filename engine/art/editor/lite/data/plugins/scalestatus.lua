--[[
    scalestatus.lua
    displays current scale (zoom) in status view
    version: 20200628_155804
    originally by SwissalpS

    Depends on plugin scale.lua version >= 20200628_154010
--]]
local scale = require "plugins.scale"
-- make sure plugin is installed and has get_scale field
if not scale.get_scale then
    local core = require "core"
    core.error("Plugin 'scale' needs to be updated, scalestatus inactive.")
    return false
end

local config = require "core.config"
local StatusView = require "core.statusview"

config.scalestatus_format = '%.0f%%'

local get_items = StatusView.get_items
function StatusView:get_items()

  local left, right = get_items(self)

  local t = {
    self.separator,
    string.format(config.scalestatus_format, scale.get_scale() * 100),
  }

  for _, item in ipairs(t) do
    table.insert(right, item)
  end

  return left, right

end

return true
