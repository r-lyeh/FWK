-- Copyright (c) Facebook, Inc. and its affiliates.
-- This source code is licensed under the MIT license found in the
-- LICENSE file in the root directory of this source tree.
--
-- Lua module to preprocess and parse C declarations.
-- (Leon Bottou, 2015)

-- @r-lyeh: del lcdecl module
-- @r-lyeh: add cppString method
-- @r-lyeh: add __COUNTER__ macro
-- @r-lyeh: del empty #line directives from output

-- standard libs
local string = require 'string'
local coroutine = require 'coroutine'
local table = require 'table'
local io = require 'io'

-- Lua 5.1 to 5.3 compatibility
local unpack = unpack or table.unpack

-- Debugging
local DEBUG = true
if DEBUG then pcall(require,'strict') end

-- luacheck: globals cparser
-- luacheck: ignore 43 4/ti 4/li
-- luacheck: ignore 212/.*_
-- luacheck: ignore 211/is[A-Z].* 211/Type
-- luacheck: ignore 542


---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- ALL UGLY HACKS SHOULD BE HERE


-- Sometimes we cannot find system include files but need to know at
-- least things about them. For instance, certain system include files
-- define alternate forms for keywords.

local knownIncludeQuirks = {}

knownIncludeQuirks["<complex.h>"] = { -- c99
   "#ifndef complex", "# define complex _Complex", "#endif"
}

knownIncludeQuirks["<stdbool.h>"] = { -- c99
   "#ifndef bool", "# define bool _Bool", "#endif"
}

knownIncludeQuirks["<stdalign.h>"] = { -- c11
   "#ifndef alignof", "# define alignof _Alignof", "#endif",
   "#ifndef alignas", "# define alignas _Alignas", "#endif"
}

knownIncludeQuirks["<stdnoreturn.h>"] = { -- c11
   "#ifndef noreturn", "# define noreturn _Noreturn", "#endif"
}

knownIncludeQuirks["<threads.h>"] = { -- c11
   "#ifndef thread_local", "# define thread_local _Thread_local", "#endif"
}

knownIncludeQuirks["<iso646.h>"] = { -- c++
   "#define and &&", "#define and_eq &=", "#define bitand &", "#define bitor |",
   "#define compl ~", "#define not !", "#define not_eq !=", "#define or ||",
   "#define or_eq |=", "#define xor ^", "#define xor_eq ^="
}


---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- TAGGED TABLES


-- Utilities to produce and print tagged tables.
-- The tag name is simply the contents of table key <tag>.
-- Function <newTag> returns a node constructor
--
-- Example:
--
-- > Foo = newTag('Foo')
-- > Bar = newTag('Bar')
--
-- > print( Foo{const=true,next=Bar{name="Hello"}} )
-- Foo{next=Bar{name="Hello"},const=true}
--
-- > print( Bar{name="hi!", Foo{1}, Foo{2}, Foo{3}} )
-- Bar{Foo{1},Foo{2},Foo{3},name="hi!"}

local function newTag(tag)
   -- the printing function
   local function tostr(self)
      local function str(x)
         if type(x)=='string' then
            return string.format("%q",x):gsub("\\\n","\\n")
         elseif type(x)=='table' and not getmetatable(x) then
            return "{..}"
         else
            return tostring(x)
         end
      end
      local p = string.format("%s{", self.tag or "Node")
      local s = {}
      local seqlen = 0
      for i=1,#self do
         if self[i] then seqlen=i else break end end
      for i=1,seqlen do
         s[1+#s] = str(self[i]) end
      for k,v in pairs(self) do
         if type(k) == 'number' then
            if k<1 or k>seqlen then
               s[1+#s] = string.format("[%s]=%s",k,str(v)) end
         elseif type(k) ~= 'string' then
            s.extra = true
         elseif k:find("^_") and type(v)=='table' then
            s[1+#s] = string.format("%s={..}",k) -- hidden
         elseif k ~= 'tag' then
            s[1+#s] = string.format("%s=%s",k,str(v)) end
      end
      if s.extra then s[1+#s] = "..." end
      return p .. table.concat(s,',') .. '}'
   end
   -- the constructor
   return function(t) -- must be followed by a table constructor
      t = t or {}
      assert(type(t)=='table')
      setmetatable(t, { __tostring=tostr } )
      t.tag = tag
      return t
   end
end

-- hack to print any table: print(Node(nn))
local Node = newTag(nil)   -- luacheck: ignore 211

---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- UTILITIES


-- Many functions below have an optional argument 'options' which is
-- simply an array of compiler-like options that are specified in the
-- toplevel call and passed to nearly all functions. Because it
-- provides a good communication channel across the code components,
-- many named fields are also used for multiple purposes. The
-- following function is called at the beginning of the user facing
-- functions to make a copy of the user provided option array and
-- setup some of these fields.

local function copyOptions(options)
   options = options or {}
   assert(type(options)=='table')
   local noptions = {}
   -- copy options
   for k,v in ipairs(options) do noptions[k]=v end
   -- copy user modifiable named fields
   noptions.sizeof = options.sizeof     -- not used yet
   noptions.alignof = options.alignof   -- not used yet
   -- create reversed hash
   noptions.hash = {}
   for i,v in ipairs(options) do
      noptions.hash[v] = i
   end
   -- compute dialect flags
   local dialect = 'gnu99'
   for _,v in ipairs(options) do
      if v:find("^%-std=%s*[^%s]") then
         dialect = v:match("^%-std=%s*(.-)%s*$")
      end
   end
   noptions.dialect = dialect
   noptions.dialectGnu = dialect:find("^gnu")
   noptions.dialect99 = dialect:find("9[9x]$")
   noptions.dialect11 = dialect:find("1[1x]$")
   noptions.dialectAnsi = not noptions.dialectGnu
   noptions.dialectAnsi = noptions.dialectAnsi and not noptions.dialect99
   noptions.dialectAnsi = noptions.dialectAnsi and not noptions.dialect11
   -- return
   return noptions
end


-- This function tests whether a particular option has been given.

local function hasOption(options, opt)
   assert(options)
   assert(options.silent or options.hash)
   return options.hash and options.hash[opt]
end


-- Generic functions for error messages

local function xmessage(err, options, lineno, message, ...)
   local msg = string.format("cparser: (%s) ",lineno)
   msg = msg .. string.format(message,...)
   if options.silent then
      if err == 'error' then error(msg, 0) end
   else
      if err == 'warning' and hasOption(options, "-Werror") then err = 'error' end
      if err == 'error' or not hasOption(options, "-w") then print(msg) end
      if err == 'error' then error("cparser: aborted",0) end
   end
end

local function xwarning(options, lineno, message, ...)
   xmessage('warning', options, lineno, message, ...)
end

local function xerror(options, lineno, message, ...)
   xmessage('error', options, lineno, message, ...)
end

local function xassert(cond, ...)
   if not cond then xerror(...) end
end

local function xdebug(lineno,message,...)
   local msg = string.format("\t\t[%s] ", lineno)
   msg = msg .. string.format(message,...)
   print(msg)
end


-- Nil-safe max

local function max(a,b)
   a = a or b
   b = b or a
   return a > b and a or b
end


-- Deep table comparison
-- (not very efficient, no loop detection)

local function tableCompare(a,b)
   if a == b then
      return true
   elseif type(a) == 'table' and type(b) == 'table' then
      for k,v in pairs(a) do
         if not tableCompare(v,b[k]) then return false end
      end
      for k,v in pairs(b) do
         if not tableCompare(a[k],v) then return false end
      end
      return true
   else
      return false
   end
end


-- Concatenate two possibly null arrays

local function tableAppend(a1, a2)
   if not a1 then
      return a2
   elseif not a2 then
      return a1
   else
      local a = {}
      for _,v in ipairs(a1) do a[1+#a] = v end
      for _,v in ipairs(a2) do a[1+#a] = v end
      return a
   end
end

-- Concatenate strings from table (skipping non-string content.)

local function tableConcat(a)
   local b = {}
   for _,v in ipairs(a) do
      if type(v) == 'string' then b[1+#b]=v end end
   return table.concat(b)
end


-- Evaluate a lua expression, return nil on error.

local function evalLuaExpression(s)
   assert(type(s)=='string')
   local f = load(string.gmatch(s,".*"))
   local function r(status,...)
      if status then return ... end end
   return r(pcall(f or error))
end


-- Bitwise manipulations
-- try lua53 operators otherwise revert to iterative version

local bit = evalLuaExpression([[
   local bit = {}
   function bit.bnot(a) return ~a end
   function bit.bor(a,b) return a | b end
   function bit.band(a,b) return a & b end
   function bit.bxor(a,b) return a ~ b end
   function bit.lshift(a,b) return a < 0 and b < 0 and ~((~a) << b) or a << b end
   return bit
]])

if not bit then
   local function bor(a,b)
      local r, c, d = 0, 1, -1
      while a > 0 or b > 0 or a < -1 or b < -1 do
         if a % 2 > 0 or b % 2 > 0 then r = r + c end
         a, b, c, d = math.floor(a / 2), math.floor(b / 2), c * 2, d * 2 end
      if a < 0 or b < 0 then r = r + d end
      return r end
   bit = {}
   function bit.bnot(a) return -1-a end
   function bit.bor(a,b) return bor(a,b) end
   function bit.band(a,b) return -1-bor(-1-a,-1-b) end
   function bit.bxor(a,b) return bor(-1-bor(a,-1-b),-1-bor(-1-a,b)) end
   function bit.lshift(a,b) return math.floor(a * 2 ^ b) end
end


-- Coroutine helpers.
-- This code uses many coroutines that yield lines or tokens.
-- All functions that can yield take an options table as first argument.


-- Wrap a coroutine f into an iterator
-- The options and all the extra arguments are passed
-- to the coroutine when it starts. Together with the
-- above calling convention, this lets us specify
-- coroutine pipelines (see example in function "cpp".)

local function wrap(options, f, ...)
   local function g(...) coroutine.yield(nil) f(...) end
   local c = coroutine.create(g)
   coroutine.resume(c, options, ...)
   local function r(s,...)
      if not s then local m = ... ; error(m, 0) end
      return ...
   end
   return function()
      if coroutine.status(c) ~= 'dead' then
         return r(coroutine.resume(c))
      end
   end
end


-- Collect coroutine outputs into an array
-- The options and the extra arguments are passed to the coroutine.

local function callAndCollect(options, f, ...) -- Bell Labs nostalgia
   local collect = {}
   for s in wrap(options, f, ...) do
      collect[1+#collect] = s
   end
   return collect
end


-- Yields all outputs from iterator iter.
-- Argument options is ignored.

local function yieldFromIterator(options_, iter)
   local function yes(v,...) coroutine.yield(v,...) return v end
   while yes(iter()) do end
end


-- Yields all values from array <arr>.
-- This function successively yields all values in the table.
-- Every yield is augmented with all extra arguments passed to the function.
-- Argument options is ignored.

local function yieldFromArray(options_, arr, ...)
   for _,v in ipairs(arr) do
      coroutine.yield(v, ...)
   end
end





---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- INITIAL PREPROCESSING



-- A routine that pulls lines from a line iterator
-- and yields them together with a location
-- composed of the optional prefix, a colon, and a line number.
-- Argument options is ignored.
-- Lua provides good line iterators such as:
--   io.lines(filename) filedesc:lines()  str:gmatch("[^\n]+")

local function yieldLines(options_,lineIterator,prefix)
   prefix = prefix or ""
   assert(type(prefix)=='string')
   local n = 0
   for s in lineIterator do
      n = n + 1
      coroutine.yield(s, string.format("%s:%d", prefix, n))
   end
end


-- A routine that obtains lines from coroutine <lines>,
-- joins lines terminated by a backslash, and yield the
-- resulting lines. The coroutine is initialized with
-- argument <options> and all extra arguments.
-- Reference: https://gcc.gnu.org/onlinedocs/cpp/Initial-processing.html (3)

local function joinLines(options, lines, ...)
   local li = wrap(options, lines, ...)
   for s, n in li do
      while type(s) == 'string' and s:find("\\%s*$") do
         local t = li() or ""
         s = s:gsub("\\%s*$", "") .. t
      end
      coroutine.yield(s, n)
   end
end


-- A routine that obtain lines from coroutine <lines>, eliminate the
-- comments and yields the resulting lines.  The coroutine is
-- initialized with argument <options> and all extra arguments.
-- Reference: https://gcc.gnu.org/onlinedocs/cpp/Initial-processing.html (4)

local function eliminateComments(options, lines, ...)
   local lineIterator = wrap(options, lines, ...)
   local s,n = lineIterator()
   while type(s) == 'string' do
      local inString = false
      local q = s:find("[\'\"\\/]", 1)
      while q ~= nil do
         if hasOption(options,"-d:comments") then
            xdebug(n, "comment: [%s][%s] %s",s:sub(1,q-1),s:sub(q),inString)
         end
         local c = s:byte(q)
         if inString then
            if c == 92 then -- \
               q = q + 1
            elseif c == inString then
               inString = false
            end
         else
            if c == 34 or c == 39 then -- " or '
               inString = c
            elseif c == 47 and s:byte(q+1) == 47 then -- "//"
               s = s:sub(1,q-1)
            elseif c == 47 and s:byte(q+1) == 42 then -- "/*"
               local p = s:find("%*/",q+2)
               if p ~= nil then
                  s = s:sub(1,q-1) .. " " .. s:sub(p+2)
               else
                  s = s:sub(1,q-1)
                  local ss,pp
                  repeat
                     ss = lineIterator()
                     xassert(ss ~= nil, options, n, "Unterminated comment")
                     pp = ss:find("%*/")
                  until pp
                  s = s .. " " .. ss:sub(pp+2)
               end
            end
         end
         q = s:find("[\'\"\\/]", q+1)
      end
      coroutine.yield(s, n)
      s, n = lineIterator()
   end
end



---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- TOKENIZER



local keywordTable = {
   ------ Standard keywords
   "auto", "break", "case", "char", "const", "continue", "default", "do",
   "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
   "long", "register", "return", "short", "signed", "sizeof", "static", "struct",
   "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
   ------ Nonstandard or dialect specific keywords do not belong here
   ------ because the main function of this table is to say which
   ------ identifiers cannot be variable names.
}

local punctuatorTable = {
   "+", "-", "*", "/", "%", "&", "|", "^", ">>", "<<", "~",
   "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", ">>=", "<<=",
   "(", ")", "[", "]", "{", "}", "++", "--",
   "==", "!=", ">=", "<=", ">", "<", "&&", "||", "!",
   ".", "->", "*", "&", "?", ":", "::", "->*", ".*", ";", ",",
   "#", "##", "...", "@", "\\" -- preprocessor stuff
}

local keywordHash = {}
for _,v in ipairs(keywordTable) do
   keywordHash[v] = true
end

local punctuatorHash = {}
for _,v in ipairs(punctuatorTable) do
   local l = v:len()
   local b = v:byte()
   punctuatorHash[v] = true
   punctuatorHash[b] = max(l,punctuatorHash[b])
end


-- The following functions test the types of the tokens returned by the tokenizer.
-- They should not be applied to arbitrary strings.

local function isSpace(tok)
   return type(tok) == 'string' and tok:find("^%s") ~= nil end
local function isNewline(tok) -- Subtype of space
   return type(tok) == 'string' and tok:find("^\n") ~= nil end
local function isNumber(tok)
   return type(tok) == 'string' and tok:find("^[.0-9]") ~= nil end
local function isString(tok)
   if type(tok) ~= 'string' then return false end
   return tok:find("^[\'\"]") ~= nil end
local function isHeaderName(tok)
   if type(tok) ~= 'string' then return false end
   return tok:find("^\"") or tok:find("^<") and tok:find(">$") end
local function isPunctuator(tok)
   return type(tok) == 'string' and punctuatorHash[tok] ~= nil end
local function isIdentifier(tok)
   return type(tok) == 'string' and tok:find("^[A-Za-z_$]") ~= nil end
local function isKeyword(tok) -- Subtype of identifier
   return keywordHash[tok] ~= nil end
local function isName(tok) -- Subtype of identifier
   return isIdentifier(tok) and not keywordHash[tok] end

-- Magic tokens are used to mark macro expansion boundaries (see expandMacros.)
local function isMagic(tok)
   return tok and type(tok) ~= 'string' end
local function isBlank(tok) -- Treats magic token as space.
   return isMagic(tok) or isSpace(tok) end

-- The tokenizeLine() function takes a line, splits it into tokens,
-- and yields tokens and locations. The number tokens are the weird
-- preprocessor numbers defined by ansi c. The string tokens include
-- character constants and angle-bracket delimited strings occuring
-- after an include directive. Every line begins with a newline
-- token giving the proper indentation. All subsequent spaces
-- are reduced to a single space character.

local function tokenizeLine(options, s, n, notNewline)
   -- little optimization for multiline macros
   -- s may be an array of precomputed tokens
   if type(s) == 'table' then
      return yieldFromArray(options, s, n)
   end
   -- normal operation
   assert(type(s) == 'string')
   local p = s:find("[^%s]")
   -- produce a newline token
   if p and not notNewline then
      local r = '\n' .. s:sub(1,p-1)
      coroutine.yield(r, n)
   end
   -- produce one token
   local function token()
      local b, l, r
      if hasOption(options, "-d:tokenize") then
         xdebug(n, "[%s][%s]",s:sub(1,p-1),s:sub(p))
      end
      -- space
      l = s:find("[^%s]", p)
      if l == nil then
         return nil
      elseif l > p then
         p = l
         return " ", n
      end
      -- identifier
      r = s:match("^[a-zA-Z_$][a-zA-Z0-9_$]*", p)
      if r ~= nil then
         p = p + r:len()
         return r, n
      end
      -- preprocessor numbers
      r = s:match("^%.?[0-9][0-9a-zA-Z._]*", p)
      if r ~= nil then
         l = r:len()
         while r:find("[eEpP]$") and s:find("^[-+]", p+l) do
            r = r .. s:match("^[-+][0-9a-zA-Z._]*", p+l)
            l = r:len()
         end
         p = p + l
         return r, n
      end
      -- angle-delimited strings in include directives
      b = s:byte(p)
      if b == 60 and s:find("^%s*#%s*include") then
         r = s:match("^<[^>]+>", p)
         if r ~= nil then
            p = p + r:len()
            return r, n
         end
      end
      -- punctuator
      l = punctuatorHash[b]
      if l ~= nil then
         while l > 0 do
            r = s:sub(p,p+l-1)
            if punctuatorHash[r] then
               p = p + l
               return r, n
            end
            l = l - 1
         end
      end
      -- string
      if b == 34 or b == 39 then -- quotes
         local q = p
         repeat
            q = s:find("[\'\"\\]", q + 1)
            l = s:byte(q)
            xassert(q ~= nil, options, n, "Unterminated string or character constant")
            if l == 92 then
               q = q + 1
            end
         until l == b
         r = s:sub(p,q)
         p = q + 1
         return r, n
      end
      -- other stuff (we prefer to signal an error here)
      xerror(options, n,"Unrecognized character (%s)", s:sub(p))
   end
   -- loop
   if p then
      for tok,tokn in token do
         coroutine.yield(tok, tokn)
      end
   end
end


-- Obtain lines from coroutine <lines>,
-- and yields their tokens. The coroutine is initialized with
-- argument <options> and all extra arguments.

local function tokenize(options, lines, ...)
   for s,n in wrap(options, lines, ...) do
      tokenizeLine(options, s, n)
   end
end


---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- PREPROCESSING


-- Preprocessing is performed by two coroutines. The first one
-- processes all the preprocessor directives and yields the remaining
-- lines. The second one processes tokens from the remaining lines and
-- perform macro expansions. Both take a table of macro definitions as
-- argument. The first one writes into the table and the second one
-- reads from it.
--
-- Each macro definition is an array of tokens (for a single line
-- macro) or a table whose entry <"lines"> contains an array of arrays
-- of tokens (#defmacro). If the macro takes arguments, the entry
-- <"args"> contains a list of argument names. If the macro is
-- recursive (#defrecmacro), the entry <recursive> is set.
-- Alternatively, the macro definition may be a function called at
-- macro-expansion time. This provides for complicated situations.


-- forward declarations
local function expandMacros() end
local function processDirectives() end


-- Starting with the second coroutine which takes a token producing
-- coroutine and yields the preprocessed tokens. Argument macros is
-- the macro definition table.

-- The standard mandates that the result of a macro-expansion must be
-- scanned for further macro invocations whose argunent list possibly
-- consume tokens that follow the macro-expansion. This means that one
-- cannot recursively call expandMacros but one must prepend the
-- macro-expansion in front of the remaining tokens. The standard also
-- mandates that the result of any macro-expansion must be marked to
-- prevent recursive invocation of the macro that generated it,
-- whether when expanding macro arguments or expanding the macro
-- itself.  We achieve this by bracketing every macro-expansion with
-- magic tokens that track which macro definitions must be disabled.
-- These magic tokens are removed later in the coroutines
-- <filterSpaces> or <preprocessedLines>.

expandMacros = function(options, macros, tokens, ...)
   -- basic iterator
   local ti = wrap(options, tokens, ...)
   -- prepending tokens in front of the token stream
   local prepend = {}
   local function prependToken(s,n)
      table.insert(prepend,{s,n}) end
   local function prependTokens(pti)
      local pos = 1+#prepend
      for s,n in pti do table.insert(prepend,pos,{s,n}) end end
   local ti = function()
      if #prepend > 0 then return unpack(table.remove(prepend))
      else return ti() end end
   -- iterator that handles magic tokens to update macro definition table
   local ti = function()
      local s,n = ti()
      while type(s) == 'table' do
         if s.tag == 'push' then
            local nmacros = {}
            setmetatable(nmacros, {__index=macros})
            if s.symb then nmacros[s.symb] = false end
            macros = nmacros
         elseif s.tag == 'pop' then
            local mt = getmetatable(macros)
            if mt and mt.__index then macros = mt.__index end
         end
         coroutine.yield(s,n)
         s,n = ti()
      end
      return s,n
   end
   -- redefine ti() to ensure tok,n remain up-to-date
   local tok,n = ti()
   local ti = function() tok,n=ti() return tok,n end
   -- collect one macro arguments into an array
   -- stop when reaching a closing parenthesis or a comma
   local function collectArgument(ti, varargs)
      local count = 0
      local tokens = {}
      ti()
      while isSpace(tok) do
         tok = ti()
      end
      while tok do
         if tok == ')' and count == 0 then
            break
         elseif tok == ')' then
            count = count - 1
         elseif tok == '(' then
            count = count + 1
         elseif tok == ',' and count == 0 and not varargs then
            break
         end
         if isSpace(tok) then tok = " " end
         tokens[1+#tokens] = tok
         tok = ti()
      end
      if #tokens > 0 and isSpace(tokens[#tokens]) then
         tokens[#tokens] = nil
      end
      return tokens
   end
   -- collects all macro arguments
   local function collectArguments(ti,def,ntok,nn)
      local args = def.args
      local nargs = { [0]={} }
      if #args == 0 then ti() end
      for _,name in ipairs(args) do
         if tok == ')' and name == "__VA_ARGS__" then
            nargs[0][name] = { negComma=true }
            nargs[name] = { negComma=true }
         else
            xassert(tok=='(' or tok==',', options, nn, "not enough arguments for macro '%s'", ntok)
            local arg = collectArgument(ti, name == "__VA_ARGS__")
            nargs[0][name] = arg
            nargs[name] = callAndCollect(options, expandMacros, macros, yieldFromArray, arg, nn)
         end
      end
      if def.nva then -- named variadic argument (implies dialectGnu)
         nargs[def.nva] = nargs["__VA_ARGS__"]
         nargs[0][def.nva] = nargs[0]["__VA_ARGS__"]
      end
      xassert(tok, options, nn, "unterminated arguments for macro '%s'", ntok)
      xassert(tok==')', options, nn, "too many arguments for macro '%s'", ntok)
      return nargs
   end
   -- coroutine that substitute the macro arguments
   -- and stringification and concatenation are handled here
   local function substituteArguments(options, def, nargs, n, inDirective)
      local uargs = nargs[0] or nargs        -- unexpanded argument values
      if inDirective then nargs = uargs end  -- use unexpanded arguments in directives
      -- prepare loop
      local i,j,k = 1,1,1
      while def[i] do
         if isBlank(def[i]) then
            -- copy blanks
            coroutine.yield(def[i], n)
         else
            -- positions j and k on next non-space tokens
            local function updateJandK()
               if j <= i then j=i
                  repeat j=j+1 until def[j] == nil or not isBlank(def[j]) end
               if k <= j then k=j
                  repeat k=k+1 until def[k] == nil or not isBlank(def[k]) end
            end
            updateJandK()
            -- alternatives
            if def[i]=='#' and def[j] and nargs[def[j]] then
               -- stringification (with the weird quoting rules)
               local v = { '\"' }
               for _,t in ipairs(uargs[def[j]]) do
                  if type(t)=='string' then
                     if t:find("^%s+$") then t = ' ' end
                     if t:find("^[\'\"]") then t = string.format("%q", t):sub(2,-2) end
                     v[1+#v] = t end end
               v[1+#v] = '\"'
               coroutine.yield(tableConcat(v), n)
               i = j
            elseif def.nva and def[i]==',' and def[j]=='##' and def[k]==def.nva then
               -- named variadic macro argument with ## to signal negative comma (gcc crap)
               if nargs[def.nva].negComma then i=i+1 end
               while i < j do coroutine.yield(def[i], n) ; i=i+1 end
            elseif def[i]==',' and def[j]=='__VA_ARGS__' and def[k]==')' then
               -- __VA_ARGS__ with implied negative comma semantics
               if nargs[def[j]].negComma then i=i+1 end
               while i < j do coroutine.yield(def[i], n) ; i=i+1 end
               i = j-1
            elseif def[j]=='##' and def[k] and not inDirective then
               -- concatenation
               local u = {}
               local function addToU(s)
                  if nargs[s] then for _,v in ipairs(uargs[s]) do u[1+#u] = v end
                  else u[1+#u]=s end end
               addToU(def[i])
               while def[j] == '##' and def[k] do
                  addToU(def[k])
                  i = k
                  updateJandK()
               end
               tokenizeLine(options, tableConcat(u), n, true)
            elseif nargs[def[i]] then
               -- substitution
               yieldFromArray(options, nargs[def[i]], n)
            else
               -- copy
               coroutine.yield(def[i], n)
            end
         end
         i = i + 1
      end
   end
   -- main loop
   local newline, directive = true, false
   while tok ~= nil do
      -- detects Zpassed directives
      if newline and tok == '#' then
         newline, directive = false, true
      elseif not isBlank(tok) then
         newline = false
      elseif isNewline(tok) then
         newline, directive = true, false
      end
      -- process code
      local def = macros[tok]
      if not def or directive then
         -- not a macro
         coroutine.yield(tok, n)
      elseif type(def) == 'function' then
         -- magic macro
         def(ti,tok,n)
      elseif def.args == nil then
         -- object-like macro
         prependToken({tag='pop'},n)
         prependTokens(wrap(options, substituteArguments, def, {}, n))
         prependToken({tag='push', symb=tok},n)
      else
         -- function-like macro
         local ntok, nn = tok,n
         local spc = false
         ti()
         if isSpace(tok) then spc=true ti() end
         if (tok ~= '(') then
            coroutine.yield(ntok, nn)
            if spc then coroutine.yield(' ', n) end
            if tok then prependToken(tok,n) end
         else
            local nargs = collectArguments(ti,def,ntok,nn)
            if def.lines == nil then
               -- single-line function-like macro
               prependToken({tag='pop'},n)
               prependTokens(wrap(options, substituteArguments, def, nargs, nn))
               prependToken({tag='push', symb=ntok},nn)
            else
               -- multi-line function-like macro
               local lines = def.lines
               -- a coroutine that yields the macro definition
               local function yieldMacroLines()
                  local count = 0
                  for i=1,#lines,2 do
                     local ls,ln = lines[i], lines[i+1]
                     -- are we possibly in a cpp directive
                     local dir = false
                     if ls[2] and ls[2]:find('^#') then
                        dir = isIdentifier(ls[3]) and ls[3] or ls[4]
                     end
                     if dir and nargs[dir] then
                        dir = false      -- leading stringification
                     elseif dir == 'defmacro' then
                        count = count + 1  -- entering a multiline macto
                     elseif dir == 'endmacro' then
                        count = count - 1  -- leaving a multiline macro
                     end
                     dir = dir or count > 0
                     -- substitute
                     ls = callAndCollect(options,substituteArguments,ls,nargs,ln,dir)
                     -- compute lines (optimize speed by passing body lines as tokens)
                     local j=1
                     while isBlank(ls[j]) do j=j+1 end
                     if ls[j] and ls[j]:find("^#") then -- but not directives
                        ls = ls[1]:sub(2) .. tableConcat(ls, nil, 2)
                     end
                     coroutine.yield(ls,ln)
                  end
               end
               -- recursively reenters preprocessing subroutines in order to handle
               -- preprocessor directives located inside the macro expansion. As a result
               -- we cannot expand macro invocations that extend beyond the macro-expansion.
               local nmacros = {}
               setmetatable(nmacros,{__index=macros})
               if not def.recursive then nmacros[ntok]=false end
               if not def.recursive then coroutine.yield({tag='push',symb=ntok}) end
               expandMacros(options, nmacros, tokenize, processDirectives, nmacros, yieldMacroLines)
               if not def.recursive then coroutine.yield({tag='pop'}) end
            end
         end
      end
      ti()
   end
end


-- Processing conditional directive requires evaluating conditions
-- This function takes an iterator on preprocessed expression tokens
-- and computes the value. This does not handle defined(X) expressions.
-- Optional argument resolver is a function that takes an indentifer
-- name and returns a value. Otherwise zero is assumed

local function evaluateCppExpression(options, tokenIterator, n, resolver)
   -- redefine token iterator to skip spaces and update tok
   local tok
   local function ti()
      repeat tok = tokenIterator()
      until not isBlank(tok) return tok
   end
   -- operator tables
   local unaryOps = {
      ["!"] = function(v) return v == 0 and 1 or 0 end,
      ["~"] = function(v) return bit.bnot(v) end,
      ["+"] = function(v) return v end,
      ["-"] = function(v) return -v end,
      ["L"] = function(v) return v end
   }
   local binaryOps = {
      ["*"] = function(a,b) return a * b end,
      ["/"] = function(a,b) xassert(b~=0,options,n,"division by zero"); return math.floor(a / b) end,
      ["%"] = function(a,b) xassert(b~=0,options,n,"division by zero"); return a % b end,
      ["+"] = function(a,b) return a + b end,
      ["-"] = function(a,b) return a - b end,
      [">>"] = function(a,b) return bit.lshift(a, -b) end,
      ["<<"] = function(a,b) return bit.lshift(a, b) end,
      [">="] = function(a,b) return a >= b and 1 or 0 end,
      ["<="] = function(a,b) return a <= b and 1 or 0 end,
      [">"] = function(a,b) return a > b and 1 or 0 end,
      ["<"] = function(a,b) return a < b and 1 or 0 end,
      ["=="] = function(a,b) return a == b and 1 or 0 end,
      ["!="] = function(a,b) return a ~= b and 1 or 0 end,
      ["&"] = function(a,b) return bit.band(a,b) end,
      ["^"] = function(a,b) return bit.bxor(a,b) end,
      ["|"] = function(a,b) return bit.bor(a,b) end,
      ["&&"] = function(a,b) return (a ~= 0 and b ~= 0) and 1 or 0 end,
      ["||"] = function(a,b) return (a ~= 0 or b ~= 0) and 1 or 0 end,
   }
   local binaryPrec = {
      ["*"] = 1, ["/"] = 1, ["%"] = 1,
      ["+"] = 2, ["-"] = 2,
      [">>"] = 3, ["<<"] = 3,
      [">="] = 4, ["<="] = 4, ["<"] = 4, [">"] = 4,
      ["=="] = 5, ["!="] = 5,
      ["&"] = 6, ["^"] = 7, ["|"] = 8,
      ["&&"] = 9, ["||"] = 10
   }
   -- forward
   local function evaluate() end
   -- unary operations
   local function evalUnary()
      if unaryOps[tok] then
         local op = unaryOps[tok]
         ti(); return op(evalUnary())
      elseif tok == '(' then
         ti(); local v = evaluate()
         xassert(tok == ')', options, n, "missing closing parenthesis")
         ti(); return v
      elseif tok == 'defined' then -- magic macro should have removed this
         xerror(options, n, "syntax error after <defined>")
      elseif isIdentifier(tok) then
         local v = type(resolver) == 'function' and resolver(tok,ti)
         ti(); return v or 0
      elseif isNumber(tok) then
         local v = tok:gsub("[ULul]+$","")
         if v:find("^0[0-7]+$") then
            v = tonumber(v,8) -- octal
         elseif v:find("^0[bB][01]+") then
            v = tonumber(v:sub(3),2) -- binary
         else
            v = tonumber(v) -- lua does the rest
         end
         xassert(v and v == math.floor(v), options, n, "syntax error (invalid integer '%s')", tok)
         ti(); return v
      elseif isString(tok) then
         local v = '""'
         if tok:find("^'") then -- interpret character constant as number
            v = evalLuaExpression(string.format("return string.byte(%s)", tok))
            xassert(type(v)=='number', options, n, "syntax error (invalid value '%s')", tok)
            ti()
         else
            while isString(tok) do
               xassert(tok:find('^"'), options, n, "syntax error (invalid value '%s')", tok)
               v = v:gsub('"$','') .. tok:gsub('^"','')
               ti()
            end
         end
         return v
      end
      xerror(options, n, "syntax error (invalid value '%s')", tok)
   end
   -- binary operations
   local function evalBinary(p)
      if p == 0 then
         return evalUnary()
      else
         local val = evalBinary(p-1)
         while binaryPrec[tok] == p do
            local op = binaryOps[tok]  ti()
            local oval = evalBinary(p-1)
            xassert(p==4 or p==5 or type(val)=='number', options, n,
                    "expression uses arithmetic operators on strings")
            xassert(type(val) == type(oval), options, n,
                    "expression compares numbers and strings")
            val = op(val,oval)
         end
         return val
      end
   end
   -- eval ternary conditonal
   local function evalTernary()
      local c = evalBinary(10)
      if tok ~= '?' then return c end
      ti() local v1 = evalBinary(10)
      xassert(tok == ':', options, n, "expecting ':' after '?'")
      ti() local v2 = evalBinary(10)
      if c == 0 then return v2 else return v1 end
   end

   -- actual definition of evaluate
   evaluate = function()
      return evalTernary()
   end
   -- main function
   ti()
   xassert(tok, options, n, "constant expression expected");
   local result = evaluate()
   if hasOption(options, "-d:eval") then
      xdebug(n, "eval %s", result)
   end
   -- warn about garbage when called from cpp (but not when called with resolver)
   while isBlank(tok) do ti() end
   xassert(resolver or not tok, options, n, "garbage after conditional expression");
   return result
end


-- Now dealing with the coroutine that processes all directives.
-- This coroutine obtains lines from coroutine <lines>,
-- processes all directives, and yields remaining lines

processDirectives = function(options, macros, lines, ...)
   local li = wrap(options, lines, ...)
   local s, n = li()
   -- redefine li to make sure vars s and n are up-to-date
   local li = function() s,n=li() return s,n end
   -- directives store their current token in these vars
   local dirtok, tok, spc
   -- forward declaration
   local processLine
   -- the captureTable mechanism communicates certain preprocessor
   -- events to the declaration parser in order to report them
   -- to the user (the parsing does not depend on this).
   -- if macros[1] is a table, the preprocessor will append
   -- records to this table for the parser to process.
   local function hasCaptureTable()
      local captable = rawget(macros,1)
      return captable and type(captable)=='table' and captable
   end
   local function addToCaptureTable(record)
      local captable = hasCaptureTable()
      if captable and record then captable[1+#captable] = record end
   end

   -- simple directives
   local function doIgnore()
      if hasOption(options, "-Zpass") then coroutine.yield(s, n) end
   end
   local function doError()
      xerror(options, n, "unexpected preprocessor directive #%s", dirtok)
   end
   local function doMessage()
      local msg = s:match("^%s*#+%s*[a-z]*%s+([^%s].*)")
      xmessage(dirtok, options, n, msg or '#' .. dirtok)
   end
   -- undef
   local function doUndef(ti)
      ti()
      local nam = tok
      xassert(isIdentifier(nam), options, n, "symbol expected after #undef")
      if ti() then xwarning(options, n, "garbage after #undef directive") end
      if hasOption(options, "-d:defines") then xdebug(n, "undef %s", nam) end
      if hasCaptureTable() and macros[nam] and macros[nam].captured then
         addToCaptureTable{directive='undef',name=nam, where=n}
      end
      macros[nam] = false -- false overrides inherited definitions
   end
   -- define
   local function getMacroArguments(ti)
      local args = {}
      local msg = "argument list in function-like macro"
      local nva = nil -- named variadic argument
      ti()
      while tok and tok ~= ')' do
         local nam = tok
         ti()
         if options.dialectGnu and isIdentifier(nam) and tok == '...' then nam,nva=tok,nam ; ti() end
         xassert(nam ~= "__VA_ARGS__", options, n, "name __VA_ARGS__ is not allowed here")
         xassert(tok == ')' or nam ~= '...', options, n, "ellipsis in argument list must appear last")
         xassert(tok == ')' or tok == ',', options, n, "bad " .. msg)
         if tok == ',' then ti() end
         if nam == '...' then nam = "__VA_ARGS__" end
         xassert(isIdentifier(nam), options, n, "bad " .. msg)
         args[1+#args] = nam
      end
      xassert(tok == ')', options, n, "unterminated " .. msg)
      ti()
      return args, nva
   end
   local function doDefine(ti)
      xassert(isIdentifier(ti()), options, n, "symbol expected after #define")
      local nam, args, nva = tok, nil, nil
      -- collect arguments
      if ti() == '(' and not spc then
         args, nva = getMacroArguments(ti)
      end
      -- collect definition
      local def = { tok, args = args, nva = nva }
      while ti(true) do
         def[1+#def] = tok
      end
      -- define macro
      if macros[nam] and not tableCompare(def,macros[nam]) then
         xwarning(options, n,"redefinition of preprocessor symbol '%s'", nam)
      end
      macros[nam] = def
      -- debug
      if hasOption(options, "-d:defines") then
         if args then args = "(" .. tableConcat(args,",") .. ")"  end
         xdebug(n, "define %s%s = %s", nam, args or "", tableConcat(def,' '))
      end
      -- capture integer macro definitions
      if hasCaptureTable() and args == nil then
         local i = 0
         local v = callAndCollect(options, expandMacros, macros, yieldFromArray, def, n)
         local function ti() i=i+1 return v[i] end
         local ss,r = pcall(evaluateCppExpression,{silent=true}, ti, n, error)
         if ss and type(r)=='number' then
            def.captured = true
            addToCaptureTable{directive='define', name=nam, intval=r, where=n}
         end
      end
   end
   -- defmacro
   local function checkDirective(stop)
      xassert(s, options, n, "unterminated macro (missing #%s)", stop)
      local r = type(s) == 'string' and s:match("^%s*#%s*([a-z]+)")
      if r == "endmacro" or r == "endif" then
         if s:find(r .. "%s*[^%s]") then
            xwarning(options, n, "garbage after #%s directive", r)
         end
      end
      return r
   end
   local function doMacroLines(lines, stop)
      while true do
         li()
         local ss = callAndCollect(options,tokenizeLine,s,n)
         if #ss > 0 then lines[1+#lines] = ss ; lines[1+#lines] = n end
         local r = checkDirective(stop)
         if r == "endmacro" or r == "endif" then
            xassert(r == stop, options,n, "unbalanced directives (got #%s instead of #%s)",r,stop)
            return r
         elseif r=="defmacro" then
            doMacroLines(lines,"endmacro")
         elseif r == "if" or r == "ifdef" or r == "ifndef" then
            doMacroLines(lines,"endif")
         end
      end
   end
   local function doDefmacro(ti)
      xassert(isIdentifier(ti()), options, n, "symbol expected after #defmacro")
      local nam,nn = tok,n
      xassert(ti()=='(', options, n, "argument list expected in #defmacro")
      local args, nva = getMacroArguments(ti)
      xassert(not tok, options, n, "garbage after argument list in #defmacro")
      -- collect definition
      local lines = {}
      local def = { args=args, nva=nva, lines=lines, recursive=(dirtok=="defrecursivemacro") }
      doMacroLines(lines,"endmacro")
      lines[#lines] = nil
      lines[#lines] = nil
      if hasOption(options,"-d:directives") then
         xdebug(n, "directive: #endmacro")
      end
      if macros[nam] and not tableCompare(def,macros[nam]) then
         xwarning(options, n, "redefinition of preprocessor symbol '%s'", nam)
      end
      if hasOption(options, "-d:defines") then
         xdebug(nn, "defmacro %s(%s) =", nam, tableConcat(args,','))
         for i=1,#lines,2 do
            xdebug(lines[i+1], "\t%s", tableConcat(lines[i]):gsub("^\n","")) end
      end
      macros[nam] = def
   end
   -- include
   local function doInclude(ti)
      -- get filename
      local pti = wrap(options, expandMacros, macros, yieldFromIterator, ti)
      local tok = pti()
      while isBlank(tok) do tok=pti() end
      if tok == '<' then              -- computed include
         repeat local tok2 = pti()
           tok = tok .. tostring(tok2)
         until tok2==nil or tok2=='>' or isNewline(tok2)
         tok = tok:gsub('%s>$','>')   -- gcc does this
      end
      xassert(isHeaderName(tok), options, n, "malformed header name after #include")
      local ttok = pti()
      while isBlank(ttok) do ttok=pti() end
      if ttok then xwarning(options, n, "garbage after #include directive") end
      -- interpret filename
      local sys = tok:byte() == 60
      local min = dirtok=="include_next" and options.includedir or 0
      local fname = evalLuaExpression(string.format("return '%s'", tok:sub(2,-2)))
      local pname, fd, fdi
      for i,v in ipairs(options) do
         if v == "-I-" then
            sys=false
         elseif i > min and v:find("^%-I") and not sys then
            pname = v:match("^%-I%s*(.*)") .. '/' .. fname
            fdi, fd = i, io.open(pname, "r")
            if fd then break end
         end
      end
      if fd then
         -- include file
         if hasOption(options, "-d:include") then xdebug(n, "including %q", pname) end
         local savedfdi = options.includedir
         options.includedir = fdi -- saved index to implement include_next
         processDirectives(options, macros, eliminateComments, joinLines,
                           yieldLines, fd:lines(), pname)
         options.includedir = savedfdi
      else
         -- include file not found
         if hasOption(options, "-Zpass") then
            coroutine.yield(string.format("#include %s",tok), n)
         else
            xwarning(options, n, "include directive (%s) was unresolved", tok)
         end
         -- quirks
         if knownIncludeQuirks[tok] then
            processDirectives(options, macros, eliminateComments, joinLines,
                              yieldFromArray, knownIncludeQuirks[tok], n)
         end
         -- capture
         if hasCaptureTable() then
            addToCaptureTable{directive='include',name=tok, where=n}
         end
      end
   end
   -- conditionals
   local function doConditionalBranch(execute)
      checkDirective("endif")
      while true do
         li()
         local r = checkDirective("endif")
         if r == "else" or r == "elif" or r == "endif" then
            return r
         elseif execute then
            processLine()
         elseif r == "if" or r == "ifdef" or r == "ifndef" then
            while doConditionalBranch(false) ~= "endif" do end
         end
      end
   end
   local function doConditional(result)
      local r = doConditionalBranch(result)
      if r == 'elif' and not result then
         return processLine(true)
      end
      while r ~= "endif" do
         r = doConditionalBranch(not result)
      end
      if hasOption(options,"-d:directives") then
         xdebug(n, "directive: %s",s:gsub('^%s*',''))
      end
   end
   local function doIfdef(ti)
      ti()
      xassert(isIdentifier(tok), options, n, "symbol expected after #%s", dirtok)
      local result = macros[tok]
      if ti() then xwarning(options, n, "garbage after #undef directive") end
      if dirtok == 'ifndef' then result = not result end
      doConditional(result)
   end
   local function doIf(ti)
      -- magic macro for 'defined'
      local nmacros = {}
      setmetatable(nmacros,{__index=macros})
      nmacros['defined'] = function(ti)
         local tok,n = ti()
         if tok == '(' then tok = ti()
            if ti() ~= ')' then tok = nil end end
         if isIdentifier(tok) then
            coroutine.yield(macros[tok] and "1" or "0", n)
         else
            coroutine.yield("defined", n) -- error
         end
      end
      -- evaluate and branch
      local pti = wrap(options, expandMacros, nmacros, yieldFromIterator, ti)
      local result = evaluateCppExpression(options, pti, n)
      doConditional(result ~= 0)
   end
   -- table of directives
   local directives = {
      ["else"] = doError, ["elif"] = doError, ["endif"] = doError,
      ["pragma"] = doIgnore, ["ident"] = doIgnore, ["line"] = doIgnore,
      ["error"] = doMessage, ["warning"] = doMessage,
      ["if"] = doIf, ["ifdef"] = doIfdef, ["ifndef"] = doIfdef,
      ["define"] = doDefine, ["undef"] = doUndef,
      ["defmacro"] = doDefmacro, ["defrecursivemacro"] = doDefmacro,
      ["endmacro"] = doError,
      ["include"] = doInclude, ["include_next"] = doInclude,
   }
   -- process current line
   processLine = function(okElif)
      if type(s) == 'table' then
         -- optimization for multiline macros:
         -- When s is an an array of precomputed tokens, code is assumed.
         coroutine.yield(s, n)
      elseif not s:find("^%s*#") then
         -- code
         coroutine.yield(s, n)
      elseif s:find("^%s*##") and hasOption(options, "-Zpass") then
         -- pass
         local ns = s:gsub("^(%s*)##","%1#")
         coroutine.yield(ns, n)
      else
         if hasOption(options, "-d:directives") then
            xdebug(n, "directive: %s",s:gsub('^%s*',''))
         end
         -- tokenize directive
         local ti = wrap(options, tokenizeLine, s, n)
         -- a token iterator that skips spaces unless told otherwise
         local ti = function(keepSpaces)
            tok = ti()
            spc = isSpace(tok)
            while not keepSpaces and isBlank(tok) do
               tok = ti()
               spc = spc or isSpace(tok)
            end
            return tok, n
         end
         -- start parsing directives
         ti()
         assert(tok=='#' or tok=='##')
         if tok == '##' then
            xwarning(options, n, "directive starts with ## without -Zpass") end
         dirtok = ti()
         if isIdentifier(tok) then
            local f = directives[dirtok]
            if okElif and dirtok == "elif" then f = doIf end
            xassert(f, options, n, "unrecognized preprocessor directive #%s", tok)
            f(ti)
         elseif tok ~= nil then
            xerror(options, n, "unrecognized preprocessor directive '#%s'", s:gsub("^%s*",""))
         end
      end
   end
   -- main loop
   while s ~= nil do
      processLine()
      li()
   end
end


-- This function yields initialization lines

local function initialDefines(options)
   -- cpp-extracted definitions
   if hasOption(options, "-Zcppdef") then
      local fd = io.popen("cpp -dM < /dev/null","r")
      yieldLines(options, fd:lines(), "<cppdef>")
      fd:close()
   end
   -- builtin definitions
   local sb = { "#define __CPARSER__ 1" }
   local function addDef(s,v)
      sb[1+#sb] = string.format("#ifndef %s",s)
      sb[1+#sb] = string.format("# define %s %s",s,v)
      sb[1+#sb] = string.format("#endif")
   end
   addDef("__STDC__", "1")
   local stdc = "199409L"
   if options.dialect11 then stdc = "201112L" end
   if options.dialect99 then stdc = "199901L" end
   addDef("__STDC_VERSION__", stdc)
   if options.dialectGnu then
      addDef("__GNUC__", 4)
      addDef("__GNUC_MINOR__", 2)
   end
   yieldLines(options, wrap(options, yieldFromArray, sb), "<builtin>")
   -- command line definitions
   local sc = {}
   for _,v in ipairs(options) do
      local d
      if v:find("^%-D(.*)=") then
         d = v:gsub("^%-D%s*(.*)%s*=%s*(.-)%s*$","#define %1 %2")
      elseif v:find("^%-D") then
         d = v:gsub("^%-D%s*(.-)%s*$","#define %1 1")
      elseif v:find("^%-U") then
         d = v:gsub("^%-U%s*(.-)%s*$","#undef %1")
      end
      if d then sc[1+#sc] = d end
   end
   yieldLines(options, wrap(options, yieldFromArray, sc), "<cmdline>")
end


-- This function creates the initial macro directory

local function initialMacros(options)
   local macros = {}
   -- magic macros
   macros["__FILE__"] = function(_,_,n)
      local f
      if type(n) == 'string' then f=n:match("^[^:]*") end
      coroutine.yield(string.format("%q", f or "<unknown>"), n)
   end
   macros["__LINE__"] = function(_,_,n)
      local d = n
      if type(d) == 'string' then d=tonumber(d:match("%d*$")) end
      coroutine.yield(string.format("%d", d or 0), n)
   end
   macros["__DATE__"] = function(_,_,n)
      coroutine.yield(string.format("%q", os.date("%b %e %Y")), n)
   end
   macros["__TIME__"] = function(_,_,n)
      coroutine.yield(string.format("%q", os.date("%T")), n)
   end
   --< @r-lyeh: add __COUNTER__
   macros["__COUNTER__"] = function(_,_,n)
      counter = counter or 0
      counter = counter + 1
      coroutine.yield(string.format("%d", counter), n)
   end
   --<
   -- initial macros
   local li = wrap(options,processDirectives,macros,initialDefines)
   for _ in li do end
   -- return
   return macros
end


-- This function prepares a string containing the definition of the
-- macro named <name> in macro definition table <macros>, or nil if no
-- such definition exists.

local function macroToString(macros, name)
   local v = macros[name]
   if type(v) == 'table' then
      local dir = 'define'
      if v.recursive and v.lines then
         dir = 'defrecursivemacro'
      elseif v.lines then
         dir = 'defmacro'
      end
      local arr = {"#", dir, ' ', name }
      if v.args then
         arr[1+#arr] = '('
         for i,s in ipairs(v.args) do
            if i ~= 1 then arr[1+#arr] = ',' end
            if s == '__VA_ARGS__' then s = (v.nva or '') .. '...' end
            arr[1+#arr] = s
         end
         arr[1+#arr] = ') '
      else
         arr[1+#arr] = ' '
      end
      for _,s in ipairs(v) do
         arr[1+#arr] = s
      end
      if v.lines then
         for i = 1, #v.lines, 2 do
            local vl = v.lines[i]
            arr[1+#arr] = '\n'
            if type(vl)=='table' then vl=tableConcat(vl) end
            arr[1+#arr] = vl:gsub('^%s?%s?','  '):gsub('^\n','')
         end
         arr[1+#arr] = '\n'
         arr[1+#arr] = "#endmacro"
      end
      return tableConcat(arr)
   end
end

-- This function dumps all macros to file descriptor outputfile

local function dumpMacros(macros, outputfile)
   outputfile = outputfile or io.output()
   assert(type(macros) == 'table')
   assert(io.type(outputfile) == 'file')
   for k,_ in pairs(macros) do
      local s = macroToString(macros,k)
      if s then outputfile:write(string.format("%s\n", s)) end
   end
end



-- A coroutine that filters out spaces, directives, and magic tokens

local function filterSpaces(options, tokens, ...)
   local ti = wrap(options, tokens, ...)
   local tok,n = ti()
   while tok do
      -- skip directives
      while isNewline(tok) do
         tok,n = ti()
         while isBlank(tok) do tok,n = ti() end
         if tok == '#' then
            while not isNewline(tok) do
               tok, n = ti()
            end
         end
      end
      -- output nonspaces
      if not isBlank(tok) then
         coroutine.yield(tok, n)
      end
      tok,n = ti()
   end
end

-- This function takes a line iterator and an optional location prefix.
-- It returns a token iterator for the preprocessed tokens
-- and a table of macro definitions.

local function cppTokenIterator(options, lines, prefix)
   options = copyOptions(options)
   prefix = prefix or ""
   assert(type(options)=='table')
   assert(type(lines)=='function')
   assert(type(prefix)=='string')
   local macros = initialMacros(options)
   local ti = wrap(options,
                   filterSpaces,
                   expandMacros, macros,
                   tokenize,
                   processDirectives, macros,
                   eliminateComments,
                   joinLines,
                   yieldLines, lines, prefix)
   return ti, macros
end


-- A coroutine that reconstructs lines from the preprocessed tokens

local function preprocessedLines(options, tokens, ...)
   local ti = wrap(options, tokens, ...)
   local tok,n = ti()
   while tok do
      local curn = n
      local curl = {}
      if isNewline(tok) then
         curn = n
         curl[1+#curl] = tok:sub(2)
         tok, n = ti()
      end
      while tok and not isNewline(tok) do
         if not isMagic(tok) then curl[1+#curl] = tok end
         tok, n = ti()
      end
      coroutine.yield(table.concat(curl), curn)
   end
end


-- This function preprocesses file <filename>.
-- The optional argument <outputfile> specifies where to write the
-- preprocessed file and may be a string or a file descriptor.
-- The optional argument <options> contains an array of option strings.
-- Note that option "-Zpass" is added, unless the option "-Znopass" is present.

--< @r-lyeh: iterator+string methods
local function cppIterator(lines, outputfile, options, prefix)
   -- handle optional arguments
   prefix = prefix or ''
   options = copyOptions(options)
   outputfile = outputfile or "-"
   assert(type(lines)=='function')
   assert(type(options)=='table')
   local closeoutputfile = false
   if io.type(outputfile) ~= 'file' then
      assert(type(outputfile) == 'string')
      if outputfile == '-' then
         outputfile = io.output()
      else
         closeoutputfile = true
         outputfile = io.open(outputfile,"w")
      end
   end
   assert(io.type(outputfile) == 'file')
   -- makes option -Zpass on by default
   if not hasOption(options,"-Znopass") then
      options.hash["-Zpass"] = true
   end
   -- prepare iterator
   local dM = hasOption(options, "-dM")
   local macros = initialMacros(options)
   local li = wrap(options,
                   preprocessedLines,
                   expandMacros, macros,
                   tokenize,
                   processDirectives, macros,
                   eliminateComments,
                   joinLines,
                   yieldLines, lines, prefix)
   -- iterate, inserting line markers
   local lm = hasOption(options,"-Zpass") and "line" or ""
   local cf, cn
   for s,n in li do
      if not dM and s:find("[^%s]") then
         local xf, xn
         if type(n) == 'number' then
            xn = n
         elseif type(n) == 'string' then
            xf, xn = n:match("^([^:]*).-(%d*)$")
            xn = tonumber(xn)
         end
         if cf ~= xf or cn ~= xn then
            cf, cn = xf, xn
            if lm ~= "" then --< @r-lyeh: do not output empty #line directives: ie, `#line 19 "file.c"` -> `# 19 ""`
            outputfile:write(string.format("#%s %d %q\n", lm, cn, cf))
            end
         end
         outputfile:write(s)
         outputfile:write("\n")
         cn = cn + 1
      end
   end
   if dM then
      dumpMacros(macros, outputfile)
   end
   if closeoutputfile then outputfile:close() end
end

local function cppString(str, outputfile, options)
   assert(type(str)=='string')

   return cppIterator(str:gmatch("[^\r\n]+"), outputfile, options)
end

local function cpp(filename, outputfile, options)
   assert(type(filename)=='string')

   return cppIterator(io.lines(filename), outputfile, options, filename)
end
--<

---------------------------------------------------
---------------------------------------------------
---------------------------------------------------
-- EXPORTS

cparser = {}

cparser.cpp = cpp
cparser.cppTokenIterator = cppTokenIterator
cparser.macroToString = macroToString

--< @r-lyeh: add string methods
cparser.cppString = cppString
cparser.parseString = parseString
--<

return cparser
