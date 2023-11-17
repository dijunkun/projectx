set_project("remote_desk")
set_version("0.0.1")
set_license("LGPL-3.0")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

add_requires("spdlog 1.11.0", {system = false})
add_requires("imgui 1.89.9", {configs = {sdl2 = true, sdl2_renderer = true}})
add_defines("UNICODE")

add_requires("sdl2", {system = false})

if is_os("windows") then
    add_links("Shell32", "windowsapp", "dwmapi", "User32", "kernel32")
    add_requires("vcpkg::ffmpeg 5.1.2", {configs = {shared = false}})
elseif is_os("linux") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_syslinks("pthread", "dl")
elseif is_os("macosx") then
    add_requires("ffmpeg 5.1.2", {system = false})
end

if is_mode("debug") then
    add_defines("REMOTE_DESK_DEBUG")
end

add_packages("spdlog")

includes("thirdparty")

target("log")
    set_kind("headeronly")
    add_packages("spdlog")
    add_headerfiles("../utils/log/log.h")
    add_includedirs("../utils/log", {public = true})

target("screen_capture")
    set_kind("static")
    add_packages("log", "ffmpeg")
    if is_os("windows") then
        add_files("screen_capture/windows/*.cpp")
        add_includedirs("screen_capture/windows", {public = true})
    elseif is_os("macosx") then
         add_files("screen_capture/macosx/*.cpp")
         add_includedirs("screen_capture/macosx", {public = true})
    elseif is_os("linux") then
         add_files("screen_capture/linux/*.cpp")
         add_includedirs("screen_capture/linux", {public = true})
    end

target("remote_desk")
    set_kind("binary")
    add_deps("projectx", "screen_capture")
    add_packages("log", "sdl2", "imgui", "ffmpeg")
    add_files("remote_desk_gui/main.cpp")
    add_includedirs("../../src/interface")
    if is_os("windows") then
        add_links("SDL2-static", "SDL2main", "gdi32", "winmm", 
        "setupapi", "version", "Imm32", "iphlpapi")
    elseif is_os("macosx") then
        add_links("SDL2")
    elseif is_os("linux") then
        add_links("SDL2")
        add_ldflags("-lavformat", "-lavdevice", "-lavfilter", "-lavcodec",
        "-lswscale", "-lavutil", "-lswresample",
        "-lasound", "-lxcb-shape", "-lxcb-xfixes", "-lsndio", "-lxcb", 
        "-lxcb-shm", "-lXext", "-lX11", "-lXv", "-ldl", "-lpthread", 
        {force = true})
    end
    after_install(function (target)
        os.cp("$(projectdir)/thirdparty/nvcodec/Lib/x64/*.so", "$(projectdir)/out/bin")
        os.cp("$(projectdir)/thirdparty/nvcodec/Lib/x64/*.so.1", "$(projectdir)/out/bin")
        os.cp("$(projectdir)/out/lib/*.so", "$(projectdir)/out/bin")
        os.rm("$(projectdir)/out/include")
        os.rm("$(projectdir)/out/lib")
    end)

-- target("linux_capture")
--     set_kind("binary")
--     add_packages("sdl2", "imgui",  "ffmpeg", "openh264")
--     add_files("remote_desk_gui/linux_capture.cpp")
--     add_ldflags("-lavformat", "-lavdevice", "-lavfilter", "-lavcodec",
--     "-lswscale", "-lavutil", "-lswresample",
--     "-lasound", "-lxcb-shape", "-lxcb-xfixes", "-lsndio", "-lxcb", 
--     "-lxcb-shm", "-lXext", "-lX11", "-lXv", "-lpthread", "-lSDL2", "-lopenh264",
--     "-ldl" ,{force = true})