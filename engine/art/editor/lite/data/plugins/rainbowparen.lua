local tokenizer = require "core.tokenizer"
local style = require "core.style"
local common = require "core.common"

local tokenize = tokenizer.tokenize
local closers = {
  ["("] = ")",
  ["["] = "]",
  ["{"] = "}"
}
local function parenstyle(parenstack)
  return "paren" .. ((#parenstack % 5) + 1)
end
function tokenizer.tokenize(syntax, text, state)
  state = state or {}
  local res, istate = tokenize(syntax, text, state.istate)
  local parenstack = state.parenstack or ""
  local newres = {}
  -- split parens out
  -- the stock tokenizer can't do this because it merges identical adjacent tokens
  for i, type, text in tokenizer.each_token(res) do
    if type == "normal" or type == "symbol" then
      for normtext1, paren, normtext2 in text:gmatch("([^%(%[{}%]%)]*)([%(%[{}%]%)]?)([^%(%[{}%]%)]*)") do
        if #normtext1 > 0 then
          table.insert(newres, type)
          table.insert(newres, normtext1)
        end
        if #paren > 0 then
          if paren == parenstack:sub(-1) then -- expected closer
            parenstack = parenstack:sub(1, -2)
            table.insert(newres, parenstyle(parenstack))
          elseif closers[paren] then -- opener
            table.insert(newres, parenstyle(parenstack))
            parenstack = parenstack .. closers[paren]
          else -- unexpected closer
            table.insert(newres, "paren_unbalanced")
          end
          table.insert(newres, paren)
        end
        if #normtext2 > 0 then
          table.insert(newres, type)
          table.insert(newres, normtext2)
        end
      end
    else
      table.insert(newres, type)
      table.insert(newres, text)
    end
  end
  return newres, { parenstack = parenstack, istate = istate }
end

style.syntax.paren_unbalanced = style.syntax.paren_unbalanced or { common.color "#DC0408" }
style.syntax.paren1  =  style.syntax.paren1 or { common.color "#FC6F71"}
style.syntax.paren2  =  style.syntax.paren2 or { common.color "#fcb053"}
style.syntax.paren3  =  style.syntax.paren3 or { common.color "#fcd476"}
style.syntax.paren4  =  style.syntax.paren4 or { common.color "#52dab2"}
style.syntax.paren5  =  style.syntax.paren5 or { common.color "#5a98cf"}
