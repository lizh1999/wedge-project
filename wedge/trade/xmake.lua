
target("wedge.trade", function () 
  set_kind("binary")
  add_files("*.cc")
  add_deps("wedge.binance", "wedge.strategy")
end)