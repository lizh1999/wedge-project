
target("wedge.strategy", function () 
  set_kind("static")
  add_files("*.cc")
  add_packages("spdlog", {public = true})
end)