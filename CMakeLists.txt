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

# vi: fenc=utf-8 ff=unix et ai sw=2 ts=2 sts=2
