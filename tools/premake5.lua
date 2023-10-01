require "ninja"

-- workspace

workspace "project"
    configurations { "devel", "debug", "release", "retail" }
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
        debugdir "../"
        symbols "On"
        optimize "Off"
        kind "consoleApp" -- "WindowedApp"
    filter "configurations:devel"
        debugdir "../"
        symbols "On"
        optimize "On"
        kind "consoleApp" -- "WindowedApp"
    filter "configurations:release"
        debugdir "../"
        defines {"NDEBUG"}
        symbols "Off"
        optimize "On"
        staticruntime "on"
        runtime "Release"
        kind "consoleApp" -- "WindowedApp"
    filter "configurations:retail"
        debugdir "../"
        defines {"NDEBUG","ENABLE_RETAIL","main=WinMain"}
        symbols "Off"
        optimize "On"
        staticruntime "on"
        runtime "Release"
        kind "WindowedApp"

-- dlls

project "engine"
    language "C"
    kind "SharedLib"
    files {"../engine/fwk.c","../engine/fwk.h"}
    includedirs {"../engine/"}
    defines {"API=EXPORT"}

-- exes

project "editor"
    language "C"
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

function demos(static,...)
    for _, name in ipairs({...}) do
        project (name)
            -- defaults()
            uuid (os.uuid(name))
            language "C"
            includedirs {"../engine/", "../tools/editor/"}
            links {"engine"} defines {"API="..static} -- kind "SharedLib" links {"engine"}
            files {
                "../demos/*"..name.."*.h*",
                "../demos/*"..name.."*.c*",
                "../demos/*"..name.."*.inl",
            }
    end
end

demos("IMPORT",
  "00-loop",    "00-script",   "01-demo2d",     "01-easing",  "01-font",      "01-ui",
  "02-ddraw",   "02-frustum",  "03-anims",      "04-actor",   "06-material",  "06-scene",
  "07-netsync", "07-network",  "08-audio",      "08-video",   "09-cubemap",   "09-shadertoy",
  "99-bt",      "99-compute",  "99-controller", "99-demo",    "99-geom",      "99-lod",
  "99-pathfind","99-pbr",      "99-spine",      "99-sponza",  "99-sprite"
)

demos("STATIC",
  "99-nodes"
)

-- games

function games(...)
    for _, name in ipairs({...}) do
        project ("game-" .. name)
            uuid (os.uuid("game-" .. name))
            language "C++"
            includedirs {"../engine/", "../tools/editor/"}
            links {"engine"} defines {"API=IMPORT"} -- kind "SharedLib" links {"engine"}
            files {
                "../games/"..name.."/**.h*",
                "../games/"..name.."/**.c*",
                "../games/"..name.."/**.inl",
            }
        -- defaults()
    end
end

-- games("untitled")
