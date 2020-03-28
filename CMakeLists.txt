# nanabozo:CMakeLists.txt

cmake_minimum_required( VERSION 3.10 )
project( nanabozo )

set( INPUTSIZE 512 CACHE STRING "Maximum line length." )
add_definitions( -DINPUTSIZE=${INPUTSIZE} )

if ( WIN32 )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
else()
  add_definitions( -g -Wall -O3 )
endif()

add_executable( nanabozo nanabozo.c )
install( TARGETS nanabozo RUNTIME DESTINATION bin )

# man page
if ( NOT WIN32 )
  add_custom_command(
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/nanabozo.1
    COMMAND gzip ARGS -c ${CMAKE_CURRENT_SOURCE_DIR}/nanabozo.1 > ${CMAKE_CURRENT_BINARY_DIR}/nanabozo.1.gz
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nanabozo.1.gz )

  add_custom_target( manpage ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/nanabozo.1.gz )

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/nanabozo.1.gz
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1 )
endif()

# vi: fenc=utf-8 ff=unix et ai sw=2 ts=2 sts=2
