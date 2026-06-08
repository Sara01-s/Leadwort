#pragma once

#include "Logger.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

namespace Engine::Utils {

// Opaque handle returned by Subscribe. Pass it to Unsubscribe for exact removal.
struct SubscriptionHandle {
    uint64_t id = 0;
    bool IsValid() const { return id != 0; }
};

template <typename... Args>
class ReactiveCommand {
public:
    ~ReactiveCommand() { Clear(); }

    ReactiveCommand() = default;
    ReactiveCommand(const ReactiveCommand&) = delete;
    ReactiveCommand& operator=(const ReactiveCommand&) = delete;
    ReactiveCommand(ReactiveCommand&&) = default;
    ReactiveCommand& operator=(ReactiveCommand&&) = default;

    // Returns a handle for later unsubscription
    // Optional owner pointer for bulk unsubscription
    SubscriptionHandle Subscribe(std::function<void(Args...)> action, void* owner = nullptr) {
        CORE_ASSERT(action, "Subscribing a null callback");

        const SubscriptionHandle handle { ++m_NextId };
        m_Callbacks.push_back({ std::move(action), owner, handle.id });

        return handle;
    }

    // Exact removal via handle, O(n), always correct
    void Unsubscribe(SubscriptionHandle handle) {
        if (!handle.IsValid()) {
            return;
        }

        MarkForRemoval([&](const Entry& entry) {
	        return entry.id == handle.id;
        });
    }

    // Remove all callbacks registered with this owner pointer
    void UnsubscribeAllFrom(void* owner) {
        MarkForRemoval([owner](const Entry& entry) {
	        return entry.owner == owner;
        });
    }

    void Execute(Args... args) {
        if (m_Callbacks.empty()) {
            return;
        }

        ++m_Depth;
        for (const auto& entry : m_Callbacks) {
            if (!entry.pendingRemoval) {
                entry.action(args...);
            }
        }
        --m_Depth;

        if (m_Depth == 0 && m_PendingRemovals > 0) {
            Sweep();
        }
    }

    void Clear() {
        // Safe even if called during Execute, just marks everything
        if (m_Depth > 0) {
            for (auto& entry : m_Callbacks) {
                entry.pendingRemoval = true;
            }

            m_PendingRemovals = static_cast<int>(m_Callbacks.size());
        }
    	else {
            m_Callbacks.clear();
        }
    }

    int Count() const {
        return static_cast<int>(m_Callbacks.size()) - m_PendingRemovals;
    }

private:
    struct Entry {
        std::function<void(Args...)> action;
        void*    owner          = nullptr;
        uint64_t id             = 0;
        bool     pendingRemoval = false;
    };

    template <typename Predicate>
    void MarkForRemoval(Predicate pred) {
        for (auto& entry : m_Callbacks) {
            if (!entry.pendingRemoval && pred(entry)) {
                entry.pendingRemoval = true;
                ++m_PendingRemovals;
            }
        }
        if (m_Depth == 0) {
            Sweep();
        }
    }

    void Sweep() {
        m_Callbacks.erase(
            std::remove_if(m_Callbacks.begin(), m_Callbacks.end(), [](const Entry& entry) {
	            return entry.pendingRemoval;
            }),

            m_Callbacks.end()
        );

        m_PendingRemovals = 0;
    }

    std::vector<Entry> m_Callbacks;
    uint64_t m_NextId         = 0;
    int      m_Depth          = 0; // re-entrance guard
    int      m_PendingRemovals = 0;
};

// Convenience aliases
using ReactiveVoidCommand = ReactiveCommand<>;

template <typename T>
using ReactiveCommand1 = ReactiveCommand<T>;

template <typename T1, typename T2>
using ReactiveCommand2 = ReactiveCommand<T1, T2>;

} // namespace Engine::Utils