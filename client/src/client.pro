
macx:ICON = $${PWD}/klapklap.icns

QT += core \
      gui \
      widgets \
      websockets

CONFIG += c++11
CONFIG+=sdk_no_version_check

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

TARGET          = KlapKlap_Client

HEADERS         = ../../libs/src/classes/crdt/kk_crdt.h \
                ../../libs/src/classes/crdt/char/kk_char.h \
                ../../libs/src/classes/crdt/pos/kk_pos.h \
                ../../libs/src/classes/crdt/identifier/kk_identifier.h \
                ../../libs/src/classes/crypt/kk_crypt.h \
                ../../libs/src/classes/logger/kk_logger.h \
                ../../libs/src/classes/payload/kk_payload.h \
                ../../libs/src/constants/kk_constants.h \
                ../../libs/src/classes/user/kk_user.h \
                ../../libs/src/classes/task/kk_task.h \
                classes/access/accessdialog.h \
                classes/chat/kk_chat.h \
                classes/client/kk_client.h \
                classes/modal/kk_modal.h \
                classes/openfile/openfiledialog.h \
                classes/openfile/chooseavatardialog.h \
                classes/textedit/kk_cursor.h \
                classes/textedit/kk_editor.h \
                classes/textedit/kk_textedit.h

SOURCES         = ../../libs/src/classes/crdt/kk_crdt.cpp \
                ../../libs/src/classes/crdt/char/kk_char.cpp \
                ../../libs/src/classes/crdt/pos/kk_pos.cpp \
                ../../libs/src/classes/crdt/identifier/kk_identifier.cpp \
                ../../libs/src/classes/crypt/kk_crypt.cpp \
                ../../libs/src/classes/logger/kk_logger.cpp \
                ../../libs/src/classes/payload/kk_payload.cpp \
                ../../libs/src/classes/user/kk_user.cpp \
                ../../libs/src/classes/task/kk_task.cpp \
                classes/access/accessdialog.cpp \
                classes/chat/kk_chat.cpp \
                classes/client/kk_client.cpp \
                classes/modal/kk_modal.cpp \
                classes/textedit/kk_editor.cpp \
                classes/textedit/kk_textedit.cpp \
                main.cpp \
                classes/openfile/openfiledialog.cpp \
                classes/openfile/chooseavatardialog.cpp

RESOURCES += assets/client.qrc

FORMS += \
        ui/accessdialog.ui \
        ui/chooseavatardialog.ui \
        ui/chatdialog.ui \
        ui/openfiledialog.ui

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

EXAMPLE_FILES = textedit.qdoc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/klapklap
INSTALLS += target

DISTFILES +=

CONFIG += app_bundle

