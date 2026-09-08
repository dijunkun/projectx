package("libdatachannel")
    set_homepage("https://libdatachannel.org/")
    set_description("C/C++ WebRTC network library featuring Data Channels, Media Transport, and WebSockets")
    set_license("MPL-2.0")
    -- A cache built against pre-UPnP libnice must not survive this dependency change.
    set_policy("package.librarydeps.strict_compatibility", true)

    add_urls("https://github.com/paullouisageneau/libdatachannel/archive/refs/tags/$(version).tar.gz",
             "https://github.com/paullouisageneau/libdatachannel.git", {submodules = false})

    add_versions("v0.24.5", "454537c3cd526bed935d847bb2dff4046f266eef84d43b2a5f2f2f293c0026f4")
    -- 0.24 already includes the consent-freshness fix. Its teardown barrier
    -- still detaches callbacks and closes sockets outside the polling context.
    add_patches("v0.24.5", path.join(os.scriptdir(), "patches", "nice_context_teardown_0.24.patch"), "3bb46e4ec2b763cb0d12c9740c42a29313dad5e142ee000707a9ab1c027baee6")

    add_configs("gnutls", {description = "Use GnuTLS instead of OpenSSL", default = false, type = "boolean", readonly = true})
    add_configs("mbedtls", {description = "Use Mbed TLS instead of OpenSSL", default = false, type = "boolean"})
    add_configs("nice", {description = "Use libnice instead of libjuice", default = true, type = "boolean"})
    add_configs("multi_stun", {description = "Use MiniRTC multi-endpoint STUN extension", default = true, type = "boolean", readonly = true})
    add_configs("websocket", {description = "Enable WebSocket support", default = false, type = "boolean"})
    add_configs("media", {description = "Enable media transport support", default = true, type = "boolean"})
    add_configs("capi_stdcall", {description = "Set calling convention of C API callbacks stdcall", default = false, type = "boolean"})
    add_configs("sctp_debug", {description = "Enable SCTP debugging output to verbose log", default = false, type = "boolean"})
    add_configs("disable_consent_freshness", {description = "Disable RFC 7675 Consent Freshness", default = false, type = "boolean"})
    add_configs("rtc_update_version_header", {description = "Enable updating the version header", default = false, type = "boolean"})

    if is_plat("windows", "mingw") then
        add_syslinks("ws2_32")
    elseif is_plat("linux", "bsd") then
        add_syslinks("pthread")
    end

    add_deps("cmake~host", {host = true})
    add_deps("plog", "usrsctp")
    add_deps("nlohmann_json 3.11.3")

    on_check("wasm", function (target)
        raise("package(libdatachannel) dep(usrsctp) unsupported wasm platform")
    end)

    on_check("android", function (package)
        local ndk = package:toolchain("ndk")
        local ndk_sdkver = ndk:config("ndk_sdkver")
        assert(ndk_sdkver and tonumber(ndk_sdkver) > 23, "package(libdatachannel) dep(usrsctp) need ndk api level > 23")
    end)

    on_load(function (package)
        if package:config("mbedtls") then
            raise("Unsupported now, build failed with `src/impl/dtlstransport.cpp:373:7: error: 'mbedtls_ssl_srtp_profile' does not name a type; did you mean 'mbedtls_x509_crt_profile'?`")
            package:add("deps", "mbedtls")
        elseif package:config("gnutls") then
            package:add("deps", "gnutls")
        else
            package:add("deps", "openssl3 3.3.2")
        end

        if package:config("nice") then
            package:add("deps", "libnice 0.1.24")
        else
            package:add("deps", "libjuice")
        end

        if package:config("media") then
            package:add("deps", "libsrtp v2.7.0")
        else
            package:add("defines", "RTC_ENABLE_MEDIA=0")
        end

        if package:config("capi_stdcall") then
            package:add("defines", "CAPI_STDCALL")
        end

        package:add("defines", "RTC_ENABLE_WEBSOCKET=" .. (package:config("websocket") and "1" or "0"))

        if not package:config("shared") then 
            package:add("defines", "RTC_STATIC")
        end 
    end)

    on_install("!mingw", function (package)
        -- Upstream explicitly disables UPnP even when libnice includes GUPnP.
        -- Match the native transport, while respecting relay-only privacy.
        if package:config("nice") then
            io.replace("src/impl/icetransport.cpp",
                "// Add one STUN server\n\tbool success = false;\n\tfor (auto &server : servers) {",
                [[// Use every configured STUN endpoint on each libnice UDP socket.
    const bool multiStun = g_object_class_find_property(
        G_OBJECT_GET_CLASS(mNiceAgent.get()), "stun-servers") != nullptr;
    if (multiStun) {
        std::string endpoints;
        unsigned count = 0;
        for (const auto &server : config.iceServers) {
            if (server.type != IceServer::Type::Stun || server.hostname.empty()) continue;
            if (count++ == 8) break;
            if (!endpoints.empty()) endpoints += ',';
            const auto &host = server.hostname;
            endpoints += (host.find(':') == std::string::npos ? host : "[" + host + "]");
            endpoints += ":" + std::to_string(server.port ? server.port : 3478);
        }
        if (!endpoints.empty())
            g_object_set(G_OBJECT(mNiceAgent.get()), "stun-servers", endpoints.c_str(), nullptr);
    }
    bool success = false;
    for (auto &server : servers) {
        if (multiStun) break;]], {plain = true})
            io.replace("src/impl/icetransport.cpp", '"upnp", FALSE, nullptr',
                '"upnp", config.iceTransportPolicy == TransportPolicy::Relay ? FALSE : TRUE, nullptr',
                {plain = true})
            io.replace("src/impl/icetransport.cpp", '"upnp-timeout", 200, nullptr',
                '"upnp-timeout", 3000u, nullptr', {plain = true})
        end
        io.replace("CMakeLists.txt", "set(CMAKE_POSITION_INDEPENDENT_CODE ON)", "", {plain = true})
        -- add -DJUICE_STATIC from config mode 
        io.replace("CMakeLists.txt", "find_package(LibJuice REQUIRED)", "find_package(LibJuice CONFIG REQUIRED)", {plain = true})
        io.replace("CMakeLists.txt", "find_package(LibJuice 1.7.0 REQUIRED)", "find_package(LibJuice 1.7.0 CONFIG REQUIRED)", {plain = true})
        -- Error evaluating generator expression: $<TARGET_PDB_FILE:datachannel>
        -- TARGET_PDB_FILE is allowed only for targets with linker created artifacts.
        if package:is_plat("windows") then
            io.replace("CMakeLists.txt", "if(MSVC)\n\tinstall", "if(0)\ninstall", {plain = true})
        end

        local configs = {
            "-DNO_EXAMPLES=ON",
            "-DNO_TESTS=ON",
            "-DWARNINGS_AS_ERRORS=OFF",
            "-DPREFER_SYSTEM_LIB=ON",
        }
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))

        local openssl = package:dep("openssl3")
        if openssl and not openssl:is_system() then
            table.insert(configs, "-DOPENSSL_USE_STATIC_LIBS=" .. (not openssl:config("shared") and "ON" or "OFF"))
            table.insert(configs, "-DOPENSSL_ROOT_DIR=" .. openssl:installdir())
        end

        table.insert(configs, "-DUSE_GNUTLS=" .. (package:config("gnutls") and "ON" or "OFF"))
        table.insert(configs, "-DUSE_MBEDTLS=" .. (package:config("mbedtls") and "ON" or "OFF"))
        table.insert(configs, "-DUSE_NICE=" .. (package:config("nice") and "ON" or "OFF"))
        table.insert(configs, "-DNO_WEBSOCKET=" .. (not package:config("websocket") and "ON" or "OFF"))
        table.insert(configs, "-DNO_MEDIA=" .. (not package:config("media") and "ON" or "OFF"))
        table.insert(configs, "-DCAPI_STDCALL=" .. (package:config("capi_stdcall") and "ON" or "OFF"))
        table.insert(configs, "-DSCTP_DEBUG=" .. (package:config("sctp_debug") and "ON" or "OFF"))
        table.insert(configs, "-DDISABLE_CONSENT_FRESHNESS=" .. (package:config("disable_consent_freshness") and "ON" or "OFF"))
        table.insert(configs, "-DRTC_UPDATE_VERSION_HEADER=" .. (package:config("rtc_update_version_header") and "ON" or "OFF"))

        import("package.tools.cmake").install(package, configs, {
            targets = {
                package:config("shared") and "datachannel" or "datachannel-static",
            }
        })
    end)

    on_test(function (package)
        assert(package:has_cfuncs("rtcSetUserPointer", {includes = "rtc/rtc.h"}))
    end)
