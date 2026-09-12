-- From MiniRTC: xmake lua -P . tests/build_codec_options_test.lua macosx arm64 false
-- Resolve the actual build graph without downloading or compiling dependencies.
function main(plat, arch, enable_aom)
    import("core.project.config")
    import("core.project.project")
    config.load()
    config.set("plat", plat, {force = true})
    config.set("arch", arch, {force = true})
    config.set("USE_CUDA", false, {force = true})
    config.set("MINIRTC_ENABLE_AOM", enable_aom == "true", {force = true})
    local targets = project.targets()
    local media = targets.media
    local aom_count = 0
    local svt, dav1d = false, false
    for _, file in ipairs(media:sourcefiles()) do
        if file:find("/aom/", 1, true) then aom_count = aom_count + 1 end
        if file:endswith("svt_av1_encoder.cpp") then svt = true end
        if file:endswith("dav1d_av1_decoder.cpp") then dav1d = true end
    end
    assert(svt and dav1d, "default AV1 codecs must remain available")
    assert(aom_count == (enable_aom == "true" and 2 or 0), "incorrect AOM sources")
    for _, name in ipairs({"media", "minirtc"}) do
        assert(table.contains(table.wrap(targets[name]:get("packages")), "aom") ==
            (enable_aom == "true"), "incorrect AOM dependency on " .. name)
    end
    print("Codec build options passed: %s/%s aom=%s", plat, arch, enable_aom)
end
