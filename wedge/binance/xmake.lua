
target("wedge.binance", function () 
  set_kind("static")
  add_files("*.cc")
  add_packages("openssl", "boost", "nlohmann_json", {public = true})
end)