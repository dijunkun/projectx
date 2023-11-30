set_project("projectx")
set_version("0.0.1")
set_license("LGPL-3.0")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

set_installdir("$(projectdir)/out")

add_defines("ASIO_STANDALONE", "ASIO_HAS_STD_TYPE_TRAITS", "ASIO_HAS_STD_SHARED_PTR", 
    "ASIO_HAS_STD_ADDRESSOF", "ASIO_HAS_STD_ATOMIC", "ASIO_HAS_STD_CHRONO", 
    "ASIO_HAS_CSTDINT", "ASIO_HAS_STD_ARRAY",  "ASIO_HAS_STD_SYSTEM_ERROR")

add_requires("asio 1.24.0", "nlohmann_json", "spdlog 1.11.0", "openfec", "libopus 1.4")
add_packages("asio", "nlohmann_json", "spdlog", "openfec", "libopus")

includes("thirdparty")

if is_os("windows") then
    add_requires("vcpkg::ffmpeg 5.1.2", {configs = {shared = false}})
    add_requires("vcpkg::libnice 0.1.21")
    add_requires("openh264 2.1.1", {configs = {shared = false}})
    add_packages("vcpkg::ffmpeg", "vcpkg::libnice", "openh264", "cuda")
    add_defines("_WEBSOCKETPP_CPP11_INTERNAL_")
    add_requires("cuda")
elseif is_os("linux") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_requires("glib", {system = true})
    add_requires("vcpkg::libnice 0.1.21")
    add_requires("openh264 2.1.1", {configs = {shared = false}})
    add_packages("ffmpeg", "glib", "vcpkg::libnice", "openh264", "cuda")
    add_cxflags("-fPIC") 
    add_syslinks("pthread")
elseif is_os("macosx") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_requires("brew::libnice", "brew::glib")
    add_requires("brew::openh264", {configs = {shared = false}})
    add_packages("ffmpeg", "glib", "libnice")
    add_ldflags("-Wl,-ld_classic")
end

target("log")
    set_kind("object")
    add_files("src/log/log.cpp")
    add_includedirs("src/log", {public = true})

target("common")
    set_kind("object")
    add_files("src/common/common.cpp")
    add_includedirs("src/common", {public = true})

target("inih")
    set_kind("object")
    add_files("src/inih/ini.c", "src/inih/INIReader.cpp")
    add_includedirs("src/inih", {public = true})

target("ringbuffer")
    set_kind("object")
    add_files("src/ringbuffer/ringbuffer.cpp")
    add_includedirs("src/ringbuffer", {public = true})

target("thread")
    set_kind("object")
    add_deps("log")
    add_files("src/thread/*.cpp")
    add_includedirs("src/thread", {public = true})

target("frame")
    set_kind("object")
    add_files("src/frame/*.cpp")
    add_includedirs("src/frame", {public = true})

target("fec")
    set_kind("object")
    add_deps("log")
    add_files("src/fec/*.cpp")
    add_includedirs("src/fec", {public = true})

target("rtcp")
    set_kind("object")
    add_deps("log")
    add_files("src/rtcp/*.cpp")
    add_includedirs("src/rtcp", {public = true})

target("rtp")
    set_kind("object")
    add_deps("log", "frame", "ringbuffer", "thread", "rtcp", "fec")
    add_files("src/rtp/*.cpp")
    add_includedirs("src/rtp", {public = true})

target("ice")
    set_kind("object")
    add_deps("log", "common", "ws")
    add_files("src/ice/*.cpp")
    add_includedirs("src/ws", "src/ice", {public = true})
    if is_os("windows") then
        add_includedirs(path.join(os.getenv("VCPKG_ROOT"), 
            "installed/x64-windows-static/include/glib-2.0"), {public = true})
        add_includedirs(path.join(os.getenv("VCPKG_ROOT"), 
            "installed/x64-windows-static/lib/glib-2.0/include"), {public = true})
    elseif is_os("macosx") then
        add_includedirs(path.join("$(shell brew --cellar)",
            "glib/2.78.0/include/glib-2.0"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)",
            "glib/2.78.0/lib/glib-2.0/include"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)",
            "glib/2.78.0/lib/glib-2.0/include"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)",
            "glib/2.78.0/include"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)",
            "libnice/0.1.21/include"), {public = true})
    end
    
target("ws")
    set_kind("object")
    add_deps("log")
    add_files("src/ws/*.cpp")
    add_includedirs("thirdparty/websocketpp/include", {public = true})

target("media")
    set_kind("object")
    add_deps("log", "frame")
    if is_os("windows") then
        add_files("src/media/video/encode/*.cpp",
        "src/media/video/decode/*.cpp",
        "src/media/video/encode/nvcodec/*.cpp",
        "src/media/video/decode/nvcodec/*.cpp",
        "src/media/video/encode/ffmpeg/*.cpp",
        "src/media/video/decode/ffmpeg/*.cpp",
        "src/media/video/encode/openh264/*.cpp",
        "src/media/video/decode/openh264/*.cpp")
        add_includedirs("src/media/video/encode",
        "src/media/video/decode",
        "src/media/video/encode/nvcodec",
        "src/media/video/decode/nvcodec",
        "src/media/video/encode/ffmpeg",
        "src/media/video/decode/ffmpeg",
        "src/media/video/encode/openh264",
        "src/media/video/decode/openh264",
        "thirdparty/nvcodec/Interface",
        "thirdparty/nvcodec/Samples", {public = true})
    elseif is_os(("linux")) then
        add_files("src/media/video/encode/*.cpp",
        "src/media/video/decode/*.cpp",
        "src/media/video/encode/nvcodec/*.cpp",
        "src/media/video/decode/nvcodec/*.cpp",
        "src/media/video/encode/ffmpeg/*.cpp",
        "src/media/video/decode/ffmpeg/*.cpp",
        "src/media/video/encode/openh264/*.cpp",
        "src/media/video/decode/openh264/*.cpp")
        add_includedirs("src/media/video/encode",
        "src/media/video/decode",
        "src/media/video/encode/nvcodec",
        "src/media/video/decode/nvcodec",
        "src/media/video/encode/ffmpeg",
        "src/media/video/decode/ffmpeg",
        "src/media/video/encode/openh264",
        "src/media/video/decode/openh264",
        "thirdparty/nvcodec/Interface",
        "thirdparty/nvcodec/Samples", {public = true})
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
        add_files("src/media/audio/encode/*.cpp",
        "src/media/audio/decode/*.cpp")
    add_includedirs("src/media/audio/encode",
        "src/media/audio/decode", {public = true})

target("qos")
    set_kind("object")
    add_deps("log")
    add_files("src/qos/kcp/*.c")
    add_includedirs("src/qos/kcp", {public = true})

target("transmission")
    set_kind("object")
    add_deps("log", "ws", "ice", "qos", "rtp", "rtcp")
    add_files("src/transmission/*.cpp")
    add_includedirs("src/ws", "src/ice", "src/qos", {public = true})

target("pc")
    set_kind("object")
    add_deps("log")
    add_deps("ws", "ice", "transmission", "inih", "common", "media")
    add_files("src/pc/*.cpp")
    add_includedirs("src/transmission", "src/interface", {public = true})

target("projectx")
    set_kind("shared")
    add_deps("log", "pc")
    add_files("src/rtc/*.cpp")
    add_includedirs("src/rtc", "src/pc", "src/interface")

    if is_os("windows") then
        add_linkdirs("thirdparty/nvcodec/Lib/x64")
        add_links("nice", "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0",
        "pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix", 
        "zlib", "ffi", "libcrypto", "libssl", "intl", "iconv", "charset", "bz2",
        "Shell32", "Advapi32", "Dnsapi", "Shlwapi", "Iphlpapi",
        "cuda", "nvencodeapi", "nvcuvid",
        "ws2_32", "Bcrypt", "windowsapp", "User32", "Strmiids", "Mfuuid",
        "Secur32", "Bcrypt")
    elseif is_os(("linux")) then
        add_linkdirs("thirdparty/nvcodec/Lib/x64")
        add_links("cuda", "nvidia-encode", "nvcuvid")
    elseif is_os("macosx") then
        add_linkdirs(path.join("$(shell brew --cellar)", "glib/2.78.0/lib"))
        add_linkdirs(path.join("$(shell brew --cellar)", "libnice/0.1.21/lib"))
        add_links("nice", "glib-2.0", "gio-2.0", "gobject-2.0")
    end

    add_installfiles("src/interface/*.h", {prefixdir = "include"})
    add_rules("utils.symbols.export_list", {symbols = {
        "CreatePeer",
        "Init",
        "CreateConnection",
        "JoinConnection",
        "LeaveConnection",
        "SendData"}})


    -- add_rules("utils.symbols.export_all", {export_classes = true})
-- after_install(function (target)
--     os.rm("$(projectdir)/out/lib/*.a")
--     os.rm("$(projectdir)/out/include/log.h")
-- end)

-- target("host")
--     set_kind("binary")
--     add_deps("projectx")
--     add_files("tests/peerconnection/host.cpp")
--     add_includedirs("src/interface")

-- target("guest")
--     set_kind("binary")
--     add_deps("projectx")
--     add_files("tests/peerconnection/guest.cpp")
--     add_includedirs("src/interface")

-- target("nicetest")
--     set_kind("binary")
--     add_files("tests/peerconnection/nice.cpp")
--     add_includedirs("E:/SourceCode/vcpkg/installed/x64-windows-static/include/glib-2.0")
--     add_includedirs("E:/SourceCode/vcpkg/installed/x64-windows-static/lib/glib-2.0/include")
--     add_linkdirs("E:/SourceCode/vcpkg/installed/x64-windows-static/lib")
--     add_links("nice", "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0",
--     "pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix", 
--     "zlib", "ffi", "libcrypto", "libssl", "intl", "iconv", "charset", "bz2",
--     "Shell32", "Advapi32", "Dnsapi", "Shlwapi", "Iphlpapi")

-- target("fec_client")
--     set_kind("binary")
--     add_packages("openfec")
--     add_files("tests/fec/simple_client.cpp")
--     add_includedirs("tests/fec")

-- target("fec_server")
--     set_kind("binary")
--     add_packages("openfec")
--     add_files("tests/fec/simple_server.cpp")
--     add_includedirs("tests/fec")

-- target("opus_test")
--     set_kind("binary")
--     add_packages("libopus", "sdl2")
--     add_files("tests/opus/OpusEncoderImpl.cpp",
--         "tests/opus/OpusDecoderImpl.cpp",
--         "tests/opus/main.cpp")
--     add_includedirs("tests/opus")