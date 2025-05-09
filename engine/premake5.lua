workspace "Zeytin"
    configurations { "EDITOR_MODE", "STANDALONE" }
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "On"

    includedirs {
        "include",
        "3rdparty",
        "3rdparty/zmq",
        "3rdparty/raylib",
        "3rdparty/rttr",
        "3rdparty/tracy",
    }

    filter "system:linux"
        toolset "clang"
        libdirs {
            "3rdparty/raylib/lib/linux",
            "3rdparty/rttr/lib/linux",
        }
        buildoptions {
            "-w",
            "-std=c++17",
            "-static-libstdc++"
        }

    filter { "system:linux", "configurations:EDITOR_MODE" }
        links {
            "raylib", "m", "pthread", "dl", "rt", "X11", "asound", "stdc++",
            "rttr_core", "zmq"
            , "dw", "bfd", "dwarf", "unwind" -- for better callstack, not included in windows
        }
        libdirs {
            "3rdparty/zmq/linux", 
        }
        files {
            "3rdparty/backward-cpp/backward.cpp", -- not included in windows 
            "3rdparty/tracy/TracyClient.cpp", -- not included in windows
        }
        defines {
            "PROFILE=1",
        }

    filter { "system:linux", "configurations:STANDALONE" }
        links {
            "raylib", "m", "pthread", "dl", "rt", "X11", "asound", "stdc++",
            "rttr_core"
        }

    filter "system:windows"
        toolset "gcc"
        libdirs {
            "3rdparty/raylib/lib/windows",
            "3rdparty/rttr/lib/windows",
        }
        buildoptions {
            "-std=c++17",
            "-w",
            "-static-libgcc",
            "-static-libstdc++"
        }

    filter { "system:windows", "configurations:STANDALONE" }
        links {
            "raylib", "rttr_core",
            "winmm", "gdi32", "user32", "shell32"
        }
    
    filter { "system:windows", "configurations:EDITOR_MODE" }
        links {
            "raylib", "rttr_core",
            "winmm", "gdi32", "user32", "shell32", "libzmq",
        }
        libdirs {
            "3rdparty/zmq/windows",
        }
        postbuildcommands {
            "cp ../3rdparty/zmq/windows/libzmq-mt-4_3_5.dll %{cfg.targetdir}"
        }

    filter {}

    project "Zeytin"
        kind "ConsoleApp"
        language "C++"
        files {
            "source/**.cpp"
        }

        filter "configurations:EDITOR_MODE"
            defines {
                "DEBUG=1",
                "EDITOR_MODE=1",
                "TRACY_ENABLE=1"
            }
            symbols "On"
            optimize "Off"

        filter "configurations:STANDALONE"
            defines { "TRACY_ENABLE=0", "STANDALONE=1" }
            symbols "On"
            optimize "Off"
