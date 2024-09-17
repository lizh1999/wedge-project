
target("wedge.binance", function () 
  set_kind("static")
  add_files("*.cc")
  add_packages("boost", {public = true})
  add_packages("openssl")
end)