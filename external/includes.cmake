set(EXT ${CMAKE_SOURCE_DIR}/external)

macro(config_compiler_and_linker)
    include_directories(${EXT}/googletest/include)
endmacro()