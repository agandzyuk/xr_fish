#ifndef __fisher_mutex_h__
#define __fisher_mutex_h__

namespace Fisher {

///////////////////////////////////////////////////////////////////////
/// @note: we can change DEADLOCK_WAITER_LIMIT in fisher.h to redefine default deadlock timeout
#ifndef DEADLOCK_WAITER_LIMIT
    #define DEADLOCK_WAITER_LIMIT 1000 // 10 min
#endif

///////////////////////////////////////////////////////////////////////
class Mutex
{
public:
    Mutex();
    virtual ~Mutex();

    void Lock();
    void UnLock();
    bool TryLock();

protected:
    inline void CriticalSectionEnter();
    inline void CriticalSectionLeave();

private:
    long caller_;
    long owner_;
    long critical_section_;
    unsigned long handle_;
};

}
#endif  // __fisher_mutex_h__
