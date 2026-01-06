workspace "ZeytinEditor"
    configurations { "Debug" }
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "On"
    
    includedirs {
        "include",
        "3rdparty",
        "3rdparty/imgui",
        "3rdparty/zmq",
        "3rdparty/rapidjson",
        "3rdparty/raylib",
        "3rdparty/imgui_test_engine",
    }
    
    filter "system:windows"
        toolset "gcc"
        defines { "_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }
        links {
            "raylib", "winmm", "gdi32", "user32", "shell32", "libzmq"
        }
        libdirs {
            "3rdparty/raylib/lib/windows",
            "3rdparty/zmq/windows"
        }
        buildoptions { "-std=c++17", "-w", "-static-libgcc", "-static-libstdc++"}
        postbuildcommands {
            "cp ../3rdparty/zmq/windows/libzmq-mt-4_3_5.dll %{cfg.targetdir}"
        }
    
    filter "system:linux"
        toolset "gcc"
        links {
            "raylib", "m", "pthread", "dl", "rt", "X11", "asound", "stdc++", "zmq"
        }
        libdirs {
            "3rdparty/raylib/lib/linux",
            "3rdparty/zmq/linux"
        }
        files {
            "3rdparty/backward-cpp/backward.cpp",
        }
        buildoptions { "-std=c++17", "-w", "-fPIC" }
    
    filter {}
    
    project "ZeytinEditor"
        kind "ConsoleApp"
        language "C++"
        symbols "On"
        optimize "Off"
        
        files {
            "3rdparty/rlimgui/**.cpp",
            "3rdparty/imgui/**.cpp",
            "3rdparty/imgui_test_engine/**.cpp",
            "source/**.cpp"
        }
        
        defines {
            "DEBUG=1",
            "IMGUI_ENABLE_TEST_ENGINE",
            "IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1"
        }
