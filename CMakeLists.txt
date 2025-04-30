cmake_minimum_required(VERSION 3.20)

project(
  davegnukem
  LANGUAGES CXX
  VERSION 1.0.3)

set(CMAKE_CXX_STANDARD 14)

include(GNUInstallDirs)
include(FetchContent)

# default to same version for data repo
set(DATA_TAG
    ${PROJECT_VERSION}
    CACHE STRING "default")

FetchContent_Declare(
  data
  GIT_REPOSITORY https://github.com/davidjoffe/gnukem_data
  GIT_TAG ${DATA_TAG})

# default to same version for datasrc repo
set(DATASRC_TAG
    ${PROJECT_VERSION}
    CACHE STRING "default")

FetchContent_Declare(
  datasrc
  GIT_REPOSITORY https://github.com/davidjoffe/gnukem_datasrc
  GIT_TAG ${DATASRC_TAG})

FetchContent_MakeAvailable(data datasrc)

add_subdirectory(src)

install(FILES debian/appstream/com.djoffe.davegnukem.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES debian/desktop/davegnukem.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(DIRECTORY debian/icons DESTINATION ${CMAKE_INSTALL_DATADIR})
install(FILES HISTORY.txt README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

install(DIRECTORY locale DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

install(
  DIRECTORY ${data_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  PATTERN .git EXCLUDE
  PATTERN .gitignore EXCLUDE
  PATTERN README.md EXCLUDE PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ
                                        WORLD_READ)
install(FILES ${data_SOURCE_DIR}/README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}-data)

install(
  DIRECTORY ${datasrc_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  PATTERN .git EXCLUDE
  PATTERN .gitignore EXCLUDE
  PATTERN README.md EXCLUDE
  PATTERN "*.psd" EXCLUDE)
install(FILES ${datasrc_SOURCE_DIR}/README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}-datasrc)

file(READ debian/davegnukem.6 TEXT)
string(REPLACE "VERSION" "${PROJECT_VERSION}" TEXT "${TEXT}")
file(
  GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/davegnukem.6
  CONTENT "${TEXT}")
set_property(
  DIRECTORY
  APPEND
  PROPERTY CMAKE_CONFIGURE_DEPENDS main.cpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/davegnukem.6
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
