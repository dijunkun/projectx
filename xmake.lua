set_project("projectx")
set_version("0.0.1")
set_license("GPL-3.0")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

add_rules("mode.release", "mode.debug")

add_defines("ASIO_STANDALONE", "ASIO_HAS_STD_TYPE_TRAITS", "ASIO_HAS_STD_SHARED_PTR", 
    "ASIO_HAS_STD_ADDRESSOF", "ASIO_HAS_STD_ATOMIC", "ASIO_HAS_STD_CHRONO", 
    "ASIO_HAS_CSTDINT", "ASIO_HAS_STD_ARRAY",  "ASIO_HAS_STD_SYSTEM_ERROR")

if is_os("windows") then
    add_defines("_WEBSOCKETPP_CPP11_INTERNAL_")
    add_links("ws2_32", "Bcrypt")
    add_links("windowsapp", "User32", "Strmiids", "Mfuuid", "Secur32", "Bcrypt")
    add_requires("cuda")
elseif is_os("linux") then 
    set_config("cflags", "-fPIC")
    add_syslinks("pthread")
elseif is_os("macosx") then
    add_ldflags("-ld_classic", {force = true})
end

add_requires("asio 1.24.0", "nlohmann_json", "spdlog 1.11.0")

if is_os("windows") then
    add_requires("vcpkg::ffmpeg 5.1.2", {configs = {shared = false}})
    add_requires("vcpkg::libnice 0.1.21")
    add_packages("vcpkg::libnice")
elseif is_os("linux") then
    add_requireconfs("ffmpeg.x264", {configs = {pic = true}})
    add_requires("ffmpeg 5.1.2")
    add_requires("glib", {system = true})
    add_requires("vcpkg::libnice 0.1.21")
    add_packages("glib", "vcpkg::libnice")
elseif is_os("macosx") then
    add_requires("ffmpeg 5.1.2", {system = false})
    add_requires("brew::libnice", "brew::glib")
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
    add_packages("asio", "nlohmann_json")
    add_files("src/ice/*.cpp")
    add_includedirs("src/ws", "src/ice", {public = true})
    if is_os("windows") then
        add_includedirs(path.join(os.getenv("VCPKG_ROOT"), "installed/x64-windows-static/include/glib-2.0"), {public = true})
        add_includedirs(path.join(os.getenv("VCPKG_ROOT"), "installed/x64-windows-static/lib/glib-2.0/include"), {public = true})
        add_links("nice", "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0",
        "pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix", 
        "zlib", "ffi", "libcrypto", "libssl", "intl", "iconv", "charset", "bz2",
        "Shell32", "Advapi32", "Dnsapi", "Shlwapi", "Iphlpapi")
    elseif is_os("macosx") then
        add_packages("glib", "libnice")
        add_includedirs(path.join("$(shell brew --cellar)", "glib/2.78.0/include/glib-2.0"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)", "glib/2.78.0/lib/glib-2.0/include"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)", "glib/2.78.0/lib/glib-2.0/include"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)", "glib/2.78.0/include"), {public = true})
        add_includedirs(path.join("$(shell brew --cellar)", "libnice/0.1.21/include"), {public = true})
        add_linkdirs(path.join("$(shell brew --cellar)", "glib/2.78.0/lib"))
        add_linkdirs(path.join("$(shell brew --cellar)", "libnice/0.1.21/lib"))
        add_links("nice", "glib-2.0", "gio-2.0", "gobject-2.0")
    end
    

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
        add_packages("cuda", "vcpkg::ffmpeg")
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
        add_packages("cuda", "ffmpeg")
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
        add_packages("ffmpeg")
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
    add_includedirs("src/transmission", "src/interface", {public = true})


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

target("nicetest")
    set_kind("binary")
    add_files("tests/peerconnection/nice.cpp")
    add_includedirs("E:/SourceCode/vcpkg/installed/x64-windows-static/include/glib-2.0")
    add_includedirs("E:/SourceCode/vcpkg/installed/x64-windows-static/lib/glib-2.0/include")
    add_linkdirs("E:/SourceCode/vcpkg/installed/x64-windows-static/lib")
    add_links("nice", "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0",
        "pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix", 
        "zlib", "ffi", "libcrypto", "libssl", "intl", "iconv", "charset", "bz2",
        "Shell32", "Advapi32", "Dnsapi", "Shlwapi", "Iphlpapi")