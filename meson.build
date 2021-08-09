project('musplay', 'cpp')

fmt = dependency('fmt')
sdl = dependency('SDL2')
glew = dependency('glew')
subdir('dep')

sources = files([
    'src/musplay.cpp',
])

executable('musplay', sources, dependencies : [fmt, sdl, imgui_dep, glew])
