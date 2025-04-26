# CONFIG -= qt

TEMPLATE = lib
# DEFINES += COMMONLIB_LIBRARY
TARGET = zcommonlib
# CONFIG += c++17
QT       -= gui
QT       += core xml network
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH = .\
              bufmodel\
              date\
              epoll\
              mutex\
              prodata\
              prodata/sem\
              reflect\
              sempro\
              sigslot\
              socket\
              tcp\
              timer\
              udp\
              file\
              zprint


SOURCES += \
    bufmodel/ZBufModel.cpp \
    date/com_date.cpp \
    file/zfile.cpp \
    prodata/sem/syssem.cpp \
    prodata/zlockerclass.cpp \
    prodata/zqtsharemem.cpp \
    prodata/zsystemsem.cpp \
    socket/socket.cpp \
    tcp/tcp_class.cpp \
    tcp/tcp_client.cpp \
    timer/timers.cpp \
    udp/udp.cpp \
    zprint/netprint.cpp \
    zprint/zprint.cpp

HEADERS += \
    bufmodel/ZBufModel.h \
    date/com_date.h \
    epoll/e_poll.h \
    file/zfile.h \
    mutex/mutex_class.h \
    prodata/clist.h \
    prodata/pro_data.h \
    prodata/ptdataapp.h \
    prodata/qtsemshare.h \
    prodata/qtsharemem.h \
    prodata/sem/syssem.h \
    prodata/semshare.h \
    prodata/sharemem.h \
    prodata/zlockerclass.h \
    prodata/zmap.h \
    prodata/zmsg.h \
    prodata/zqtsharemem.h \
    prodata/zsystemsem.h \
    reflect/reflect.h \
    reflect/xmlprocess.h \
    sempro/semprocess.h \
    sigslot/sigslot.h \
    socket/socket.h \
    tcp/tcp_class.h \
    tcp/tcp_client.h \
    timer/timers.h \
    udp/udp.h \
    zprint/netprint.h \
    zprint/version.h \
    zprint/zprint.h

LIBS += -lssl -lcrypto
# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
