require "plugins.reflow"
local config = require "core.config"
local command = require "core.command"
local DocView = require "core.docview"

config.autowrap_files = { "%.md$", "%.txt$" }


local on_text_input = DocView.on_text_input

DocView.on_text_input = function(self, ...)
  on_text_input(self, ...)

  -- early-exit if the filename does not match a file type pattern
  local filename = self.doc.filename or ""
  local matched = false
  for _, ptn in ipairs(config.autowrap_files) do
    if filename:match(ptn) then
      matched = true
      break
    end
  end
  if not matched then return end

  -- do automatic reflow on line if we're typing at the end of the line and have
  -- reached the line limit
  local line, col = self.doc:get_selection()
  local text = self.doc:get_text(line, 1, line, math.huge)
  if #text >= config.line_limit and col > #text then
    command.perform("doc:select-lines")
    command.perform("reflow:reflow")
    command.perform("doc:move-to-next-char")
    command.perform("doc:move-to-previous-char")
  end
end
