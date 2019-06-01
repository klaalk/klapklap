QT       += core
 
TARGET = KKLIB

# Build as an application
#TEMPLATE = app

# Build as a library
TEMPLATE = lib
DEFINES += KKLIB_BUILD
win32:CONFIG += dll

SOURCES += \
            src/classes/crdt/kk_crdt.cpp \
            src/classes/crdt/char/kk_char.cpp \
            src/classes/crdt/pos/kk_pos.cpp \
            src/classes/crdt/identifier/kk_identifier.cpp \
            src/classes/crypt/kk_crypt.cpp \
            src/classes/payload/kk_payload.cpp \

HEADERS  += \
            src/classes/crdt/kk_crdt.h \
            src/classes/crdt/char/kk_char.h \
            src/classes/crdt/pos/kk_pos.h \
            src/classes/crdt/identifier/kk_identifier.h \
            src/classes/crypt/kk_crypt.h \
            src/classes/payload/kk_payload.h \
            src/constants/kk_constants.h \
            src/kklibexport.h\
            src/kkmime.h
