#Application version
VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 1

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"
QT_DEBUG_PLUGINS=1

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

macx:ICON = $${PWD}/klapklap.icns

QT +=   core \
        widgets \
        websockets \
        network\
        xml \
        sql

CONFIG   += staticlib

requires(qtConfig(filedialog))
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app

TARGET          = KlapKlap_Server

HEADERS         = ../../libs/src/classes/crdt/kk_crdt.h \
                ../../libs/src/classes/crdt/char/kk_char.h \
                ../../libs/src/classes/crdt/pos/kk_pos.h \
                ../../libs/src/classes/crdt/identifier/kk_identifier.h \
                ../../libs/src/classes/crypt/kk_crypt.h \
                ../../libs/src/classes/payload/kk_payload.h \
                ../../libs/src/classes/logger/kk_logger.h \
                ../../libs/src/constants/kk_constants.h \
                ../../libs/src/classes/user/kk_user.h \
                ../../libs/src/classes/task/kk_task.h \
                classes/server/participant/kk_participant.h \
                classes/smtp/mime/SmtpMime.h \
                classes/smtp/mime/emailaddress.h\
                classes/smtp/mime/mimecontentformatter.h\
                classes/smtp/mime/mimehtml.h\
                classes/smtp/mime/mimemessage.h\
                classes/smtp/mime/mimemultipart.h\
                classes/smtp/mime/mimepart.h\
                classes/smtp/mime/mimetext.h\
                classes/smtp/mime/quotedprintable.h\
                classes/smtp/mime/smtpclient.h\
                classes/smtp/kk_smtp.h\
                classes/db/kk_db.h \
                classes/server/kk_server.h \
                classes/server/file/kk_file.h \
                classes/server/filesys/kk_filesys.h \
                classes/server/room/kk_room.h \
                classes/server/session/kk_session.h

SOURCES         = ../../libs/src/classes/crdt/kk_crdt.cpp \
                ../../libs/src/classes/crdt/char/kk_char.cpp \
                ../../libs/src/classes/crdt/pos/kk_pos.cpp \
                ../../libs/src/classes/crdt/identifier/kk_identifier.cpp \
                ../../libs/src/classes/crypt/kk_crypt.cpp \
                ../../libs/src/classes/logger/kk_logger.cpp \
                ../../libs/src/classes/payload/kk_payload.cpp \
                ../../libs/src/classes/user/kk_user.cpp \
                ../../libs/src/classes/task/kk_task.cpp \
                classes/smtp/mime/emailaddress.cpp \
                classes/smtp/mime/mimecontentformatter.cpp \
                classes/smtp/mime/mimehtml.cpp \
                classes/smtp/mime/mimemessage.cpp \
                classes/smtp/mime/mimemultipart.cpp \
                classes/smtp/mime/mimepart.cpp \
                classes/smtp/mime/mimetext.cpp \
                classes/smtp/mime/quotedprintable.cpp \
                classes/smtp/mime/smtpclient.cpp \
                classes/smtp/kk_smtp.cpp\
                classes/db/kk_db.cpp \
                classes/server/participant/kk_participant.cpp \
                classes/server/kk_server.cpp \
                classes/server/file/kk_file.cpp \
                classes/server/filesys/kk_filesys.cpp \
                classes/server/room/kk_room.cpp \
                classes/server/session/kk_session.cpp \
                main.cpp

RESOURCES += assets/server.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

EXAMPLE_FILES = klapklap_be.qdoc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/klapklap_be
INSTALLS += target
