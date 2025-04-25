add_requires("mem", "minhook", "spdlog")

target("AntiTelemetry")
    add_rules("mode.debug", "mode.release")
    set_kind("shared")
    set_languages("cxx20")
    add_files("src/*.cpp")
    add_includedirs("src", "vendor")
    add_headerfiles("src/*.h")
    add_headerfiles("src/*.hpp")

    add_packages("mem", "minhook", "spdlog")
    add_defines("WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "NOMINMAX")