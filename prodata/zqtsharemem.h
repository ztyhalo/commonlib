#ifndef ZQTSHAREMEM_H
#define ZQTSHAREMEM_H

#include <QVector>
#include <QString>
#include <QSharedMemory>
#include "zprint.h"
using namespace std;

class ZQTShareMem
{
public:
    QSharedMemory lhshare;
    QString       m_shmKey;

public:
    ZQTShareMem():m_shmKey("")
    {
        lhshare.setKey(m_shmKey);
    }
    virtual ~ZQTShareMem()
    {
        zprintf3("destory ZQTShareMem!\n");
        if (lhshare.isAttached())
        {
            zprintf3("qt share have attach!\n");
            lhshare.detach();
        }
    }
    bool    destory();
    int     creat_data(int size);
    void*   creat_data(int size, const QString & keyid);
    void    lock(void);
    void    unlock(void);
    int     read_creat_data(int size, const QString & keyid);

};

#endif // ZQTSHAREMEM_H
