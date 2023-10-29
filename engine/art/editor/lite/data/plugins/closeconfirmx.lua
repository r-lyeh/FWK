-- CloseConfirmX plugin for lite text editor
-- implementation by chekoopa

local core = require "core"
local config = require "core.config"

config.closeconfirmx_use_legacy = false
config.closeconfirmx_use_short_name = true

local legacy_confirm = core.confirm_close_all

local function commandful_confirm()
  local dirty_count = 0
  local dirty_name
  for _, doc in ipairs(core.docs) do
    if doc:is_dirty() then
      dirty_count = dirty_count + 1
      dirty_name = doc:get_name()
    end
  end
  if dirty_count > 0 then
    local text
    if dirty_count == 1 then
      if config.closeconfirmx_use_short_name then
         dirty_name = dirty_name:match("[^/%\\]*$")
      end
      text = string.format("Unsaved changes in \"%s\"; Confirm Exit", dirty_name)
    else
      text = string.format("Unsaved changes in %d docs; Confirm Exit", dirty_count)
    end
    core.command_view:enter(text, function(_, item)
      if item.text:match("^[cC]") then
        core.quit(true)
      end
    end, function(text)
      local items = {}
      if not text:find("^[^sS]") then table.insert(items, "Stay here") end
      if not text:find("^[^cC]") then table.insert(items, "Close Without Saving") end
      return items
    end)
    -- as we delegate a choice inside the callback,
    return false
  end
  return true
end

function core.confirm_close_all()
  if config.closeconfirmx_use_legacy then
    return legacy_confirm()
  else
    return commandful_confirm()
  end
end
