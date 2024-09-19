
target("wedge.dataset", function () 
  set_kind("static")
  add_files("sql_dataset.cc", "sql_iterator.cc")
  add_packages("sqlite3", { public = true })
end)

target("wedge.download", function () 
  set_kind("binary")
  add_files("download.cc")
  add_deps("wedge.binance", "wedge.dataset")
end)