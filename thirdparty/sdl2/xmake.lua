package("sdl2")
    add_urls("https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.28.3.tar.gz", {alias = "github"})
    add_versions("github:2.28.3", "c17455d6e0c484bfe634b8de6af4c608e86ee449c28e40af04064aa6643fe382")

    add_deps("cmake")
    on_install(function (package)
        local configs = {}
        import("package.tools.cmake").install(package, configs)
    end)
package_end()