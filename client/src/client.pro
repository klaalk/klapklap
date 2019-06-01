QT += core \
      gui \
      widgets \
      websockets

CONFIG += c++11
## Location of SMTP Library
#KK_LIBRARY_LOCATION = $$PWD/../../build-kklib-Desktop_Qt_5_12_2_MinGW_64_bit-Debug

#win32:CONFIG(release, debug|release): LIBS += -L$$KK_LIBRARY_LOCATION/release/ -lKKLIB
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$KK_LIBRARY_LOCATION/debug/ -lKKLIB
#else:unix: LIBS += -L$$KK_LIBRARY_LOCATION -lKKLIB

#INCLUDEPATH += $$KK_LIBRARY_LOCATION
#DEPENDPATH += $$KK_LIBRARY_LOCATION

requires(qtConfig(filedialog))
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app

TARGET          = klapklap

HEADERS         = ../../libs/src/classes/crdt/kk_crdt.h \
                ../../libs/src/classes/crdt/char/kk_char.h \
                ../../libs/src/classes/crdt/pos/kk_pos.h \
                ../../libs/src/classes/crdt/identifier/kk_identifier.h \
                ../../libs/src/classes/crypt/kk_crypt.h \
                ../../libs/src/classes/payload/kk_payload.h \
                ../../libs/src/constants/kk_constants.h \
                classes/textedit/textedit.h \
                classes/client/kk_client.h \
                classes/mainwindow.h\

SOURCES         = ../../libs/src/classes/crdt/kk_crdt.cpp \
                ../../libs/src/classes/crdt/char/kk_char.cpp \
                ../../libs/src/classes/crdt/pos/kk_pos.cpp \
                ../../libs/src/classes/crdt/identifier/kk_identifier.cpp \
                ../../libs/src/classes/crypt/kk_crypt.cpp \
                ../../libs/src/classes/payload/kk_payload.cpp \
                classes/textedit/textedit.cpp \
                classes/client/kk_client.cpp \
                classes/mainwindow.cpp\
                main.cpp

RESOURCES += assets/client.qrc

FORMS += \
        ui/mainwindow.ui

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

EXAMPLE_FILES = textedit.qdoc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/klapklap
INSTALLS += target
