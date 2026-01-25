target("ZeytinEditor")
    set_kind("binary")
    set_policy("build.c++.modules", true)
    set_languages("c++latest")

    add_deps("ZeytinCommon")

    add_files("source/**/*.cpp")
    add_files("source/**/*.cppm")

    add_files("3rdparty/imgui/*.cpp", {cxxflags = {"-w"}})
    add_files("3rdparty/imgui_test_engine/*.cpp", {cxxflags = {"-w"}})
    add_files("3rdparty/rlimgui/*.cpp", {cxxflags = {"-w"}})
    add_files("3rdparty/backward-cpp/backward.cpp", {cxxflags = {"-w"}})

    add_includedirs(
        "3rdparty",
        "3rdparty/imgui",
        "3rdparty/zmq",
        "3rdparty/rapidjson",
        "3rdparty/raylib",
        "3rdparty/imgui_test_engine"
    )

    add_linkdirs("3rdparty/raylib/lib/linux", "3rdparty/zmq/linux")

    add_cxflags("-Wall", "-Wextra", "-Wpedantic", "-Werror", "-g", "-O0", "-Wno-deprecated-literal-operator")

    add_defines(
        "NO_FONT_AWESOME",
        "DEBUG=1",
        "IMGUI_ENABLE_TEST_ENGINE",
        "IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1"
    )

    add_links("raylib", "zmq")
    add_syslinks("m", "pthread", "dl", "rt", "X11", "asound")

