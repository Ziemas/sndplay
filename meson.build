project('musplay', 'cpp')

sdl = dependency('SDL2')
fmt = dependency('fmt')

sources = files([
    'src/musplay.cpp',
])

executable('musplay', sources, dependencies : [sdl, fmt])
