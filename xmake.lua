set_project("projectx")
set_version("0.0.1")
set_license("GPL-3.0")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

add_rules("mode.release", "mode.debug")

add_requires("asio 1.24.0", "nlohmann_json", "spdlog 1.11.0")
add_requires("libjuice", {system = false})

if is_os("windows") then
    add_requires("vcpkg::ffmpeg 5.1.2", {configs = {shared = false}})
    add_packages("vcpkg::ffmpeg")
elseif is_os("linux") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_packages("ffmpeg")
elseif is_os("macosx") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_packages("ffmpeg")
end

add_defines("JUICE_STATIC")
add_defines("ASIO_STANDALONE", "ASIO_HAS_STD_TYPE_TRAITS", "ASIO_HAS_STD_SHARED_PTR", 
    "ASIO_HAS_STD_ADDRESSOF", "ASIO_HAS_STD_ATOMIC", "ASIO_HAS_STD_CHRONO", "ASIO_HAS_CSTDINT", "ASIO_HAS_STD_ARRAY",
    "ASIO_HAS_STD_SYSTEM_ERROR")

if is_os("windows") then
    add_defines("_WEBSOCKETPP_CPP11_INTERNAL_")
    add_links("ws2_32", "Bcrypt")
    add_links("windowsapp", "User32", "Strmiids", "Mfuuid", "Secur32", "Bcrypt")
    add_requires("cuda")
elseif is_os("linux") then 
    set_config("cflags", "-fPIC")
    add_syslinks("pthread")
end

add_packages("spdlog")

includes("thirdparty")
includes("application/remote_desk")
includes("application/signal_server")

target("log")
    set_kind("headeronly")
    add_packages("spdlog")
    add_headerfiles("src/log/log.h")
    add_includedirs("src/log", {public = true})

target("common")
    set_kind("headeronly")
    add_includedirs("src/common", {public = true})

target("inih")
    set_kind("static")
    add_files("src/inih/ini.c", "src/inih/INIReader.cpp")
    add_includedirs("src/inih", {public = true})

target("ringbuffer")
    set_kind("headeronly")
    add_includedirs("src/ringbuffer", {public = true})

target("thread")
    set_kind("static")
    add_deps("log")
    add_files("src/thread/*.cpp")
    add_includedirs("src/thread", {public = true})

target("frame")
    set_kind("static")
    add_files("src/frame/*.cpp")
    add_includedirs("src/frame", {public = true})

target("rtcp")
    set_kind("static")
    add_deps("log")
    add_files("src/rtcp/*.cpp")
    add_includedirs("src/rtcp", {public = true})

target("rtp")
    set_kind("static")
    add_deps("log", "frame", "ringbuffer", "thread", "rtcp")
    add_files("src/rtp/*.cpp")
    add_includedirs("src/rtp", {public = true})

target("ice")
    set_kind("static")
    add_deps("log", "common", "ws")
    add_packages("asio", "nlohmann_json", "libjuice")
    add_files("src/ice/*.cpp")
    add_includedirs("src/ws", {public = true})
    add_includedirs("thirdparty/libjuice/include", {public = true})

target("ws")
    set_kind("static")
    add_deps("log")
    add_files("src/ws/*.cpp")
    add_packages("asio")
    add_includedirs("thirdparty/websocketpp/include", {public = true})

target("media")
    set_kind("static")
    add_deps("log", "frame")
    if is_os("windows") then
        add_packages("cuda")
        add_files("src/media/video/encode/*.cpp",
        "src/media/video/decode/*.cpp",
        "src/media/video/encode/nvcodec/*.cpp",
        "src/media/video/decode/nvcodec/*.cpp",
        "src/media/video/encode/ffmpeg/*.cpp",
        "src/media/video/decode/ffmpeg/*.cpp")
        add_includedirs("src/media/video/encode",
        "src/media/video/decode",
        "src/media/video/encode/nvcodec",
        "src/media/video/decode/nvcodec",
        "src/media/video/encode/ffmpeg",
        "src/media/video/decode/ffmpeg",
        "thirdparty/nvcodec/Interface",
        "thirdparty/nvcodec/Samples", {public = true})
        add_linkdirs("thirdparty/nvcodec/Lib/x64")
        add_links("cuda", "nvencodeapi", "nvcuvid")
    elseif is_os(("linux")) then
        add_packages("cuda")
        add_files("src/media/video/encode/*.cpp",
        "src/media/video/decode/*.cpp",
        "src/media/video/encode/nvcodec/*.cpp",
        "src/media/video/decode/nvcodec/*.cpp",
        "src/media/video/encode/ffmpeg/*.cpp",
        "src/media/video/decode/ffmpeg/*.cpp")
        add_includedirs("src/media/video/encode",
        "src/media/video/decode",
        "src/media/video/encode/nvcodec",
        "src/media/video/decode/nvcodec",
        "src/media/video/encode/ffmpeg",
        "src/media/video/decode/ffmpeg",
        "thirdparty/nvcodec/Interface",
        "thirdparty/nvcodec/Samples", {public = true})
        add_linkdirs("thirdparty/nvcodec/Lib/x64")
        add_links("cuda", "nvidia-encode", "nvcuvid")
    elseif is_os("macosx") then
        add_files("src/media/video/encode/*.cpp",
        "src/media/video/decode/*.cpp",
        "src/media/video/encode/ffmpeg/*.cpp",
        "src/media/video/decode/ffmpeg/*.cpp")
        add_includedirs("src/media/video/encode",
        "src/media/video/decode",
        "src/media/video/encode/ffmpeg",
        "src/media/video/decode/ffmpeg", {public = true})
    end

target("qos")
    set_kind("static")
    add_deps("log")
    add_files("src/qos/kcp/*.c")
    add_includedirs("src/qos/kcp", {public = true})

target("transmission")
    set_kind("static")
    add_deps("log", "ws", "ice", "qos", "rtp", "rtcp")
    add_files("src/transmission/*.cpp")
    add_packages("asio", "nlohmann_json")
    add_includedirs("src/ws", "src/ice", "src/qos", {public = true})

target("pc")
    set_kind("static")
    add_deps("log")
    add_deps("ws", "ice", "transmission", "inih", "common", "media")
    add_files("src/pc/*.cpp")
    add_packages("asio", "nlohmann_json", "cuda")
    add_includedirs("src/transmission", {public = true})


target("projectx")
    set_kind("shared")
    add_deps("log")
    add_deps("pc")
    add_files("src/rtc/*.cpp")
    add_packages("asio", "nlohmann_json", "cuda")
    add_includedirs("src/rtc", "src/pc", "src/interface")
    add_rules("utils.symbols.export_all", {export_classes = true})
    -- set_policy("build.merge_archive", true)
    -- set_targetdir("$(projectdir)/libdrtc/lib")

target("host")
    set_kind("binary")
    add_deps("projectx")
    add_files("tests/peerconnection/host.cpp")
    add_includedirs("src/interface")

target("guest")
    set_kind("binary")
    add_deps("projectx")
    add_files("tests/peerconnection/guest.cpp")
    add_includedirs("src/interface")