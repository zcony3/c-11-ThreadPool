#include <deque>
#include <future>
#include <mutex>
#include <functional>
#include <atomic>
#include <vector>
#include <condition_variable>

class TaskPool{
private:
    std::atomic_bool m_stop;
    using Queue = std::deque<std::function<void()>>;
    Queue m_taskQueue;
    std::mutex m_mutex;
    std::condition_variable m_taskChanged;
    std::vector<std::thread> m_works;
public:
    explicit TaskPool(size_t size);
    ~TaskPool();
    TaskPool(const TaskPool& ) = delete;
    TaskPool& operator=(const TaskPool&) = delete;
    template <typename Task, typename... Args>
    auto push(bool front, Task&& task, Args&&... args) -> std::future<decltype(task(args...))>;
};

TaskPool::TaskPool(size_t size):m_stop{false}
{
    for(size_t i=0; i<size; ++i){
        m_works.emplace_back(
            [this](){
                while(1){
                    auto canGoDown = [this](){return m_stop || !m_taskQueue.empty();};
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_taskChanged.wait(lock,canGoDown);
                        if(m_stop)
                            break;
                        task = m_taskQueue.front();
                        m_taskQueue.pop_front();
                    }
                    if(task)
                        task();
                }
            }
        );
    }
}

TaskPool::~TaskPool()
{
    m_stop = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskQueue.clear();
        m_taskChanged.notify_all();
    }
    for(auto &work:m_works){
        if(work.joinable()){
            work.join();
        }
    }
}

template <typename Task, typename... Args>
auto TaskPool::push(bool front, Task&& task, Args&&... args) -> std::future<decltype(task(args...))>
{
    auto bindTask = std::bind(std::forward<Task>(task),std::forward<Args>(args)...);
    auto pask_task = std::make_shared<std::packaged_task<decltype(bindTask())()>>(bindTask);

    auto operator_task = [pask_task](){
        pask_task->operator()();
    };

    {
        // if(m_stop)
        //     std::__throw_runtime_error("taskPool has stopped!");
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskQueue.emplace(front?m_taskQueue.begin():m_taskQueue.end(),operator_task);
        m_taskChanged.notify_all();
    }
    return pask_task->get_future();
}
