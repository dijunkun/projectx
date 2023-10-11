set_project("projectx")
set_version("0.0.1")

add_rules("mode.release", "mode.debug")
set_languages("c++17")

add_rules("mode.release", "mode.debug")

add_requires("asio 1.24.0", "nlohmann_json", "spdlog 1.11.0")

add_defines("ASIO_STANDALONE", "ASIO_HAS_STD_TYPE_TRAITS", "ASIO_HAS_STD_SHARED_PTR", 
    "ASIO_HAS_STD_ADDRESSOF", "ASIO_HAS_STD_ATOMIC", "ASIO_HAS_STD_CHRONO", "ASIO_HAS_CSTDINT", "ASIO_HAS_STD_ARRAY",
    "ASIO_HAS_STD_SYSTEM_ERROR", "SIGNAL_LOGGER")

if is_os("windows") then
    add_defines("_WEBSOCKETPP_CPP11_INTERNAL_")
    add_links("ws2_32", "Bcrypt")
    add_requires("cuda")
elseif is_os("linux") then 
    add_links("pthread")
    set_config("cxxflags", "-fPIC")
end

add_packages("spdlog")

includes("../../thirdparty")

target("log")
    set_kind("headeronly")
    add_packages("spdlog")
    add_headerfiles("../utils/log/log.h")
    add_includedirs("../utils/log", {public = true})

target("common")
    set_kind("headeronly")
    add_includedirs("../../src/common", {public = true})

target("signal_server")
    set_kind("binary")
    add_deps("log", "common")
    add_files("*.cpp")
    add_packages("asio", "nlohmann_json", "spdlog")
    add_includedirs("../../thirdparty/websocketpp/include")
