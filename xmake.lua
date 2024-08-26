add_rules("mode.debug", "mode.release")

add_requires("boost")
add_requires("sqlite3")
add_requires("openssl")
add_requires("libcurl")
add_requires("nlohmann_json")

set_languages("cxx23")
add_includedirs("$(projectdir)")

target("network", function () 
  set_kind("static")
  add_files("network/*.cc")
  add_packages("libcurl", { public = true })
end)

target("network2", function () 
  set_kind("static")
  add_files("network2/rest_api.cc")
  add_packages("boost", { public = true })
  add_packages("openssl", { public = true })
end)

target("backtest", function () 
  set_kind("static")
  add_files(
    "backtest/order/limit_buy_order.cc",
    "backtest/order/limit_sell_order.cc",
    "backtest/order/market_buy_order.cc",
    "backtest/order/market_sell_order.cc",
    "backtest/backtest_broker.cc",
    "backtest/backtest_context.cc",
    "backtest/data_loader.cc"
  )
  add_packages("sqlite3")
end)

target("strategy", function () 
  set_kind("static")
  add_files("strategy/grid_strategy.cc")
end)

target("make_dataset", function () 
  set_kind("binary")
  add_files("app/make_dataset.cc")
  add_deps("network")
  add_packages("nlohmann_json", "sqlite3")
end)

target("test_strategy", function () 
  set_kind("binary")
  add_files("app/test_strategy.cc")
  add_deps("backtest", "strategy")
end)