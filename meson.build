project('RiEngine', 'cpp', default_options : ['cpp_std=c++17'])

sol2_dep = dependency('sol2', fallback: ['sol2', 'sol2_dep'])

RiEngine_deps = [sol2_dep]
RiEngine_inc = include_directories('include')
RiEngine_dep = declare_dependency(include_directories : RiEngine_inc, dependencies : RiEngine_deps)

executable('run', 'app/main.cpp', dependencies : RiEngine_deps)