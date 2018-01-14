# nanabozo:CMakeLists.txt

cmake_minimum_required( VERSION 3.7 )
project( nanabozo )

if ( WIN32 )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

add_executable( nanabozo nanabozo.c )
install( TARGETS nanabozo RUNTIME DESTINATION bin )

# vi: set fenc=utf-8 ff=unix et sw=2 ts=2 sts=2 :
