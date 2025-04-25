/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 ****************************************************/

#ifndef __QTSEMSHARE_H__
#define __QTSEMSHARE_H__

#include "qtsharemem.h"
#include "syssem.h"

using namespace  std;



// qt共享内存加linux信号 不继承线程类
template < class T >
class Sem_Qt_Data : public QTShareDataT< T >
{
  public:
    int         m_semId;
    int         m_bufSize;
    // int         m_num;
    int         m_created;
    int     *   m_pRd;
    int     *   m_pWr;
    int     *   m_pSize;
    key_t       m_semKey;

  public:
    Sem_Qt_Data():m_semId(-1),m_bufSize(0),m_created(0),
          m_pRd(NULL),m_pWr(NULL),m_pSize(NULL),m_semKey(19860610)
    {
        ;
    }
    explicit Sem_Qt_Data(int size, key_t semkey = 19860610, const QString & sharekey = "lhshare")
    {
        creat_sem_data(size, semkey, sharekey);
    }
    virtual ~Sem_Qt_Data()
    {
        zprintf3("destory Sem_Qt_Data!\n");
        cleanHandle();
    }

    void cleanHandle()
    {
        if(m_created)
        {
            zprintf1("sem_qtdata rm msem %d keyid %d!\n", m_semId, m_semKey);
            if(-1 != m_semId)
            {
                if(-1 == semctl(m_semId, 0, IPC_RMID, 0))
                {
                    perror("semctl IPC_RMID failed");
                    zprintf1("Sem_Qt_Data rm msem %d error code:%d!\n", m_semId, errno);
                }
                m_semId = -1;
            }
            m_created = 0;
        }
    }

    int creat_sem_data(int size, key_t semkey = 19860610, const QString & sharekey = "lhshare");
    int write_send_data(const T & val);
    int read_send_data(T& val);
    int a8_read_send_data(T& val);
    int wait_thread_sem(void);
};

template < class T >
int Sem_Qt_Data< T >::creat_sem_data(int size, key_t semkey, const QString & sharekey)
{
    int       err     = 0;
    int *     midp    = NULL;
    T*        midaddr = NULL;

    zprintf2("Sem_Qt_Data create sem semkey is %d!\n", semkey);

    int aline = Z_MEM_ALIGEN_SIZE(size, 4);
    int alinesize = Z_MEM_ALIGEN_SIZE(aline + sizeof(int) * 3, 4);

    midaddr = this->creat_data(alinesize, sharekey);
    m_bufSize = size / sizeof(T);

    zprintf2("Sem_Qt_Data create m_bufsize %d aline %d alinesize %d!\n", m_bufSize, aline, alinesize);

    if (midaddr != NULL)
    {
        ZLockerClass<Sem_Qt_Data< T >> locker(this);
        locker.lock();
        midp      = (int *)this->m_data;
        midp      += aline/4;
        m_pRd  = midp;
        m_pWr  = midp + 1;
        m_pSize  = midp + 1;
        *m_pRd = 0;
        *m_pWr = 0;
        *m_pSize = 0;

        m_semId     = new_create_sem(semkey, 0, m_created);
        if(m_semId <= 0)
        {
            zprintf1("Sem_Qt_Data create_sem %d error !\n",semkey);
        }
        else
            m_semKey = semkey;
        err = m_semId > 0 ? 0 : -2;
        // m_num = 0;
    }
    else
    {
        zprintf1("Sem_Qt_Data create share mem %s error !\n",sharekey.toStdString().c_str());
        err = -1;
    }
    return err;
}

template < class T >
int Sem_Qt_Data< T >::write_send_data(const T & val)
{
    ZLockerClass<Sem_Qt_Data< T >> locker(this);
    locker.lock();
    int count = *m_pSize;
    if(count >= m_bufSize)
    {
        zprintf1("Sem_Qt_Data write off \n");
        return -1;
    }
    int mid = *m_pWr;

    this->noblock_set_data(mid, val);

    mid++;
    mid %= m_bufSize;
    *m_pWr = mid;
    count++;
    *m_pSize = count;

    sem_v(m_semId);
    return 0;
}

template < class T >
int Sem_Qt_Data< T >::read_send_data(T & val)
{
    ZLockerClass<Sem_Qt_Data< T >> locker(this);
    locker.lock();

    int count = *m_pSize;
    if(count <= 0)
    {
        zprintf4("Sem_Qt_Data read data error!\n");
        return -1;
    }
    int mid = *m_pRd;

    this->noblock_get_data(mid, val);

    mid++;
    mid %= m_bufSize;
    *m_pRd = mid;
    count--;
    *m_pSize = count;

    return 0;
}
template < class T >
int Sem_Qt_Data< T >::a8_read_send_data(T & val)  //专门为a8系统，没有信号量互斥锁准备的读函数
{
    ZLockerClass<Sem_Qt_Data< T >> locker(this);
    locker.lock();

    int mid = *m_pRd;
    int count = * m_pSize;
    int i = 0;
    int zs = get_sem_count(m_semId);

    if(count <= 0)
    {
        zprintf3("Sem_Qt_Data a8 read data error!\n");
        return -1;
    }
    if(count < (zs + 1))
    {
        zprintf3("Sem_Qt_Data a8 count error!\n");
        return -2;
    }

    i = count -zs;

    this->noblock_get_data(mid, val);

    mid++;
    mid %= m_bufSize;
    *m_pRd = mid;
    count--;
    *m_pSize = count;
    i--;

    return i;

}

template < class T >
int Sem_Qt_Data< T >::wait_thread_sem(void)
{
    if (sem_p(m_semId) == 0)
        return 0;
    else
        return -1;
}

// qt共享内存加linux信号 继承线程类
template < class T, class FAT >
class Sem_QtPth_Data : public Sem_Qt_Data< T >, public Call_B_T< T, FAT >
{
  public:
    Sem_QtPth_Data()
    {
        ;
    }
    virtual ~Sem_QtPth_Data()
    {
        zprintf3("destory Sem_QtPth_Data!\n");
        this->running = 0;
    }

    void run();
};

template < class T, class FAT >
void Sem_QtPth_Data< T, FAT >::run(void)
{
    zprintf3("Sem_QtPth_Data run glic ver %d.%d!\n", __GLIBC__, __GLIBC_MINOR__);

    while (this->running)
    {
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 18)  // glibc ≥ 2.4 的代码逻辑
        if (sem_p(this->m_semId) == 0)
        {

            while(this->running)
            {
                T   val;
                int err = this->read_send_data(val);
                if ( err >= 0)
                {
                    if (this->z_callbak != NULL) //执行操作
                    {
                        this->z_callbak(this->father, val);
                    }
                    if(err == 0)
                        break;
                }
                else
                    break;
            }
        }
        else
        {
            pthread_testcancel();
        }
#else
        if (sem_p(this->m_semId, 10) == 0)
        {

            while(this->running)
            {
                T   val;
                int err = this->a8_read_send_data(val);
                if ( err >= 0)
                {
                    if (this->z_callbak != NULL) //执行操作
                    {
                        this->z_callbak(this->father, val);
                    }
                    if(err == 0)
                        break;
                }
                else
                    break;
            }
        }
        else
        {
            pthread_testcancel();
        }
#endif
    }
    pthread_exit(NULL);
}

#endif /*__SEMSHARE_H__*/
