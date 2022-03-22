add_executable(sndplay sndplay.cpp
  player.cpp
  midi_handler.cpp
  ame_handler.cpp
  synth.cpp
  voice.cpp
  util.cpp
  loader.cpp
  envelope.cpp
  )

target_include_directories(sndplay PRIVATE ../src)
target_compile_features(sndplay PRIVATE cxx_std_20)
target_link_libraries(sndplay PRIVATE fmt::fmt cubeb)
