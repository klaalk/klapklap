QT += widgets \
      websockets
requires(qtConfig(filedialog))
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app

TARGET          = klapklap

HEADERS         = ../../libs/src/classes/crdt/kk_crdt.h \
                  ../../libs/src/classes/crdt/char/kk_char.h \
                  ../../libs/src/classes/crdt/pos/kk_pos.h \
                  ../../libs/src/classes/crdt/identifier/kk_identifier.h \
                  classes/textedit/textedit.h \
                  classes/client/kk_client.h

SOURCES         = ../../libs/src/classes/crdt/kk_crdt.cpp \
                  ../../libs/src/classes/crdt/char/kk_char.cpp \
                  ../../libs/src/classes/crdt/pos/kk_pos.cpp \
                  ../../libs/src/classes/crdt/identifier/kk_identifier.cpp \
                  classes/textedit/textedit.cpp \
                  classes/client/kk_client.cpp \
                  main.cpp

RESOURCES += assets/client.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

EXAMPLE_FILES = textedit.qdoc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/klapklap
INSTALLS += target
