add_rules("mode.debug", "mode.release")

add_rules("plugin.compile_commands.autoupdate")

add_requires("boost", "fmt", "nlohmann_json", "openssl", "sqlite3")
add_requires("spdlog", { configs = {fmt_external = true} })

set_languages("cxx20")
add_includedirs("$(projectdir)")

add_packages("boost", "fmt", "nlohmann_json", "spdlog")

local project_root = os.projectdir():gsub("\\", "/")
add_defines("PROJECT_ROOT_DIR=\"" .. project_root .. "\"")

if is_plat("windows") then
  add_cxxflags("/utf-8")
end

includes("wedge")