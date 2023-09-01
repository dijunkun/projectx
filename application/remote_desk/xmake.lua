set_project("remote_desk")
set_version("0.0.1")
set_license("GPL-3.0")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

add_requires("spdlog 1.11.0", {system = false})
add_defines("UNICODE")

if is_os("windows") then
    add_ldflags("/SUBSYSTEM:CONSOLE")
    add_links("Shell32", "windowsapp", "dwmapi", "User32", "kernel32")
    add_requires("vcpkg::sdl2 2.26.4")
    add_requires("vcpkg::ffmpeg 5.1.2", {configs = {shared = false}})
elseif is_os("linux") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_links("pthread")
    set_config("cxxflags", "-fPIC")
end

add_packages("spdlog")

includes("../../thirdparty")

target("log")
    set_kind("headeronly")
    add_packages("spdlog")
    add_headerfiles("../../src/log/log.h")
    add_includedirs("../../src/log", {public = true})

target("screen_capture")
    set_kind("static")
    add_packages("log")
    add_files("screen_capture/*.cpp")
    add_includedirs("screen_capture", {public = true})

target("remote_desk_server")
    set_kind("binary")
    add_packages("log", "vcpkg::ffmpeg")
    add_deps("projectx", "screen_capture")
    add_files("remote_desk_server/*.cpp")
    add_includedirs("../../src/interface")
    add_links("swscale", "avutil")
    add_defines("WIN32_LEAN_AND_MEAN")
    -- if is_os("windows") then
    --     add_links("iphlpapi")
    --     add_includedirs("../../thirdparty/ffmpeg/include")
    --     if is_mode("debug") then
    --         add_linkdirs("../../thirdparty/ffmpeg/lib/debug/win")
    --     else
    --         add_linkdirs("../../thirdparty/ffmpeg/lib/release/win")
    --     end
    -- end 

target("remote_desk_client")
    set_kind("binary")
    add_deps("projectx")
    add_packages("log")
    add_packages("ffmpeg")
    add_packages("vcpkg::sdl2")
    add_files("remote_desk_client/*.cpp")
    add_includedirs("../../src/interface")
    if is_os("windows") then
        add_links("SDL2-static", "SDL2main", "gdi32", "winmm", 
        "setupapi", "version", "Imm32", "iphlpapi")
    end

-- target("remote_desk")
--     set_kind("binary")
--     add_deps("projectx")
--     add_packages("log")
--     add_packages("ffmpeg")
--     add_packages("vcpkg::sdl2")
--     add_links("avfilter", "avdevice", "avformat", "avcodec", "swscale", "swresample", "avutil")
--     add_files("**.cpp")
--     add_includedirs("../../src/interface")
--     add_links("SDL2-static", "SDL2main", "Shell32", "gdi32", "winmm", 
--         "setupapi", "version", "WindowsApp", "Imm32", "avutil")


