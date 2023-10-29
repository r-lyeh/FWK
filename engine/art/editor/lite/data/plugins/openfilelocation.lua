local core = require "core"
local command = require "core.command"
local config = require "core.config"


if PLATFORM == "Windows" then
  config.filemanager = "explorer"
elseif PLATFORM == "Mac OS X" then
  config.filemanager = "open"
else
  config.filemanager = "xdg-open"
end


command.add("core.docview", {
  ["open-file-location:open-file-location"] = function()
    local doc = core.active_view.doc
    if not doc.filename then
      core.error "Cannot open location of unsaved doc"
      return
    end
    local folder = doc.filename:match("^(.*)[/\\].*$") or "."
    core.log("Opening \"%s\"", folder)
    if PLATFORM == "Windows" then
      system.exec(string.format("%s %s", config.filemanager, folder))
    else
      system.exec(string.format("%s %q", config.filemanager, folder))
    end
  end
})