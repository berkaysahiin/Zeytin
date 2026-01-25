target("ZeytinCommon")
    set_kind("shared")
    set_policy("build.c++.modules", true)
    set_languages("c++20")

    add_files("**/*.cpp")
    add_files("**/*.cppm", {public = true})

    add_includedirs(
        "../editor/3rdparty/",
        "../engine/3rdparty/"
    )

