
target("wedge.backtest2", function () 
  set_kind("binary")
  add_files("*.cc")
  add_deps("wedge.dataset")
end)