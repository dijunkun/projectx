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
elseif is_os("linux") then 
    add_links("pthread")
    set_config("cxxflags", "-fPIC")
end

add_packages("spdlog")

includes("thirdparty")

target("ice")
    set_kind("static")
    add_deps("log", "ws")
    add_packages("asio", "nlohmann_json", "libjuice")
    add_files("src/ice/*.cpp")
    add_includedirs("src/ws")
    add_includedirs("thirdparty/libjuice/include", {public = true})

target("ws")
    set_kind("static")
    add_deps("log")
    add_files("src/ws/*.cpp")
    add_packages("asio")
    add_includedirs("thirdparty/websocketpp/include", {public = true})

target("log")
    set_kind("headeronly")
    add_packages("spdlog")
    add_headerfiles("src/log/log.h")
    add_includedirs("src/log", {public = true})

target("pc")
    set_kind("static")
    add_deps("log")
    add_deps("ws", "ice")
    add_files("src/pc/*.cpp")
    add_packages("asio", "nlohmann_json")
    add_includedirs("src/ws", "src/ice")

target("projectx")
    set_kind("shared")
    add_deps("log")
    add_deps("ice", "ws", "pc")
    add_files("src/rtc/*.cpp")
    add_packages("asio", "nlohmann_json")
    add_includedirs("src/rtc", "src/ice", "src/ws", "src/pc")
    add_rules("utils.symbols.export_all", {export_classes = true})
    -- set_policy("build.merge_archive", true)
    -- set_targetdir("$(projectdir)/libdrtc/lib")

target("signal_server")
    set_kind("binary")
    add_deps("log")
    add_files("tests/signal_server/*.cpp")
    add_packages("asio", "nlohmann_json", "spdlog")
    add_includedirs("thirdparty/websocketpp/include")

target("Offer")
    set_kind("binary")
    add_deps("projectx")
    add_files("tests/peerconnection/offer.cpp")
    add_includedirs("src/rtc")

target("Answer")
    set_kind("binary")
    add_deps("projectx")
    add_files("tests/peerconnection/answer.cpp")
    add_includedirs("src/rtc")