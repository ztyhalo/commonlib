#include "zqtsharemem.h"
#include <sys/ipc.h>
#include <sys/shm.h>

bool ZQTShareMem::destory(void)
{
    key_t unix_key;
    zprintf2("QT_Share_MemT is exist key:%s nativekey: %s!\n", lhshare.key().toStdString().c_str(),
             lhshare.nativeKey().toStdString().c_str());
    unix_key = ftok(lhshare.nativeKey().toStdString().c_str(), 'Q');
    if(unix_key == -1)
        return true;

    int id = shmget(unix_key, 0, 0400);

    zprintf1("get unix_key %d id %d!\n", unix_key, id);

    struct shmid_ds shmid_ds;
    if (0 != shmctl(id, IPC_STAT, &shmid_ds)) {
        switch (errno) {
        case EINVAL:
            return true;
        default:
            return false;
        }
    }
    if (shmid_ds.shm_nattch == 0) {
        // mark for removal
        if (-1 == shmctl(id, IPC_RMID, &shmid_ds)) {
            zprintf1("remove error %d!\n", errno);
            switch (errno) {
            case EINVAL:
                return true;
            default:
                return false;
            }
        }

        // remove file
        if (!remove(lhshare.nativeKey().toStdString().c_str()))
            return false;
    }
    return true;
}

int ZQTShareMem::creat_data(int size)
{

    lhshare.setKey(m_shmKey);
    if (lhshare.isAttached())
    {
        zprintf2("qt share have attach!\n");
        lhshare.detach();
    }
    destory();

    if (!lhshare.create(size))
    {
        zprintf2("qt share %s creat fail!\n", m_shmKey.toStdString().c_str());

        if (lhshare.error() == QSharedMemory::AlreadyExists) //已经存在
        {
            if (!lhshare.attach())
            {
                zprintf2("can't attatch qt share!\n");
                return -2;
            }
            if (lhshare.isAttached())
            {
                zprintf2("qt share test attach!\n");
            }
        }
        else
        {
            zprintf1("QT_Share_MemT creat data fail!\n");
            return -1;
        }
    }


    return 0;
}

int ZQTShareMem::read_creat_data(int size, const QString & keyid)
{
    m_shmKey = keyid;

    lhshare.setKey(m_shmKey);

    if (!lhshare.attach())
    {
        zprintf1("can't attatch qt share\n");
        return -1;
    }




    return 0;
}

void* ZQTShareMem::creat_data(int size, const QString & keyid)
{
    m_shmKey = keyid;
    if (creat_data(size) == 0)

    return NULL;
}
