-- mod-version:2 -- lite-xl 2.0
local core = require "core"
local command = require "core.command"
local translate = require "core.doc.translate"

local function split_lines(text)
  local res = {}
  for line in (text .. "\n"):gmatch("(.-)\n") do
    table.insert(res, line)
  end
  return res
end

command.add("core.docview", {
  ["sort:sort"] = function()
    local doc = core.active_view.doc

    local l1, c1, l2, c2, swap = doc:get_selection(true)
    l1, c1 = translate.start_of_line(doc, l1, c1)
    l2, c2 = translate.end_of_line(doc, l2, c2)
    doc:set_selection(l1, c1, l2, c2, swap)

    doc:replace(function(text)
      local head, body, foot = text:match("(\n*)(.-)(\n*)$")
      local lines = split_lines(body)
      table.sort(lines, function(a, b) return a:lower() < b:lower() end)
      return head .. table.concat(lines, "\n") .. foot
    end)
  end,
})
