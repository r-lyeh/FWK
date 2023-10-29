local syntax = require "core.syntax"

-- batch syntax for lite <liqube>

-- windows batch files use caseless matching for symbols
local symtable = {
  ["keyword"] = {
  "if", "else", "elsif", "not", "for", "do", "in",
  "equ", "neq", "lss", "leq", "gtr", "geq", -- == != < <= > >=
  "nul", "con", "prn", "prn", "lpt1", "com1", "com2", "com3", "com4",
  "exist", "defined",
  "errorlevel", "cmdextversion",
  "goto", "call", "verify",
  },
  ["function"] = {
  "set", "setlocal", "endlocal", "enabledelayedexpansion",
  "echo", "type",
  "cd", "chdir",
  "md", "mkdir",
  "pause", "choice", "exit",
  "del", "rd", "rmdir",
  "copy", "xcopy",
  "move", "ren",
  "find", "findstr",
  "sort", "shift", "attrib",
  "cmd", "command",
  "forfiles",
 },
}
-- prepare a mixed symbol list
local function prepare_symbols(symtable)
  local symbols = { }
  for symtype, symlist in pairs(symtable) do
    for _, symname in ipairs(symlist) do
      symbols[symname:lower()] = symtype
    symbols[symname:upper()] = symtype
    end
  end
  return symbols
end

syntax.add {
  files = { "%.bat$", "%.cmd$" },
  comment = "rem",
  patterns = {
    { pattern = "@echo off\n",                  type = "keyword"  },
    { pattern = "@echo on\n",                   type = "keyword"  },
    { pattern = "rem.-\n",                      type = "comment"  },    -- rem comment line, rem, rem.
    { pattern = "REM.-\n",                      type = "comment"  },
    { pattern = "%s*:[%w%-]+",                  type = "symbol"   },    -- :labels
    { pattern = "%:%:.-\n",                     type = "comment"  },    -- :: comment line
    { pattern = "%%%w+%%",                      type = "symbol"   },    -- %variable%
    { pattern = "%%%%?~?[%w:]+",                type = "symbol"   },    -- %1, %~dpn1, %~1:2, %%i, %%~i
    { pattern = "[!=()%>&%^/\\@]",              type = "operator" },    -- operators
    { pattern = "-?%.?%d+f?",                   type = "number"   },    -- integer numbers
    { pattern = { '"', '"', '\\' },             type = "string"   },    -- "strings"
    { pattern = "[%a_][%w_]*",                  type = "normal"   },
    { pattern = ":eof",                         type = "keyword"  },    -- not quite as intended, but ok for now
  },
  symbols = prepare_symbols(symtable),
}