# CMakeLists.txt : CMake project for CG_TP_2
cmake_minimum_required(VERSION 3.10)

project("CG_TP_2" LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Main executable (project root sources)
add_executable(CG_TP_2 "CG_TP_2.cpp" "CG_TP_2.h")

# Path to the folder you added
set(HOUSE_DIR ${CMAKE_SOURCE_DIR}/house_zip)

# If house_zip provides its own CMake, add it; otherwise create a library
if(EXISTS "${HOUSE_DIR}/CMakeLists.txt")
  add_subdirectory(${HOUSE_DIR} house_zip_build)
  # Link a target from that CMake if provided, e.g.:
  # target_link_libraries(CG_TP_2 PRIVATE house_target_name)
else()
  # Collect source files but exclude example main.cpp so we don't get two mains
  file(GLOB_RECURSE HOUSE_SRCS
    "${HOUSE_DIR}/src/*.c" "${HOUSE_DIR}/src/*.cpp"
    "${HOUSE_DIR}/*.c" "${HOUSE_DIR}/*.cpp")
  # Remove example main.cpp if present
  list(REMOVE_ITEM HOUSE_SRCS "${HOUSE_DIR}/src/main.cpp" "${HOUSE_DIR}/main.cpp")

  file(GLOB_RECURSE HOUSE_HDRS
    "${HOUSE_DIR}/src/*.h" "${HOUSE_DIR}/src/*.hpp"
    "${HOUSE_DIR}/common/*.h" "${HOUSE_DIR}/common/*.hpp"
    "${HOUSE_DIR}/*.h" "${HOUSE_DIR}/*.hpp")

  if(HOUSE_SRCS)
    add_library(house_zip_lib STATIC ${HOUSE_SRCS} ${HOUSE_HDRS})
    add_library(house_zip_lib STATIC ${HOUSE_SRCS} ${HOUSE_HDRS})
    target_include_directories(house_zip_lib PUBLIC
    "${HOUSE_DIR}/src" "${HOUSE_DIR}/common" "${HOUSE_DIR}")

    # Find and link OpenGL and GLEW so headers like <GL/glew.h> are visible to IDE and linker
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED) # requires GLEW installed / available via vcpkg or system

  target_link_libraries(house_zip_lib PRIVATE GLEW::GLEW OpenGL::GL)
  target_link_libraries(CG_TP_2 PRIVATE house_zip_lib)

    target_include_directories(house_zip_lib PUBLIC
      "${HOUSE_DIR}/src" "${HOUSE_DIR}/common" "${HOUSE_DIR}")
    target_link_libraries(CG_TP_2 PRIVATE house_zip_lib)
  endif()
endif()

# Expose headers to main target so you can #include "shader.hpp" etc.
if(EXISTS "${HOUSE_DIR}/src" OR EXISTS "${HOUSE_DIR}/common")
  target_include_directories(CG_TP_2 PRIVATE
    "${HOUSE_DIR}/src" "${HOUSE_DIR}/common" "${HOUSE_DIR}")
endif()

# Copy runtime assets:
# - copy entire house_zip/bin next to the exe (if present)
# - copy shader files from house_zip root or house_zip/shaders to the exe folder
if(EXISTS "${HOUSE_DIR}/bin")
  add_custom_command(TARGET CG_TP_2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${HOUSE_DIR}/bin"
      "$<TARGET_FILE_DIR:CG_TP_2>/bin"
  )
endif()

file(GLOB HOUSE_SHADERS
  "${HOUSE_DIR}/*.vertexshader"
  "${HOUSE_DIR}/*.fragmentshader"
  "${HOUSE_DIR}/shaders/*")

foreach(_sh IN LISTS HOUSE_SHADERS)
  if(EXISTS "${_sh}")
    get_filename_component(_sh_name ${_sh} NAME)
    add_custom_command(TARGET CG_TP_2 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${_sh}"
        "$<TARGET_FILE_DIR:CG_TP_2>/${_sh_name}"
    )
  endif()
endforeach()
# End of file
