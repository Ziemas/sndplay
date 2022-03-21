project('sndplay', 'cpp', default_options : ['cpp_std=c++20'])

sdl = dependency('SDL2')
fmt = dependency('fmt')
threads = dependency('threads')

sources = files([
    'src/sndplay.cpp',
    'src/player.cpp',
    'src/loader.cpp',
    'src/midi_handler.cpp',
    'src/ame_handler.cpp',
    'src/synth.cpp',
    'src/voice.cpp',
    'src/envelope.cpp',
    'src/util.cpp',
])

executable('sndplay', sources, dependencies : [sdl, fmt, threads])
