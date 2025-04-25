#ifndef __SIGSLOT_H__
#define __SIGSLOT_H__

#include <vector>
#include "zprint.h"

using namespace std;

// PARA_TMP 数据参数模板 能够表示不同的回调参数类型

template < class PARA_TMP >
class ZSlotBase
{
  public:
    virtual void Exec(PARA_TMP param1) = 0;
    virtual ~ZSlotBase()               = 0;
};
// SLOT_OWN 操函数的拥有者

template < class SLOT_OWN, class PARA_TMP >
class ZSlotImpl : public ZSlotBase< PARA_TMP >
{
  public:
    explicit ZSlotImpl(SLOT_OWN* pObj, void (SLOT_OWN::*func)(PARA_TMP))
    {
        m_pObj = pObj;
        m_Func = func;
    }

    void Exec(PARA_TMP param1) override
    {
        (m_pObj->*m_Func)(param1);
    }

  private:
    SLOT_OWN* m_pObj;
    void (SLOT_OWN::*m_Func)(PARA_TMP);
};

template < class PARA_TMP >
class ZSlot
{
private:
    ZSlotBase<PARA_TMP> * m_pSlotBase;

public:
    template < class SLOT_OWN >

    ZSlot(SLOT_OWN* pObj, void (SLOT_OWN::*func)(PARA_TMP))
    {
        m_pSlotBase = new ZSlotImpl< SLOT_OWN, PARA_TMP >(pObj, func);
    }

    ZSlot(const ZSlot & other)
    {
        if(other.m_pSlotBase != NULL)
            this->m_pSlotBase = new ZSlotBase<PARA_TMP>(*other.m_pSlotBase);
    }

    ZSlot& operator=(const ZSlot& other)
    {
        if(this != &other)
        {
            delete m_pSlotBase;
            this->m_pSlotBase = new ZSlotBase<PARA_TMP>(*other.m_pSlotBase);
        }
        return *this;
    }
    virtual ~ZSlot()
    {
        zprintf3("ZSlot destruct!\n");
        if(m_pSlotBase != 0)
        {
            delete m_pSlotBase;
            m_pSlotBase = 0;
        }
    }

    void Exec(PARA_TMP param1)
    {
        m_pSlotBase->Exec(param1);
    }


};

template < class PARA_TMP >
class ZSignal
{

private:
    vector< ZSlot< PARA_TMP >* > m_pSlotSet;

public:
    template < class SLOT_OWN >
    void Bind(SLOT_OWN* pObj, void (SLOT_OWN::*func)(PARA_TMP))
    {
        m_pSlotSet.push_back(new ZSlot< PARA_TMP >(pObj, func));
    }

    virtual ~ZSignal()
    {
        zprintf3("ZSignal destruct!\n");
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            delete m_pSlotSet[i];
        }
    }

    void operator()(PARA_TMP param1)
    {
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            m_pSlotSet[i]->Exec(param1);
        }
    }


};

#define P_Connect(sender, signal, receiver, method) ((sender)->signal.Bind(receiver, method))

// PARA_TMP 数据参数模板 能够表示不同的回调参数类型

class No_SlotBase
{
  public:
    virtual void Exec(void);
    virtual ~No_SlotBase()
    {
    }
};
// SLOT_OWN 操函数的拥有者

template < class SLOT_OWN >
class No_SlotImpl : public No_SlotBase
{
  public:
    No_SlotImpl(SLOT_OWN* pObj, void (SLOT_OWN::*func)(void))
    {
        m_pObj = pObj;
        m_Func = func;
    }

    void Exec(void) override
    {
        (m_pObj->*m_Func)();
    }

  private:
    SLOT_OWN* m_pObj;
    void (SLOT_OWN::*m_Func)(void);
};

class No_Slot
{
  public:
    template < class SLOT_OWN >
    No_Slot(SLOT_OWN* pObj, void (SLOT_OWN::*func)(void))
    {
        m_pSlotBase = new No_SlotImpl< SLOT_OWN >(pObj, func);
    }

    No_Slot(const No_Slot & other)
    {
        if(other.m_pSlotBase != NULL)
            this->m_pSlotBase = new No_SlotBase(*other.m_pSlotBase);
    }

    No_Slot& operator=(const No_Slot& other)
    {
        if(this != &other)
        {
            delete m_pSlotBase;
            this->m_pSlotBase = new No_SlotBase(*other.m_pSlotBase);
        }
        return *this;
    }


    virtual ~No_Slot()
    {
        if (m_pSlotBase != nullptr)
        {
            delete m_pSlotBase;
            m_pSlotBase = nullptr;
        }
    }

    void Exec(void)
    {
        m_pSlotBase->Exec();
    }

  private:
    No_SlotBase* m_pSlotBase;
};

class No_Signal
{
  public:
    template < class SLOT_OWN >
    void Bind(SLOT_OWN* pObj, void (SLOT_OWN::*func)(void))
    {
        m_pSlotSet.push_back(new No_Slot(pObj, func));
    }

    virtual ~No_Signal()
    {
        zprintf3("No_Signal destruct!\n");
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            delete m_pSlotSet[i];
        }
    }

    void operator()(void)
    {
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            m_pSlotSet[i]->Exec();
        }
    }

  private:
    vector< No_Slot* > m_pSlotSet;
};

#endif /*__SIGSLOT_H__*/
