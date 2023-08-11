--
-- Name:        premake-ninja/ninja.lua
-- Purpose:     Define the ninja action.
-- Author:      Dmitry Ivanov
-- Created:     2015/07/04
-- Copyright:   (c) 2015 Dmitry Ivanov
--

local p = premake
local tree = p.tree
local project = p.project
local config = p.config
local fileconfig = p.fileconfig

premake.modules.ninja = {}
local ninja = p.modules.ninja

local function get_key(cfg)
	if cfg.platform then
		return cfg.project.name .. "_" .. cfg.buildcfg .. "_" .. cfg.platform
	else
		return cfg.project.name .. "_" .. cfg.buildcfg
	end
end

local build_cache = {}

local function add_build(cfg, output, extra_outputs, command, args)
	local cached = build_cache[output]
	if cached ~= nil then
		if extra_outputs == cached.extra_outputs
			and command == cached.command
			and table.equals(args or {}, cached.args or {})
		then
			-- custom_command rule is identical for each configuration (contrary to other rules)
			-- So we can compare extra parameter
			if string.startswith(cached.command, "custom_command") then
				p.w("# INFO: Rule ignored, same as " .. cached.cfg_key)
			else
				local cfg_key = get_key(cfg)
				p.warn(cached.cfg_key .. " and " .. cfg_key .. " both generate (differently?) " .. output .. ". Ignoring " .. cfg_key)
				p.w("# WARNING: Rule ignored, using the one from " .. cached.cfg_key)
			end
		else
			local cfg_key = get_key(cfg)
			p.warn(cached.cfg_key .. " and " .. cfg_key .. " both generate differently " .. output .. ". Ignoring " .. cfg_key)
			p.w("# ERROR: Rule ignored, using the one from " .. cached.cfg_key)
		end
		p.w("# build " .. output .. ": " .. command)
		for i, arg in ipairs(args or {}) do
			p.w("#   " .. arg)
		end
		return
	end
	p.w("build " .. output .. ": " .. command)
	for i, arg in ipairs(args or {}) do
		p.w("  " .. arg)
	end
	build_cache[output] = {
		cfg_key = get_key(cfg),
		extra_outputs = extra_outputs,
		command = command,
		args = args
	}
end

function ninja.esc(value)
	value = value:gsub("%$", "$$") -- TODO maybe there is better way
	value = value:gsub(":", "$:")
	value = value:gsub("\n", "$\n")
	value = value:gsub(" ", "$ ")
	return value
end

function ninja.quote(value)
	value = value:gsub("\\", "\\\\")
	value = value:gsub("'", "\\'")
	value = value:gsub("\"", "\\\"")

	return "\"" .. value .. "\""
end

-- in some cases we write file names in rule commands directly
-- so we need to propely escape them
function ninja.shesc(value)
	if type(value) == "table" then
		local result = {}
		local n = #value
		for i = 1, n do
			table.insert(result, ninja.shesc(value[i]))
		end
		return result
	end

	if value:find(" ") then
		return ninja.quote(value)
	end
	return value
end

-- generate solution that will call ninja for projects
function ninja.generateWorkspace(wks)
	local oldGetDefaultSeparator = path.getDefaultSeparator
	path.getDefaultSeparator = function() return "/" end

	p.w("# solution build file")
	p.w("# generated with premake ninja")
	p.w("")

	p.w("# build projects")
	local cfgs = {} -- key is concatenated name or variant name, value is string of outputs names
	local key = ""
	local cfg_first = nil
	local cfg_first_lib = nil

	for prj in p.workspace.eachproject(wks) do
		if p.action.supports(prj.kind) and prj.kind ~= p.NONE then
			for cfg in p.project.eachconfig(prj) do
				key = prj.name .. "_" .. cfg.buildcfg

				if cfg.platform ~= nil then key = key .. "_" .. cfg.platform end

				if not cfgs[cfg.buildcfg] then cfgs[cfg.buildcfg] = "" end
				cfgs[cfg.buildcfg] = cfgs[cfg.buildcfg] .. key .. " "

				-- set first configuration name
				if (cfg_first == nil) and (cfg.kind == p.CONSOLEAPP or cfg.kind == p.WINDOWEDAPP) then
					cfg_first = key
				end
				if (cfg_first_lib == nil) and (cfg.kind == p.STATICLIB or cfg.kind == p.SHAREDLIB) then
					cfg_first_lib = key
				end

				-- include other ninja file
				p.w("subninja " .. p.esc(ninja.projectCfgFilename(cfg, true)))
			end
		end
	end

	if cfg_first == nil then cfg_first = cfg_first_lib end

	p.w("")

	p.w("# targets")
	for cfg, outputs in pairs(cfgs) do
		p.w("build " .. p.esc(cfg) .. ": phony " .. outputs)
	end
	p.w("")

	p.w("# default target")
	p.w("default " .. p.esc(cfg_first))
	p.w("")

	path.getDefaultSeparator = oldGetDefaultSeparator
end

function ninja.list(value)
	if #value > 0 then
		return " " .. table.concat(value, " ")
	else
		return ""
	end
end

local function shouldcompileasc(filecfg)
	if filecfg.compileas and filecfg.compileas ~= "Default" then
		return p.languages.isc(filecfg.compileas)
	end
	return path.iscfile(filecfg.abspath)
end

local function shouldcompileascpp(filecfg)
	if filecfg.compileas and filecfg.compileas ~= "Default" then
		return p.languages.iscpp(filecfg.compileas)
	end
	return path.iscppfile(filecfg.abspath)
end

local function getDefaultToolsetFromOs()
	local system_name = os.target()

	if system_name == "windows" then
		return "msc"
	elseif system_name == "macosx" then
		return "clang"
	elseif system_name == "linux" then
		return "gcc"
	else
		p.warnOnce("unknown_system", "no toolchain set and unknown system " .. system_name .. " so assuming toolchain is gcc")
		return "gcc"
	end
end

local function getToolsetExecutables(cfg, toolset, toolset_name)
	local cc = ""
	local cxx = ""
	local ar = ""
	local link = ""
	local rc = ""

	if toolset_name == "msc" then
		-- TODO premake doesn't set tools names for msc, do we want to fix it ?
		cc = "cl"
		cxx = "cl"
		ar = "lib"
		link = "cl"
		rc = "rc"
	elseif toolset_name == "clang" or toolset_name == "gcc" then
		if not cfg.gccprefix then cfg.gccprefix = "" end
		cc = toolset.gettoolname(cfg, "cc")
		cxx = toolset.gettoolname(cfg, "cxx")
		ar = toolset.gettoolname(cfg, "ar")
		link = toolset.gettoolname(cfg, iif(cfg.language == "C", "cc", "cxx"))
	else
		p.error("unknown toolchain " .. toolset_name)
	end
	return cc, cxx, ar, link, rc
end

local function getFileDependencies(cfg)
	local dependencies = {}
	if #cfg.prebuildcommands > 0 or cfg.prebuildmessage then
		dependencies = {"prebuild_" .. get_key(cfg)}
	end
	for i = 1, #cfg.dependson do
		table.insert(dependencies, cfg.dependson[i] .. "_" .. cfg.buildcfg)
	end
	return dependencies
end

local function getcflags(toolset, cfg, filecfg)
	local buildopt = ninja.list(filecfg.buildoptions)
	local cppflags = ninja.list(toolset.getcppflags(filecfg))
	local cflags = ninja.list(toolset.getcflags(filecfg))
	local defines = ninja.list(table.join(toolset.getdefines(filecfg.defines), toolset.getundefines(filecfg.undefines)))
	local includes = ninja.list(toolset.getincludedirs(cfg, filecfg.includedirs, filecfg.externalincludedirs))
	local forceincludes = ninja.list(toolset.getforceincludes(cfg))

	return buildopt .. cppflags .. cflags .. defines .. includes .. forceincludes
end

local function getcxxflags(toolset, cfg, filecfg)
	local buildopt = ninja.list(filecfg.buildoptions)
	local cppflags = ninja.list(toolset.getcppflags(filecfg))
	local cxxflags = ninja.list(toolset.getcxxflags(filecfg))
	local defines = ninja.list(table.join(toolset.getdefines(filecfg.defines), toolset.getundefines(filecfg.undefines)))
	local includes = ninja.list(toolset.getincludedirs(cfg, filecfg.includedirs, filecfg.externalincludedirs))
	local forceincludes = ninja.list(toolset.getforceincludes(cfg))
	return buildopt .. cppflags .. cxxflags .. defines .. includes .. forceincludes
end

local function getldflags(toolset, cfg)
	local ldflags = ninja.list(table.join(toolset.getLibraryDirectories(cfg), toolset.getldflags(cfg), cfg.linkoptions))

	-- experimental feature, change install_name of shared libs
	--if (toolset_name == "clang") and (cfg.kind == p.SHAREDLIB) and ninja.endsWith(cfg.buildtarget.name, ".dylib") then
	--	ldflags = ldflags .. " -install_name " .. cfg.buildtarget.name
	--end
	return ldflags
end

local function prebuild_rule(cfg)
	if #cfg.prebuildcommands > 0 or cfg.prebuildmessage then
		local commands = {}
		if cfg.prebuildmessage then
			commands = {os.translateCommandsAndPaths("{ECHO} " .. cfg.prebuildmessage, cfg.project.basedir, cfg.project.location)}
		end
		commands = table.join(commands, os.translateCommandsAndPaths(cfg.prebuildcommands, cfg.project.basedir, cfg.project.location))
		if (#commands > 1) then
			commands = 'sh -c ' .. ninja.quote(table.implode(commands,"","",";"))
		else
			commands = commands[1]
		end
		p.w("rule run_prebuild")
		p.w("  command = " .. p.esc(commands))
		p.w("  description = prebuild")
		p.w("")
	end
end

local function postbuild_rule(cfg)
	if #cfg.postbuildcommands > 0 or cfg.postbuildmessage then
		local commands = {}
		if cfg.postbuildmessage then
			commands = {os.translateCommandsAndPaths("{ECHO} " .. cfg.postbuildmessage, cfg.project.basedir, cfg.project.location)}
		end
		commands = table.join(commands, os.translateCommandsAndPaths(cfg.postbuildcommands, cfg.project.basedir, cfg.project.location))
		if (#commands > 1) then
			commands = 'sh -c ' .. ninja.quote(table.implode(commands,"","",";"))
		else
			commands = commands[1]
		end
		p.w("rule run_postbuild")
		p.w("  command = " .. p.esc(commands))
		p.w("  description = postbuild")
		p.w("")
	end
end

local function compilation_rules(cfg, toolset, toolset_name, pch)
	---------------------------------------------------- figure out toolset executables
	local cc, cxx, ar, link, rc = getToolsetExecutables(cfg, toolset, toolset_name)

	local all_cflags = getcflags(toolset, cfg, cfg)
	local all_cxxflags = getcxxflags(toolset, cfg, cfg)
	local all_ldflags = getldflags(toolset, cfg)

	if toolset_name == "msc" then
		-- for some reason Visual Studio add this libraries as "defaults" and premake doesn't tell us this
		local default_msvc_libs = " kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib"

		p.w("rule cc")
		p.w("  command = " .. cc .. all_cflags .. " /nologo /showIncludes -c $in /Fo$out")
		p.w("  description = cc $out")
		p.w("  deps = msvc")
		p.w("")
		p.w("rule cxx")
		p.w("  command = " .. cxx .. all_cxxflags .. " /nologo /showIncludes -c $in /Fo$out")
		p.w("  description = cxx $out")
		p.w("  deps = msvc")
		p.w("")
		p.w("rule cc_flags")
		p.w("  command = " .. cc .. " $CFLAGS" .. " /nologo /showIncludes -c $in /Fo$out")
		p.w("  description = cc $out")
		p.w("  deps = msvc")
		p.w("")
		p.w("rule cxx_flags")
		p.w("  command = " .. cxx .. " $CXXFLAGS" .. " /nologo /showIncludes -c $in /Fo$out")
		p.w("  description = cxx $out")
		p.w("  deps = msvc")
		p.w("")
		p.w("rule rc")
		p.w("  command = " .. rc .. " /nologo /fo$out $in")
		p.w("  description = rc $out")
		p.w("")
		if cfg.kind == p.STATICLIB then
			p.w("rule ar")
			p.w("  command = " .. ar .. " $in /nologo -OUT:$out")
			p.w("  description = ar $out")
			p.w("")
		else
			p.w("rule link")
			p.w("  command = " .. link .. " $in" .. ninja.list(ninja.shesc(toolset.getlinks(cfg, true))) .. default_msvc_libs .. " /link" .. all_ldflags .. " /nologo /out:$out")
			p.w("  description = link $out")
			p.w("")
		end
	elseif toolset_name == "clang" then
		local force_include_pch = ""
		if pch then
			force_include_pch = " -include " .. p.esc(pch.placeholder)
			p.w("rule build_pch")
			p.w("  command = " .. iif(cfg.language == "C", cc .. all_cflags .. " -x c-header", cxx .. all_cxxflags .. " -x c++-header")  .. " -H -MMD -MF $out.d -c -o $out $in")
			p.w("  description = build_pch $out")
			p.w("  depfile = $out.d")
			p.w("  deps = gcc")
		end
		p.w("rule cc")
		p.w("  command = " .. cc .. all_cflags .. force_include_pch .. " -x c -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cc $out")
		p.w("  depfile = $out.d")
		p.w("  deps = gcc")
		p.w("")
		p.w("rule cxx")
		p.w("  command = " .. cxx .. all_cxxflags .. force_include_pch .. " -x c++ -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cxx $out")
		p.w("  depfile = $out.d")
		p.w("  deps = gcc")
		p.w("")
		p.w("rule cc_flags")
		p.w("  command = " .. cc .. " $CFLAGS".. force_include_pch .. " -x c -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cc $out")
		p.w("  depfile = $out.d")
		p.w("  deps = gcc")
		p.w("")
		p.w("rule cxx_flags")
		p.w("  command = " .. cxx .. " $CXXFLAGS" .. force_include_pch .. " -x c++ -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cxx $out")
		p.w("  deps = msvc")
		p.w("")
		if cfg.kind == p.STATICLIB then
			p.w("rule ar")
			p.w("  command = " .. ar .. " rcs $out $in")
			p.w("  description = ar $out")
			p.w("")
		else
			p.w("rule link")
			p.w("  command = " .. link .. " -o $out $in" .. ninja.list(ninja.shesc(toolset.getlinks(cfg, true))) .. all_ldflags)
			p.w("  description = link $out")
			p.w("")
		end
	elseif toolset_name == "gcc" then
		local force_include_pch = ""
		if pch then
			force_include_pch = " -include " .. p.esc(pch.placeholder)
			p.w("rule build_pch")
			p.w("  command = " .. iif(cfg.language == "C", cc .. all_cflags .. " -x c-header", cxx .. all_cxxflags .. " -x c++-header")  .. " -H -MMD -MF $out.d -c -o $out $in")
			p.w("  description = build_pch $out")
			p.w("  depfile = $out.d")
			p.w("  deps = gcc")
		end
		p.w("rule cc")
		p.w("  command = " .. cc .. all_cflags .. force_include_pch .. " -x c -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cc $out")
		p.w("  depfile = $out.d")
		p.w("  deps = gcc")
		p.w("")
		p.w("rule cxx")
		p.w("  command = " .. cxx .. all_cxxflags .. force_include_pch .. " -x c++ -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cxx $out")
		p.w("  depfile = $out.d")
		p.w("  deps = gcc")
		p.w("")
		p.w("rule cc_flags")
		p.w("  command = " .. cc .. " $CFLAGS".. force_include_pch .. " -x c -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cc $out")
		p.w("  depfile = $out.d")
		p.w("  deps = gcc")
		p.w("")
		p.w("rule cxx_flags")
		p.w("  command = " .. cxx .. " $CXXFLAGS" .. force_include_pch .. " -x c++ -MMD -MF $out.d -c -o $out $in")
		p.w("  description = cxx $out")
		p.w("  deps = msvc")
		p.w("")
		if cfg.kind == p.STATICLIB then
			p.w("rule ar")
			p.w("  command = " .. ar .. " rcs $out $in")
			p.w("  description = ar $out")
			p.w("")
		else
			p.w("rule link")
			p.w("  command = " .. link .. " -o $out $in" .. ninja.list(ninja.shesc(toolset.getlinks(cfg, true))) .. all_ldflags)
			p.w("  description = link $out")
			p.w("")
		end
	end
end

local function custom_command_rule()
	p.w("rule custom_command")
	p.w("  command = $CUSTOM_COMMAND")
	p.w("  description = $CUSTOM_DESCRIPTION")
	p.w("")
end

local function collect_generated_files(prj, cfg)
	local generated_files = {}
	tree.traverse(project.getsourcetree(prj), {
	onleaf = function(node, depth)
		function append_to_generated_files(filecfg)
			local output = project.getrelative(prj, filecfg.buildoutputs[1])
			table.insert(generated_files, p.esc(output))
		end
		local filecfg = fileconfig.getconfig(node, cfg)
		local rule = p.global.getRuleForFile(node.name, prj.rules)
		if fileconfig.hasCustomBuildRule(filecfg) then
			append_to_generated_files(filecfg)
		elseif rule then
			local environ = table.shallowcopy(filecfg.environ)

			if rule.propertydefinition then
				p.rule.prepareEnvironment(rule, environ, cfg)
				p.rule.prepareEnvironment(rule, environ, filecfg)
			end
			local rulecfg = p.context.extent(rule, environ)
			append_to_generated_files(rulecfg)
		end
	end,
	}, false, 1)
	return generated_files
end

local function pch_build(cfg, pch)
	local pch_dependency = ""
	if pch then
		pch_dependency = " | " .. pch.gch
		add_build(cfg, p.esc(pch.gch), "", "build_pch " .. p.esc(pch.input))
	end
	return pch_dependency
end

local function custom_command_build(prj, cfg, filecfg, filename, file_dependencies)
	local output = project.getrelative(prj, filecfg.buildoutputs[1])
	local inputs = ""
	if #filecfg.buildinputs > 0 then
		inputs = table.implode(filecfg.buildinputs," ","","")
	end

	local commands = {}
	if filecfg.buildmessage then
		commands = {os.translateCommandsAndPaths("{ECHO} " .. filecfg.buildmessage, prj.basedir, prj.location)}
	end
	commands = table.join(commands, os.translateCommandsAndPaths(filecfg.buildcommands, prj.basedir, prj.location))
	if (#commands > 1) then
		commands = 'sh -c ' .. ninja.quote(table.implode(commands,"","",";"))
	else
		commands = commands[1]
	end

	add_build(cfg, p.esc(output), "", "custom_command | " .. p.esc(filename) .. inputs .. iif(#file_dependencies > 0, "||" .. ninja.list(file_dependencies), ""),
		{"CUSTOM_COMMAND = " .. commands, "CUSTOM_DESCRIPTION = custom build " .. p.esc(output)})
end

local function compile_file_build(cfg, filecfg, toolset, pch_dependency, regular_file_dependencies, objfiles)
	local obj_dir = project.getrelative(cfg.workspace, cfg.objdir)
	local has_custom_settings = fileconfig.hasFileSettings(filecfg)

	if shouldcompileasc(filecfg) then
		local objfilename = obj_dir .. "/" .. filecfg.objname .. iif(toolset_name == "msc", ".obj", ".o")
		objfiles[#objfiles + 1] = objfilename
		local cflags = {}
		if has_custom_settings then
			cflags = {"CFLAGS = " .. getcflags(toolset, cfg, filecfg)}
		end
		add_build(cfg, p.esc(objfilename), "", iif(has_custom_settings, "cc_flags ", "cc ") .. p.esc(filecfg.relpath) .. pch_dependency .. regular_file_dependencies, cflags)
	elseif shouldcompileascpp(filecfg) then
		local objfilename = obj_dir .. "/" .. filecfg.objname .. iif(toolset_name == "msc", ".obj", ".o")
		objfiles[#objfiles + 1] = objfilename
		local cxxflags = {}
		if has_custom_settings then
			cxxflags = {"CXXFLAGS = " .. getcxxflags(toolset, cfg, filecfg)}
		end
		add_build(cfg, p.esc(objfilename), "", iif(has_custom_settings, "cxx_flags ", "cxx ") .. p.esc(filecfg.relpath) .. pch_dependency .. regular_file_dependencies, cxxflags)
	elseif path.isresourcefile(filecfg.abspath) then
		local objfilename = obj_dir .. "/" .. filecfg.name .. ".res"
		objfiles[#objfiles + 1] = objfilename
		add_build(cfg, p.esc(objfilename), "", "rc " .. p.esc(filecfg.relpath))
	end
end

local function files_build(prj, cfg, toolset, toolset_name, pch_dependency, regular_file_dependencies, file_dependencies)
	local objfiles = {}
	tree.traverse(project.getsourcetree(prj), {
	onleaf = function(node, depth)
		local filecfg = fileconfig.getconfig(node, cfg)
		local rule = p.global.getRuleForFile(node.name, prj.rules)
		if fileconfig.hasCustomBuildRule(filecfg) then
			custom_command_build(prj, cfg, filecfg, node.relpath, file_dependencies)
		elseif rule then
			local environ = table.shallowcopy(filecfg.environ)

			if rule.propertydefinition then
				p.rule.prepareEnvironment(rule, environ, cfg)
				p.rule.prepareEnvironment(rule, environ, filecfg)
			end
			local rulecfg = p.context.extent(rule, environ)
			custom_command_build(prj, cfg, rulecfg, node.relpath, file_dependencies)
		else
			compile_file_build(cfg, filecfg, toolset, pch_dependency, regular_file_dependencies, objfiles)
		end
	end,
	}, false, 1)
	p.w("")

	return objfiles
end

local function generated_files_build(cfg, generated_files, key)
	local final_dependency = ""
	if #generated_files > 0 then
		p.w("# generated files")
		add_build(cfg, "generated_files_" .. key, "", "phony" .. ninja.list(generated_files))
		final_dependency = " || generated_files_" .. key
	end
	return final_dependency
end

-- generate project + config build file
function ninja.generateProjectCfg(cfg)
	local oldGetDefaultSeparator = path.getDefaultSeparator
	path.getDefaultSeparator = function() return "/" end

	local prj = cfg.project
	local key = prj.name .. "_" .. cfg.buildcfg
	-- TODO why premake doesn't provide default name always ?
	local toolset_name = _OPTIONS.cc or cfg.toolset or ninja.getDefaultToolsetFromOs()
	local toolset = p.tools[toolset_name]

	p.w("# project build file")
	p.w("# generated with premake ninja")
	p.w("")

	-- premake-ninja relies on scoped rules
	-- and they were added in ninja v1.6
	p.w("ninja_required_version = 1.6")
	p.w("")

	---------------------------------------------------- figure out settings
	local pch = nil
	if toolset_name ~= "msc" then
		pch = p.tools.gcc.getpch(cfg)
		if pch then
			pch = {
				input = pch,
				placeholder = project.getrelative(cfg.workspace, path.join(cfg.objdir, path.getname(pch))),
				gch = project.getrelative(cfg.workspace, path.join(cfg.objdir, path.getname(pch) .. ".gch"))
			}
		end
	end

	---------------------------------------------------- write rules
	p.w("# core rules for " .. cfg.name)
	prebuild_rule(cfg)
	postbuild_rule(cfg)
	compilation_rules(cfg, toolset, toolset_name, pch)
	custom_command_rule()

	---------------------------------------------------- build all files
	p.w("# build files")

	local pch_dependency = pch_build(cfg, pch)

	local generated_files = collect_generated_files(prj, cfg)
	local file_dependencies = getFileDependencies(cfg)
	local regular_file_dependencies = ""
	if #generated_files > 0 then
		regular_file_dependencies = " || generated_files_" .. key .. ninja.list(file_dependencies)
	elseif #file_dependencies > 0 then
		regular_file_dependencies = " ||" .. ninja.list(file_dependencies)
	end

	local obj_dir = project.getrelative(cfg.workspace, cfg.objdir)
	local objfiles = files_build(prj, cfg, toolset, toolset_name, pch_dependency, regular_file_dependencies, file_dependencies)
	local final_dependency = generated_files_build(cfg, generated_files, key)

	---------------------------------------------------- build final target
	if #cfg.prebuildcommands > 0 or cfg.prebuildmessage then
		p.w("# prebuild")
		add_build(cfg, "prebuild_" .. get_key(cfg), "", "run_prebuild")
	end
	if #cfg.postbuildcommands > 0 or cfg.postbuildmessage then
		p.w("# postbuild")
		add_build(cfg, "postbuild_" .. get_key(cfg), "", "run_postbuild | " .. ninja.outputFilename(cfg))
	end

	-- we don't pass getlinks(cfg) through dependencies
	-- because system libraries are often not in PATH so ninja can't find them
	local libs = ninja.list(p.esc(config.getlinks(cfg, "siblings", "fullpath")))
	if cfg.kind == p.STATICLIB then
		p.w("# link static lib")
		add_build(cfg, p.esc(ninja.outputFilename(cfg)), "", "ar " .. table.concat(p.esc(objfiles), " ") .. libs .. final_dependency)

	elseif cfg.kind == p.SHAREDLIB then
		local output = ninja.outputFilename(cfg)
		p.w("# link shared lib")

		local extra_output = ""
		if ninja.endsWith(output, ".dll") then
			extra_output = " | " .. p.esc(ninja.noext(output, ".dll")) .. ".lib" .. " " .. p.esc(ninja.noext(output, ".dll")) .. ".exp"
		elseif ninja.endsWith(output, ".so") then
			extra_output = " | " .. p.esc(ninja.noext(output, ".so")) .. ".a"
		elseif ninja.endsWith(output, ".dylib") then
			-- in case of .dylib there are no corresponding .a file
		else
			p.error("unknown type of shared lib '" .. output .. "', so no idea what to do, sorry")
		end

		add_build(cfg, p.esc(output), extra_output, "link " .. table.concat(p.esc(objfiles), " ") .. libs .. final_dependency)

	elseif (cfg.kind == p.CONSOLEAPP) or (cfg.kind == p.WINDOWEDAPP) then
		p.w("# link executable")
		add_build(cfg, p.esc(ninja.outputFilename(cfg)), "", "link " .. table.concat(p.esc(objfiles), " ") .. libs .. final_dependency)

	else
		p.error("ninja action doesn't support this kind of target " .. cfg.kind)
	end

	p.w("")
	if #cfg.postbuildcommands > 0 or cfg.postbuildmessage then
		add_build(cfg, key, "", "phony postbuild_" .. get_key(cfg))
	else
		add_build(cfg, key, "", "phony " .. ninja.outputFilename(cfg))
	end
	p.w("")

	path.getDefaultSeparator = oldGetDefaultSeparator
end

-- return name of output binary relative to build folder
function ninja.outputFilename(cfg)
	return project.getrelative(cfg.workspace, cfg.buildtarget.directory) .. "/" .. cfg.buildtarget.name
end

-- return name of build file for configuration
function ninja.projectCfgFilename(cfg, relative)
	if relative ~= nil then
		relative = project.getrelative(cfg.workspace, cfg.location) .. "/"
	else
		relative = ""
	end

	local ninjapath = relative .. "build_" .. cfg.project.name  .. "_" .. cfg.buildcfg

	if cfg.platform ~= nil then ninjapath = ninjapath .. "_" .. cfg.platform end

	return ninjapath .. ".ninja"
end

-- check if string starts with string
function ninja.startsWith(str, starts)
	return str:sub(0, starts:len()) == starts
end

-- check if string ends with string
function ninja.endsWith(str, ends)
	return str:sub(-ends:len()) == ends
end

-- removes extension from string
function ninja.noext(str, ext)
	return str:sub(0, str:len() - ext:len())
end

-- generate all build files for every project configuration
function ninja.generateProject(prj)
	if not p.action.supports(prj.kind) or prj.kind == p.NONE then
		return
	end
	for cfg in project.eachconfig(prj) do
		p.generate(cfg, ninja.projectCfgFilename(cfg), ninja.generateProjectCfg)
	end
end

include("_preload.lua")

return ninja
