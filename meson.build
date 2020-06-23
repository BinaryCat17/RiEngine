project('RiEngine', 'cpp',
    version : '1.0',
    default_options : ['cpp_std=c++2a', 'warning_level=3'])

cmake = import('cmake')

# deps --------------------------------------------------------------------------------------------

assimp = dependency('assimp', required : false)
if not assimp.found()
    assimp = cmake.subproject('assimp').dependency('assimp')
endif

spdlog = dependency('spdlog', required : false)
if not spdlog.found()
    spdlog = cmake.subproject('spdlog').dependency('spdlog')
endif

glm = dependency('glm', required : false)
if not glm.found()
    glm = cmake.subproject('glm').dependency('glm')
endif

toml11 = dependency('toml11', required : false)
if not toml11.found()
    toml11 = cmake.subproject('toml11', cmake_options : '-Dtoml11_BUILD_TEST=OFF').dependency('toml11')
endif

cista = dependency('cista', required : false)
if not cista.found()
    cista = cmake.subproject('cista').dependency('cista')
endif

spirv = cmake.subproject('SPIRV-Cross',
    cmake_options : '-DSPIRV_CROSS_ENABLE_TESTS=OFF').dependency('spirv-cross-c')

RiUtil = dependency('RiWindow', fallback : ['RiWindow', 'RiWindow_dep'])
sol2 = dependency('sol2', fallback : ['sol2', 'sol2_dep'])
gtest = dependency('gtest', fallback : ['gtest', 'gtest_dep'])
glew = dependency('glew', fallback : ['glew', 'glew_dep'])

# project -----------------------------------------------------------------------------------------

deps = [RiUtil, sol2, assimp, gtest, spdlog, glew, glm, toml11, cista, spirv]
inc = include_directories('src')

lib = static_library('RiEngine',
    'src/RiEngine/Format.cpp',
    'src/RiEngine/Exception.cpp',
    'src/RiEngine/loaders/MeshLoader.cpp',
    'src/RiEngine/loaders/PipelineLoader.cpp',
    cpp_pch : 'src/RiEngine/pch/pch.hpp',
    install : true,
    dependencies : deps,
    include_directories : [inc, 'src/RiEngine/pch'])

RiEngine_dep = declare_dependency(link_with : lib, include_directories : inc, dependencies : deps)

# tests -------------------------------------------------------------------------------------------

testMeshLoad = executable('testMeshLoader', 'tests/testMeshLoad.cpp', dependencies : RiEngine_dep)
test('testMeshLoad', testMeshLoad, workdir : meson.source_root() / 'tests')
