package("ffmpeg")
    set_homepage("https://www.ffmpeg.org")
    set_description("A collection of libraries to process multimedia content such as audio, video, subtitles and related metadata.")
    set_license("GPL-3.0")

    if is_plat("windows", "mingw") then
        add_urls("https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-$(version)-full_build-shared.7z")
        add_versions("5.1.2", "d9eb97b72d7cfdae4d0f7eaea59ccffb8c364d67d88018ea715d5e2e193f00e9")

        add_configs("shared", {description = "Download shared binaries.", default = true, type = "boolean", readonly = true})
        add_configs("vs_runtime", {description = "Set vs compiler runtime.", default = "MD", readonly = true})
    else
        add_urls("https://ffmpeg.org/releases/ffmpeg-$(version).tar.bz2", {alias = "home"})
        add_urls("https://github.com/FFmpeg/FFmpeg/archive/n$(version).zip", {alias = "github"})
        add_urls("https://git.ffmpeg.org/ffmpeg.git", "https://github.com/FFmpeg/FFmpeg.git", {alias = "git"})
        add_versions("home:5.1.2", "39a0bcc8d98549f16c570624678246a6ac736c066cebdb409f9502e915b22f2b")
        add_versions("github:5.1.2", "0c99f3609160f40946e2531804175eea16416320c4b6365ad075e390600539db")
        add_versions("git:5.1.2", "n5.1.2")

        add_configs("asm",              {description = "Disable asm", default = false, type = "boolean"})
        add_configs("pic",              {description = "Enable force Position Independent Code", default = true, type = "boolean"})
        add_configs("gpl",              {description = "Enable GPL code", default = true, type = "boolean"})
        add_configs("ffprobe",          {description = "Enable ffprobe program.", default = false, type = "boolean"})
        add_configs("ffmpeg",           {description = "Enable ffmpeg program.", default = true, type = "boolean"})
        add_configs("ffplay",           {description = "Enable ffplay program.", default = false, type = "boolean"})
        add_configs("zlib",             {description = "Enable zlib compression library.", default = false, type = "boolean"})
        add_configs("lzma",             {description = "Enable liblzma compression library.", default = false, type = "boolean"})
        add_configs("bzlib",            {description = "Enable bzlib compression library.", default = false, type = "boolean"})
        add_configs("libx264",          {description = "Enable libx264 decoder.", default = false, type = "boolean"})
        add_configs("libx265",          {description = "Enable libx265 decoder.", default = false, type = "boolean"})
        add_configs("iconv",            {description = "Enable libiconv library.", default = false, type = "boolean"})
        add_configs("hardcoded-tables", {description = "Enable hardcoded tables.", default = true, type = "boolean"})
    end

    -- add_links("avfilter", "avdevice", "avformat", "avcodec", "swscale", "swresample", "avutil")
    if is_plat("macosx") then
        add_frameworks("CoreFoundation", "Foundation", "CoreVideo", "CoreMedia", "AudioToolbox", "VideoToolbox", "Security")
    elseif is_plat("linux") then
        add_syslinks("pthread")
    end

    if is_plat("linux", "macosx") then
        add_deps("yasm")
    end

    if on_fetch then
        on_fetch("mingw", "linux", "macosx", function (package, opt)
            import("lib.detect.find_tool")
            if opt.system then
                local result
                for _, name in ipairs({"libavcodec", "libavdevice", "libavfilter", "libavformat", "libavutil", "libpostproc", "libswresample", "libswscale"}) do
                    local pkginfo = package:find_package("pkgconfig::" .. name, opt)
                    if pkginfo then
                        pkginfo.version = nil
                        if not result then
                            result = pkginfo
                        else
                            result = result .. pkginfo
                        end
                    else
                        return
                    end
                end
                local ffmpeg = find_tool("ffmpeg", {check = "-help", version = true, command = "-version", parse = "%d+%.?%d+%.?%d+", force = true})
                if ffmpeg then
                    result.version = ffmpeg.version
                end
                return result
            end
        end)
    end

    on_load("linux", "macos", "android", function (package)
        local configdeps = {zlib    = "zlib",
                            bzlib   = "bzip2",
                            lzma    = "xz",
                            libx264 = "x264",
                            libx265 = "x265",
                            iconv   = "libiconv"}
        for name, dep in pairs(configdeps) do
            if package:config(name) then
                package:add("deps", dep)
            end
        end
        -- https://www.ffmpeg.org/platform.html#toc-Advanced-linking-configuration
        if package:config("pic") ~= false then
            package:add("shflags", "-Wl,-Bsymbolic")
            package:add("ldflags", "-Wl,-Bsymbolic")
        end
        if not package:config("gpl") then
            package:set("license", "LGPL-3.0")
        end
    end)

    on_install("windows|x64", "mingw|x86_64", function (package)
        os.mv("bin", package:installdir())
        os.mv("include", package:installdir())
        os.mv("lib", package:installdir())
        package:addenv("PATH", "bin")
    end)

    on_install("linux", "macosx", "android@linux,macosx", function (package)
        local configs = {"--enable-version3",
                         "--disable-doc"}
        if package:config("gpl") then
            table.insert(configs, "--enable-gpl")
        end
        if is_plat("macosx") and macos.version():ge("10.8") then
            table.insert(configs, "--enable-videotoolbox")
        end
        for name, enabled in pairs(package:configs()) do
            if not package:extraconf("configs", name, "builtin") then
                if enabled then
                    table.insert(configs, "--enable-" .. name)
                else
                    table.insert(configs, "--disable-" .. name)
                end
            end
        end
        if package:config("shared") then
            table.insert(configs, "--enable-shared")
            table.insert(configs, "--disable-static")
        else
            table.insert(configs, "--enable-static")
            table.insert(configs, "--disable-shared")
        end
        if package:debug() then
            table.insert(configs, "--enable-debug")
        else
            table.insert(configs, "--disable-debug")
        end
        if package:config("pic") ~= false then
            table.insert(configs, "--enable-pic")
        end
        if package:is_plat("android") then
            import("core.base.option")
            import("core.tool.toolchain")
            local ndk = toolchain.load("ndk", {plat = package:plat(), arch = package:arch()})
            local bin = ndk:bindir()
            local ndk_sdkver = ndk:config("ndk_sdkver")
            local arch, cpu, triple, cross_prefix
            if package:is_arch("arm64-v8a") then
                arch = "arm64"
                cpu = "armv8-a"
                triple = "aarch64-linux-android"
                cross_prefix = path.join(bin, "aarch64-linux-android-")
            elseif package:arch():startswith("arm") then
                arch = "arm"
                cpu = "armv7-a"
                triple = "armv7a-linux-androideabi"
                cross_prefix = path.join(bin, "arm-linux-androideabi-")
            else
                raise("unknown arch(%s) for android!", package:arch())
            end
            local sysroot  = path.join(path.directory(bin), "sysroot")
            local cflags   = table.join(table.wrap(package:config("cxflags")), table.wrap(package:config("cflags")), table.wrap(get_config("cxflags")), get_config("cflags"))
            local cxxflags = table.join(table.wrap(package:config("cxflags")), table.wrap(package:config("cxxflags")), table.wrap(get_config("cxflags")), get_config("cxxflags"))
            assert(os.isdir(sysroot), "we do not support old version ndk!")
            if package:is_arch("arm64-v8a") then
                table.insert(cflags, "-mfpu=neon")
                table.insert(cflags, "-mfloat-abi=soft")
            else
                table.insert(cflags, "-mfpu=neon")
                table.insert(cflags, "-mfloat-abi=soft")
            end
            table.insert(configs, "--enable-neon")
            table.insert(configs, "--enable-asm")
            table.insert(configs, "--enable-jni")
            table.insert(configs, "--target-os=android")
            table.insert(configs, "--enable-cross-compile")
            table.insert(configs, "--disable-avdevice")
            table.insert(configs, "--arch=" .. arch)
            table.insert(configs, "--cpu=" .. cpu)
            table.insert(configs, "--cc=" .. path.join(bin, triple .. ndk_sdkver .. "-clang"))
            table.insert(configs, "--cxx=" .. path.join(bin, triple .. ndk_sdkver .. "-clang++"))
            table.insert(configs, "--extra-cflags=" .. table.concat(cflags, ' '))
            table.insert(configs, "--extra-cxxflags=" .. table.concat(cxxflags, ' '))
            table.insert(configs, "--sysroot=" .. sysroot)
            table.insert(configs, "--cross-prefix=" .. cross_prefix)
            table.insert(configs, "--prefix=" .. package:installdir())
            os.vrunv("./configure", configs)
            local argv = {"-j4"}
            if option.get("verbose") then
                table.insert(argv, "V=1")
            end
            os.vrunv("make", argv)
            os.vrun("make install")
        else
            import("package.tools.autoconf").install(package, configs)
        end
        package:addenv("PATH", "bin")
    end)

    on_test(function (package)
        assert(package:has_cfuncs("avformat_open_input", {includes = "libavformat/avformat.h"}))
    end)