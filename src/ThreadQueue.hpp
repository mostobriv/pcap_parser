#pragma once

// Description: thread-safe queue a-la python: popping from empty queue locks


#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>


template<typename T>
class ThreadQueue
{
    private:
        std::queue<T>                   m_queue;
        mutable std::condition_variable m_cond;
        mutable std::mutex              m_mutex;

    public:
        using queue_type      = std::queue<T>;
        using container_type  = typename queue_type::container_type;
        using value_type      = typename queue_type::value_type;
        using size_type       = typename queue_type::size_type;
        using reference       = typename queue_type::reference;
        using const_reference = typename queue_type::const_reference;

        ThreadQueue() = default;
        ThreadQueue(const ThreadQueue&) = delete;
        ThreadQueue(ThreadQueue&&) = delete;

        bool      empty() const;
        size_type size()  const;

        // Copies element. Immediately returns when empty
        std::optional<value_type> front() const;

        template<typename... Args>
        void emplace(Args&&...);
        void push(value_type&&);

        value_type pop();
        // Pop with a timeout
        template<typename Rep, typename Period>
        std::optional<value_type> pop(const std::chrono::duration<Rep, Period>&);
};


template<typename T>
bool ThreadQueue<T>::empty() const
{
    std::lock_guard _lock (m_mutex);

    return m_queue.empty();
}

template<typename T>
auto ThreadQueue<T>::size() const -> size_type
{
    std::lock_guard _lock (m_mutex);

    return m_queue.size();
}


template<typename T>
auto ThreadQueue<T>::front() const -> std::optional<value_type>
{
    std::lock_guard _lock (m_mutex);

    if (empty())  return {};
    return std::make_optional(m_queue.front());
}


template<typename T> template<typename... Args>
void ThreadQueue<T>::emplace(Args&&... args)
{
    std::lock_guard _lock (m_mutex);

    m_queue.emplace(std::forward<Args>(args)...);
}

template<typename T>
void ThreadQueue<T>::push(value_type&& value)
{
    std::lock_guard _lock (m_mutex);

    m_queue.push(std::forward<T>(value));
    m_cond.notify_one();
}


template<typename T>
auto ThreadQueue<T>::pop() -> value_type
{
    std::unique_lock _lock (m_mutex);

    // who the fuck thought spurious releases were a good idea?
    while (m_queue.empty()) { m_cond.wait(_lock); }
    value_type elem = std::move(m_queue.front()); //explicitly non-reference
    m_queue.pop();
    return elem;
}


template<typename T> template<typename Rep, typename Period>
auto ThreadQueue<T>::pop(const std::chrono::duration<Rep, Period>& timeout)
    -> std::optional<value_type>
{
    std::unique_lock _lock (m_mutex);

    bool spurious = true;
    while (spurious) {
        auto release = m_cond.wait_for(_lock, timeout);
        // Three possibilities: timeouted on wait, the condition released, or
        // spurious release happened.
        // In first case, return Nothing.
        // In second case, exit the cycle and get real element.
        // Spurious interrupt is when the release was not on timeout and the
        // condition still doesn't hold.
        // Again, why the fuck did they create a standard with spurious releases?
        if (release == std::cv_status::timeout) {
            return {};
        } else if (m_queue.empty()) {
            spurious = true;
        } else {
            // queue not empty
            spurious = false;
        }
    }
    value_type elem = std::move(m_queue.front()); //explicitly non-reference
    m_queue.pop();
    return elem;
}
