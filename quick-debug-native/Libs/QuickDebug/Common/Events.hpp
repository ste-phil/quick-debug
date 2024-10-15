//
// Created by PhilippSteininger on 08/10/2024.
//

#ifndef QD_COMMON_EVENTS_H
#define QD_COMMON_EVENTS_H

#include <vector>
#include <functional>
#include "Dbg.hpp"

namespace QD {
    template<typename T>
    class GlobalEvent {
    public:
        static inline void AddListener(std::function<void(const T)> listener) {
            m_listeners.push_back(listener);
        }

        static inline void RemoveListener(std::function<void(const T)> listener) {
            std::ranges::remove(m_listeners, listener);
        }

        static inline void Invoke(T eventData) {
            for (size_t i = 0; i < m_listeners.size(); ++i) {
                try {
                    m_listeners[i](eventData);
                } catch (const std::exception &e) {DEBUG_PRINT("[QD.GlobalEvent] Exception in listener: %s", e.what());
                }
            }
        }

    private:
        GlobalEvent() {}

        ~GlobalEvent() {}


    private:
        static inline std::vector<std::function<void(const T)>> m_listeners{};
    };

    template<typename T>
    class Event {
    public:
        inline void AddListener(const std::function<void(const T)> listener) {
            m_listeners.push_back(listener);
        }

        inline void RemoveListener(const std::function<void(const T)> listener) {
            std::ranges::remove(m_listeners, listener);
        }

        Event &operator+=(const std::function<void(const T)> rhs) {
            AddListener(rhs);
            return *this;
        }

        Event &operator-=(const std::function<void(const T)> rhs) {
            RemoveListener(rhs);
            return *this;
        }

        inline void Invoke(T event) {
            for (const auto &listener: m_listeners) {
                listener(event);
            }
        }

    public:
        Event() {}
        ~Event() {}


    private:
        std::vector<std::function<void(const T)>> m_listeners{};
    };

    template <>
    class Event<void> {
    public:
        inline void AddListener(const std::function<void()> listener) {
            m_listeners.push_back(listener);
        }

        inline void RemoveListener(const std::function<void()> listener) {
            m_listeners.erase(std::remove_if(
                m_listeners.begin(), m_listeners.end(),[&listener](const std::function<void()>& l) {
                     return l.target_type() == listener.target_type()
                            && l.target<void()>() == listener.target<void>();
                }), m_listeners.end()
            );
        }

        Event &operator+=(const std::function<void()> rhs) {
            AddListener(rhs);
            return *this;
        }

        Event &operator-=(const std::function<void()> rhs) {
            RemoveListener(rhs);
            return *this;
        }

        inline void Invoke() {
            for (const auto &listener: m_listeners) {
                listener();
            }
        }

    public:
        Event() {}
        ~Event() {}


    private:
        std::vector<std::function<void(void)>> m_listeners{};
    };
}




#endif
