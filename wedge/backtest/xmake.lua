
target("wedge.backtest", function () 
  set_kind("binary")
  add_files("*.cc", "*/*.cc")
  add_deps("wedge.dataset", "wedge.strategy")
end)