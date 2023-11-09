package("openfec")

    set_homepage("http://openfec.org/")
    set_license("CeCCIL-C")

    set_sourcedir(os.scriptdir())
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DDEBUG:STRING=" .. (package:debug() and "ON" or "OFF"))
        table.insert(configs, "-DLIBRARY_OUTPUT_PATH=" .. (path.join(package:installdir(), "lib")))
      
        import("package.tools.cmake").install(package, configs)
        os.cp("src", package:installdir())
        package:add("includedirs", "src")
    end)

    on_test(function (package)
        assert(package:has_cfuncs("of_create_codec_instance", {includes = "lib_common/of_openfec_api.h"}))
    end)