local syntax = require "core.syntax"

local patterns = {}

local symbols = {
  ["nil"] = "literal",
  ["true"] = "literal",
  ["false"] = "literal",
}

local number_patterns = {
  "0[bB][01][01_]*",
  "0o[0-7][0-7_]*",
  "0[xX]%x[%x_]*",
  "%d[%d_]*%.%d[%d_]*[eE][-+]?%d[%d_]*",
  "%d[%d_]*%.%d[%d_]*",
  "%d[%d_]*",
}

local type_suffix_patterns = {}

for _, size in ipairs({"", "8", "16", "32", "64"}) do
  table.insert(type_suffix_patterns, "'?[fuiFUI]"..size)
end

for _, pattern in ipairs(number_patterns) do
  for _, suffix in ipairs(type_suffix_patterns) do
    table.insert(patterns, { pattern = pattern..suffix, type = "literal" })
  end
  table.insert(patterns, { pattern = pattern, type = "literal" })
end

local keywords = {
  "addr", "and", "as", "asm",
  "bind", "block", "break",
  "case", "cast", "concept", "const", "continue", "converter",
  "defer", "discard", "distinct", "div", "do",
  "elif", "else", "end", "enum", "except", "export",
  "finally", "for", "from", "func",
  "if", "import", "in", "include", "interface", "is", "isnot", "iterator",
  "let",
  "macro", "method", "mixin", "mod",
  "not", "notin",
  "object", "of", "or", "out",
  "proc", "ptr",
  "raise", "ref", "return",
  "shl", "shr", "static",
  "template", "try", "tuple", "type",
  "using",
  "var",
  "when", "while",
  "xor",
  "yield",
}

for _, keyword in ipairs(keywords) do
  symbols[keyword] = "keyword"
end

local standard_types = {
  "bool", "byte",
  "int", "int8", "int16", "int32", "int64",
  "uint", "uint8", "uint16", "uint32", "uint64",
  "float", "float32", "float64",
  "char", "string", "cstring",
  "pointer",
  "typedesc",
  "void", "auto", "any",
  "untyped", "typed",
  "clong", "culong", "cchar", "cschar", "cshort", "cint", "csize", "csize_t",
  "clonglong", "cfloat", "cdouble", "clongdouble", "cuchar", "cushort",
  "cuint", "culonglong", "cstringArray",
}

for _, type in ipairs(standard_types) do
  symbols[type] = "keyword2"
end

local standard_generic_types = {
  "range",
  "array", "open[aA]rray", "varargs", "seq", "set",
  "sink", "lent", "owned",
}

for _, type in ipairs(standard_generic_types) do
  table.insert(patterns, { pattern = type.."%f[%[]", type = "keyword2" })
  table.insert(patterns, { pattern = type.." +%f[%w]", type = "keyword2" })
end

local user_patterns = {
  -- comments
  { pattern = { "##?%[", "]##?" },            type = "comment" },
  { pattern = "##?.-\n",                      type = "comment" },
  -- strings and chars
  { pattern = { '"', '"', '\\' },             type = "string" },
  { pattern = { '"""', '"""[^"]' },           type = "string" },
  { pattern = { "'", "'", '\\' },             type = "literal" },
  -- function calls
  { pattern = "[a-zA-Z][a-zA-Z0-9_]*%f[(]",   type = "function" },
  -- identifiers
  { pattern = "[A-Z][a-zA-Z0-9_]*",           type = "keyword2" },
  { pattern = "[a-zA-Z][a-zA-Z0-9_]*",        type = "symbol" },
  -- operators
  { pattern = "%.%f[^.]",                     type = "normal" },
  { pattern = ":%f[ ]",                       type = "normal" },
  { pattern = "[=+%-*/<>@$~&%%|!?%^&.:\\]+",  type = "operator" },
}

for _, pattern in ipairs(user_patterns) do
  table.insert(patterns, pattern)
end

local nim = {
  files = { "%.nim$", "%.nims$", "%.nimble$" },
  comment = "#",
  patterns = patterns,
  symbols = symbols,
}

syntax.add(nim)