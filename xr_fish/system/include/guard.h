#ifndef __fisher_guard_h__
#define __fisher_guard_h__

#include "mutex.h"

namespace Fisher {

///////////////////////////////////////////////////////////////////////
/// @note: we can change LOCKING_PER_THREAD_MAX in fisher.h to redefine ceiling for locks
/// lock with number what exceed ceiling will be skiped
///////////////////////////////////////////////////////////////////////
#ifndef LOCKING_PER_THREAD_MAX
    #define LOCKING_PER_THREAD_MAX 0xFF
#endif

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
/// atomic access (e.g. CriticalSection )
/// exclusive read and write access
class Guard
{
public:
    Guard(const Mutex& lock);
    
protected:
    ~Guard();

private:
    const Mutex& lock_;
};

///////////////////////////////////////////////////////////////////////
/// readwrite lock 
/// owner has exclusive write access
class RWGuard
{
public:
    RWGuard(const Mutex& lock);
    
protected:
    ~RWGuard();

private:
    const Mutex& lock_;
};

///////////////////////////////////////////////////////////////////////
/// prioritized MagicGuard
/// exclusive write / exclusive read
/// suspends all active accessors for own operation
class MagicGuard
{
public:
    MagicGuard(const Mutex& lock);
    
protected:
    ~MagicGuard();

private:
    const Mutex& lock_;
};

}

#endif  // __fisher_guard_h__