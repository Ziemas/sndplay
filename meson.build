project('sndplay', 'cpp', default_options : ['cpp_std=c++20'])

sdl = dependency('SDL2')
fmt = dependency('fmt')
threads = dependency('threads')

sources = files([
    'src/sndplay.cpp',
    'src/player.cpp',
    'src/midi.cpp',
    'src/synth.cpp',
    'src/voice.cpp',
    'src/envelope.cpp',
])

executable('musplay', sources, dependencies : [sdl, fmt, threads])
