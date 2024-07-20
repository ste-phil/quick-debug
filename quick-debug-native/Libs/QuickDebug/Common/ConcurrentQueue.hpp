#include <condition_variable> 
#include <iostream>  
#include <mutex> 
#include <queue> 
  
// Thread-safe queue 
template <typename T> 
class ConcurrentQueue { 
public: 
    ConcurrentQueue() {}

    size_t Size() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void Push(T item) 
    { 
        std::unique_lock<std::mutex> lock(m_mutex); 

        m_queue.push(item); 
  
        // Notify one thread that 
        // is waiting 
        m_cond.notify_one(); 
    } 
  
    T Pop() 
    { 
        std::unique_lock<std::mutex> lock(m_mutex); 

        // wait until queue is not empty 
        m_cond.wait(lock, [this]() { 
            return !m_queue.empty(); 
        }); 
  
        // retrieve item 
        T item = m_queue.front(); 
        m_queue.pop(); 
  
        // return item 
        return item; 
    } 


private: 
    std::queue<T> m_queue; 
    std::mutex m_mutex; 
    std::condition_variable m_cond; 
}; 
  