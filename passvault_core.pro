#-------------------------------------------------
#
# Project created by QtCreator 2019-06-03T23:01:34
#
#-------------------------------------------------

QT       += network gui core

#QT       -= gui

TARGET = passvault_core
TEMPLATE = lib
CONFIG += staticlib c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        account.cpp \
        accountsstore.cpp \
        opensslaesengine.cpp \
        passvault_core.cpp \
        passwordgenerator.cpp \
        settings.cpp \
        sync.cpp \
        utils.cpp

HEADERS += \
        account.h \
        accountsstore.h \
        opensslaesengine.h \
        passvault_core.h \
        passwordgenerator.h \
        settings.h \
        statusdefs.h \
        sync.h \
        utils.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

macx: LIBS += -L$$PWD/../../../../../../opt/openssl/openssl-1.1.1c_install/lib/ -lcrypto

INCLUDEPATH += $$PWD/../../../../../../opt/openssl/openssl-1.1.1c_install/include
DEPENDPATH += $$PWD/../../../../../../opt/openssl/openssl-1.1.1c_install/include

macx: PRE_TARGETDEPS += $$PWD/../../../../../../opt/openssl/openssl-1.1.1c_install/lib/libcrypto.a

DISTFILES += \
    cert.pem

RESOURCES += \
    ssl_certificate.qrc
