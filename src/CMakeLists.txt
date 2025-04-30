file(GLOB_RECURSE HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
file(GLOB SOURCES_LOC "localization/*.cpp")
file(GLOB SOURCES_SDL "sdl/*.cpp")

if(WIN32)
  file(GLOB SOURCES_WIN "win32/*.cpp")
  list(APPEND SOURCES ${SOURCES_WIN})
endif()

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
               ${HEADERS} ${SOURCES} ${SOURCES_LOC} ${SOURCES_SDL})
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
target_link_libraries(
  ${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image
                          SDL3_mixer::SDL3_mixer)
install(TARGETS ${PROJECT_NAME})
