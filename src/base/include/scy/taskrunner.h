//
// LibSourcey
// Copyright (C) 2005, Sourcey <http://sourcey.com>
//
// LibSourcey is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// LibSourcey is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//


#ifndef SCY_TaskRunner_H
#define SCY_TaskRunner_H


#include "scy/uv/uvpp.h"
#include "scy/memory.h"
#include "scy/interface.h"
#include "scy/signal.h"
#include "scy/taskrunner.h"
#include "scy/idler.h"


namespace scy {

    
class TaskRunner;


class Task: public async::Runnable
    /// This class is for implementing any kind 
    /// async task that is compatible with a TaskRunner.
{
public:    
    Task(bool repeat = false);
    
    virtual void destroy();
        // Sets the task to destroyed state.

    virtual bool destroyed() const;
        // Signals that the task should be disposed of.

    virtual bool repeating() const;
        // Signals that the task's should be called
        // repeatedly by the TaskRunner.
        // If this returns false the task will be cancelled()

    virtual UInt32 id() const;
        // Unique task ID.
    
    // Inherits async::Runnable:
    //
    // virtual void run();
    // virtual void cancel();
    // virtual bool cancelled() const;
    
protected:
    Task(const Task& task);
    Task& operator=(Task const&);

    virtual ~Task();
        // Should remain protected.

    virtual void run() = 0;    
        // Called by the TaskRunner to run the task.
        // Override this method to implement task action.
        // Returning true means the true should be called again,
        // and false will cause the task to be destroyed.
        // The task will similarly be destroyed id destroy()
        // was called during the current task iteration.

    friend class TaskRunner;
        // Tasks belong to a TaskRunner instance.

    UInt32 _id;
    bool _repeating;
    bool _destroyed;
};

    
class TaskRunner: public async::Runnable
    // The TaskRunner is an asynchronous event loop in 
    // charge of running one or many tasks. 
    //
    // The TaskRunner continually loops through each task in
    // the task list calling the task's run() method.
{
public:
    TaskRunner(async::Runner::Ptr runner = nullptr);
    virtual ~TaskRunner();
    
    virtual bool start(Task* task);
        // Starts a task, adding it if it doesn't exist.

    virtual bool cancel(Task* task);
        // Cancels a task.
        // The task reference will be managed the TaskRunner
        // until the task is destroyed.

    virtual bool destroy(Task* task);
        // Queues a task for destruction.

    virtual bool exists(Task* task) const;
        // Returns weather or not a task exists.

    virtual Task* get(UInt32 id) const;
        // Returns the task pointer matching the given ID, 
        // or nullptr if no task exists.

    virtual void setRunner(async::Runner::Ptr runner);
        // Set the asynchronous context for packet processing.
        // This may be a Thread or another derivative of Async.
        // Must be set before the stream is activated.

    static TaskRunner& getDefault();
        // Returns the default TaskRunner singleton, although
        // TaskRunner instances may be initialized individually.
        // The default runner should be kept for short running
        // tasks such as timers in order to maintain performance.
    
    NullSignal Idle;    
        // Fires after completing an iteration of all tasks.

    NullSignal Shutdown;
        // Fires when the TaskRunner is shutting down.
    
    virtual const char* className() const { return "TaskRunner"; }
        
protected:
    virtual void run();
        // Called by the async context to run the next task.
    
    virtual bool add(Task* task);
        // Adds a task to the runner.
    
    virtual bool remove(Task* task);
        // Removes a task from the runner.

    virtual Task* next() const;
        // Returns the next task to be run.
    
    virtual void clear();
        // Destroys and clears all manages tasks.
        
    virtual void onAdd(Task* task);
        // Called after a task is added.
        
    virtual void onStart(Task* task);
        // Called after a task is started.
        
    virtual void onCancel(Task* task);
        // Called after a task is cancelled.
    
    virtual void onRemove(Task* task);
        // Called after a task is removed.
    
    virtual void onRun(Task* task);
        // Called after a task has run.

protected:
    typedef std::deque<Task*> TaskList;
    
    mutable Mutex    _mutex;
    TaskList        _tasks;
    async::Runner::Ptr _runner;
};


} // namespace scy


#endif // SCY_TaskRunner_H
