#ifndef __NETPRINT_H__
#define __NETPRINT_H__

#include <QObject>
#include <QtNetwork>
#include <QString>
#include <string>
#include <QByteArray>
#include <time.h>
#include <sys/time.h>
#include <QMap>
#include <QHash>
#include <semaphore.h>
#include "epoll/e_poll.h"
#include <string>
using namespace std;

class UPDATE_SOCKET:public QObject
{
    Q_OBJECT
private:
    quint16         upsendport;
    quint16         uprecport;
public:
    QUdpSocket *    upsocket;

public slots:
    void updateDataProcess();
signals:
    void data_receive(const QByteArray & data, const QHostAddress & host);


public:
    UPDATE_SOCKET(quint16 sendp = 0xfff1,
                  quint16 recvp = 0xfff2);

    ~UPDATE_SOCKET()
    {
        if(upsocket != NULL)
            delete upsocket;
    }
    int send_data(const QString & msg);
    int send_data(const QString & msg, const QString & ip);
    int send_data(const QString & msg, const QHostAddress & ip);
    virtual int up_data_process(const QByteArray & data, const QHostAddress & host);
};

struct Print_Time
{
    struct timeval tv;
    bool operator <(const Print_Time & rhs) const {

             if(tv.tv_sec < rhs.tv.tv_sec)
                 return true;
             else if(tv.tv_sec == rhs.tv.tv_sec)
             {
                 if(tv.tv_usec < rhs.tv.tv_usec)
                     return true;
             }
             return false;

        }
};


class Print_Server:public UPDATE_SOCKET,public Pth_Class,public MUTEX_CLASS
{
    Q_OBJECT
public:
    QHostAddress  printhost;
    int      constate;
    QMap<struct Print_Time, QString > info;
    sem_t                psem;
public:
    Print_Server(quint16 sendp = 0xf418, quint16 recvp = 0xf419);
    void  netprintf(const char * format, ...);
signals:

public slots:

public:
    int up_data_process(const QByteArray & data, const QHostAddress & host);
    void run();
};


class Print_Client:public UPDATE_SOCKET
{
    Q_OBJECT

public:
    Print_Client(quint16 sendp = 0xf419, quint16 recvp = 0xf418);

signals:

public slots:

public:
    int up_data_process(const QByteArray & data, const QHostAddress & host);

};

extern Print_Server * gNetP;

//#define nprintf
#define nprintf gNetP->netprintf
//void  nprintf(const char * format, ...);

#endif //__NETPRINT_H__
