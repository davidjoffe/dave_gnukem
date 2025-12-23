file(GLOB_RECURSE HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
file(GLOB SOURCES_LOC "localization/*.cpp")
file(GLOB SOURCES_SDL "sdl/*.cpp")

if(WIN32)
  file(GLOB SOURCES_WIN "win32/*.cpp")
  list(APPEND SOURCES ${SOURCES_WIN})
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
               ${HEADERS} ${SOURCES} ${SOURCES_LOC} ${SOURCES_SDL})
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
target_link_libraries(
  ${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image
                          SDL2_mixer::SDL2_mixer)
install(TARGETS ${PROJECT_NAME})
