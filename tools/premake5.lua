require "ninja"

-- workspace

workspace "project"
    configurations { "devel", "debug", "release" }
    location "../_project"
    targetdir "../_%{cfg.buildcfg}"
    configuration "windows"
        platforms { "x64" }
        buildoptions "/W1"
--        entrypoint "main"
        -- buildoptions { "/utf-8" } "/W3", "/wd4996", "/wd4244", "/wd4305", "/wd4267"} -- "/EHsc-"}
        -- links { "user32", "gdi32" }
        -- flags { "/MT" }
    configuration "linux"
        links { "pthread", "X11", "m", "dl" }
    filter "configurations:debug"
        symbols "On"
        optimize "Off"
    filter "configurations:devel"
        symbols "On"
        optimize "On"
    filter "configurations:release"
        defines {"NDEBUG"}
        symbols "Off"
        optimize "On"

-- dlls

project "engine"
    language "C"
    kind "SharedLib"
    files {"../engine/fwk.c"}
    includedirs {"../engine/"}
    defines {"API=EXPORT"}

-- exes

project "editor"
    language "C"
    kind "ConsoleApp" -- "WindowedApp"
    links {"engine"} defines {"API=IMPORT"}
    files {
    "../tools/editor/**.",
    "../tools/editor/**.h*",
    "../tools/editor/editor.c",
    }
    includedirs {
    "../tools/editor/",
    "../engine/",
    }

project "editor2"
    language "C"
    kind "ConsoleApp"
    files {
    "../tools/editor/**.",
    "../tools/editor/**.h*",
    "../tools/editor/editor2.c",
    }
    includedirs {
    "../tools/editor/",
    "../engine/",
    }

-- demos

function demos(...)
    for _, name in ipairs({...}) do
        project (name)
            -- defaults()
            uuid (os.uuid(name))
            language "C"
            kind "consoleApp" -- "WindowedApp"
            includedirs {"../engine/"}
            links {"engine"} defines {"API=IMPORT"} -- kind "SharedLib" links {"engine"}
            includedirs {"../engine/", "../tools/editor/"}
            files {
                "../demos/*"..name.."*.h*",
                "../demos/*"..name.."*.c*",
                "../demos/*"..name.."*.inl",
            }
    end
end

demos(
    "00-hello","01-sprite","02-ddraw","03-character","04-control",
    "04-scene","05-network","06-pbr"
)

-- games

function games(...)
    for _, name in ipairs({...}) do
        project ("game-" .. name)
            uuid (os.uuid("game-" .. name))
            language "C++"
            kind "consoleApp"
            includedirs {"../engine/"}
            links {"engine"} defines {"API=IMPORT"} -- kind "SharedLib" links {"engine"}
            includedirs {"../engine/", "../tools/editor/"}
            files {
                "../games/"..name.."/**.h*",
                "../games/"..name.."/**.c*",
                "../games/"..name.."/**.inl",
            }
        -- defaults()
    end
end

-- games("untitled")
