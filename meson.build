project('musplay', 'cpp', default_options : ['cpp_std=c++20'])

sdl = dependency('SDL2')
fmt = dependency('fmt')
threads = dependency('threads')

sources = files([
    'src/musplay.cpp',
    'src/midi.cpp',
    'src/synth.cpp',
    'src/voice.cpp',
])

executable('musplay', sources, dependencies : [sdl, fmt, threads])
