set_project("projectx")
set_version("0.0.1")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

add_rules("mode.release", "mode.debug")

add_requires("asio 1.24.0", "nlohmann_json", "spdlog 1.11.0")
add_requires("libjuice", {system = false})

add_defines("JUICE_STATIC")
add_defines("ASIO_STANDALONE", "ASIO_HAS_STD_TYPE_TRAITS", "ASIO_HAS_STD_SHARED_PTR", 
    "ASIO_HAS_STD_ADDRESSOF", "ASIO_HAS_STD_ATOMIC", "ASIO_HAS_STD_CHRONO", "ASIO_HAS_CSTDINT", "ASIO_HAS_STD_ARRAY",
    "ASIO_HAS_STD_SYSTEM_ERROR")

if is_os("windows") then
    add_defines("_WEBSOCKETPP_CPP11_INTERNAL_")
    add_links("ws2_32", "Bcrypt")
    add_requires("cuda")
elseif is_os("linux") then 
    add_links("pthread")
    set_config("cxxflags", "-fPIC")
end

add_packages("spdlog")

includes("thirdparty")
includes("application/remote_desk")

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
    add_deps("log")
    add_packages("cuda")
    add_links("cuda", "nvencodeapi", "nvcuvid")
    add_files("src/media/video/encode/nvcodec/*.cpp",
    "src/media/video/decode/nvcodec/*.cpp")
    add_includedirs("src/media/video/encode/nvcodec",
    "src/media/video/decode/nvcodec", 
    "thirdparty/nvcodec/Interface",
    "thirdparty/nvcodec/Samples", {public = true})
    add_linkdirs("thirdparty/nvcodec/Lib/x64")

target("qos")
    set_kind("static")
    add_deps("log")
    add_files("src/qos/kcp/*.c")
    add_includedirs("src/qos/kcp", {public = true})

target("transmission")
    set_kind("static")
    add_deps("log", "ws", "ice", "qos")
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

target("signal_server")
    set_kind("binary")
    add_deps("log", "common")
    add_files("tests/signal_server/*.cpp")
    add_packages("asio", "nlohmann_json", "spdlog")
    add_includedirs("thirdparty/websocketpp/include")

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