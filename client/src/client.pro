QT += widgets \
      websockets
requires(qtConfig(filedialog))
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app

TARGET          = klapklap

HEADERS         = classes/textedit/textedit.h \
                  classes/client/kk_client.h

SOURCES         = classes/textedit/textedit.cpp \
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
