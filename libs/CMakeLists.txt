project(kklib LANGUAGES CXX C VERSION 0.1.0)
add_library(kklib STATIC
        src/classes/message/message.cpp src/classes/message/message.h src/constants/kk_constants.h)
