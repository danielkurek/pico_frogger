add_library(game_engine INTERFACE)

target_sources(game_engine INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/engine.cpp
)

target_include_directories(game_engine INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# Pull in pico libraries that we need
target_link_libraries(game_engine INTERFACE pico_ssd1306_spi pico_stdlib)