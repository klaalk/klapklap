project(kklib)
add_library(lib classes/connection_handler/con_handler.cpp classes/connection_handler/con_handler.h)

install(TARGETS kklib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
