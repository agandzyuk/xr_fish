#ifdef INCLUDE_TASK_POOL

#ifndef __fisher_tasks_pool_h__
#define __fisher_tasks_pool_h__

#include "fisher.h"

#include <queue>

namespace Fisher {

class Task;

/**
 void rotate()
 ===========
                              front                                  back
  TasksPool
   --T(b)->                   Task(a)      Task(b)      Task(c)      Task(d)
  |        |                  ------       ------       ------       ------
  T(c)     T(a)-- rotate --> * exec | <-- | exec | <-- | exec | <-- | exec |<-- * 
  |        :                 :      |     |      |     |      |     |      |    |
  T(d)     * <--- rotate --- * retn |     | retn |     | retn |     | retn |    |
  ...      |                  ------       ------       ------       ------     |
  Tx       |                    get                                             push_back
   <-------* ---- pop_front -->Task(a)------------------------------------------>



 note: rotate() is the sync method
       so next next rotate() will be executed after the current rotate() returned
       indexes a,b,c,d,...etc has relative values on pic
       are's values changing after each rotate() by strategy Task(x) = follow(), see below
**/

/**
 Task* follow()
 =============                    Task(1):priority(a)
                                   ...
                                Task(X-2):priority(b)
                                    |
 Thread(X)                      Task(X-1):priority(c)          TasksPool
  _                           ______|______                       _ 
 | |                         |   Task(X)   | )---- follow(1) --> | | Task(X)
 | |--> Task::exec() beg --->x.............x                     | |
 | |                         |  owned      |                     | |
 | |                         |             | )---- follow(2) --> | | Task(X or X+1 or Z)
 | |                         |             |                     | |
 | |<-- Task::exec() end <---x.............x                     | |
 | |                         | priority(d) | )---- follow(3) --> |_| Task(X+1)
 |_|                         |_____________|
                                    |
                                Task(X+1):priority(e)
                                   ...
                                  Task(0):priority(f)

note: each Task will be tested on first before Test::exec()
      the rotate() may be skipped or executed in condition of Task priority
      zero value of follow() suspends Pool without any doubts and following rotate()
      this means that pool will be resumed from another place, only under other context control
            even when nuclear blast behind the window!
      
on the pic we have three scenario:
1) get_next() before Thread(X) aquire Task(x), after Task(X-1)
2) get_next() in time of Task(X) is busy
3) get_next() after Task(X) released, before the next Task(X+1) is ready to be executed

scenarios 1 and 3 are similars, but scenario 2 can be with a triple functionality
1) waits while Task(X) will be released then returns it 
   (by default)
2) skips Task(X) by returning the primary-met free Task
   (ThreadMode::Timed)
3) verify Task(X) priority then returns or X, or X+1 or Z by comparison with a certain priority limitation
   (ThreadMode::SmartTimed )

**/

struct ThreadMode : public ThreadState 
{
    static const char Idle       = 16;  // set_mode does not affect Timed and SmartTimed 
    static const char Timed      = 32;  // mode switches to Timed during thread working
    static const char SmartTimed = 64;  // mode switches to SmartTimed during thread working
                                        // but have to check the interval is non-zero 
};

/// class TasksPool ////////////////////////////////////////
class TasksPool : private std::queue<Task>, public Task
{
public:
    TasksPool();
    ~TasksPool();

    Task* follow() const;
    void  rotate();

    // std::queue
    void  push(Task* task);

    void  set_mode(ThreadMode mode);  // change ThreadMode flag for object without effects
    ThreadMode get_mode() const;        // modes Timed and SmartTimed returns in combined with 
    void  set_idle();

    // Task implementation
    virtual void run();
    virtual void suspend();
    virtual void resume();
    virtual void cancel();
    virtual void terminate();

    virtual ThreadState get_state() const;      // Idle, Timed, SmartTimed flags auto-excludes with ThreadState casting
    virtual long get_priority() const;          // positive or negative values or zero what normal
    virtual void set_priority(long priority);   // priority use for SmartTimed mode
    virtual long get_interval() const;          // [ms] period for thread sleep before one iteration 
                                                // when tasks are passes from beginning to end of the internal queue
    virtual void set_interval(long interval);   // [ms ] interval use for Timed, SmartTimed and Idle modes


private:
};


}
#endif  // __fisher_tasks_pool_h__
#endif //INCLUDE_TASK_POOL