project('sndplay', 'cpp', default_options : ['cpp_std=c++20'])

cmake = import('cmake')
cubeb_proj = cmake.subproject('cubeb')
cubeb = cubeb_proj.dependency('cubeb')

fmt = dependency('fmt')
threads = dependency('threads')
uuid = dependency('uuid')


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

executable('sndplay', sources, dependencies : [fmt, threads, uuid, cubeb])
