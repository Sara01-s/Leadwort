#pragma once

#include "engine/utils/public/Logger.h"

#include <efsw/efsw.hpp>
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace Engine::Systems {

namespace fs = std::filesystem;

enum class FileChangeType { Added, Removed, Modified, Renamed };

struct FileChangeEvent {
    fs::path       path;
    FileChangeType type;
};

using FileChangeCallback = std::function<void(const FileChangeEvent&)>;

class FileWatcherSystem {
public:
    static FileWatcherSystem& Get() {
        static FileWatcherSystem instance;
        return instance;
    }

    FileWatcherSystem(const FileWatcherSystem&) = delete;
    FileWatcherSystem& operator=(const FileWatcherSystem&) = delete;

    int Watch(const fs::path& directory, FileChangeCallback callback) {
        const fs::path normalized = fs::path(directory).lexically_normal();

        if (!fs::exists(normalized)) {
            CORE_ERROR("FileWatcherSystem: directory does not exist: ", normalized.string());
            return -1;
        }

        std::lock_guard lock(m_Mutex);

        efsw::WatchID watchId{};
        const auto dirIt = m_DirToWatchId.find(normalized.string());

        if (dirIt != m_DirToWatchId.end()) {
            watchId = dirIt->second;
            CORE_LOG("FileWatcherSystem [Watch]: reusing existing watchId=", watchId,
                     " for dir='", normalized.string(), "'");
        }
        else {
            watchId = m_FileWatcher.addWatch(normalized.string(), &m_Listener, true);
            if (watchId < 0) {
                CORE_ERROR("FileWatcherSystem [Watch]: efsw failed to watch: ", normalized.string());
                return -1;
            }
            m_DirToWatchId[normalized.string()] = watchId;
            CORE_LOG("FileWatcherSystem [Watch]: new watchId=", watchId,
                     " for dir='", normalized.string(), "'");
        }

        const int subId = m_NextSubscriptionId++;
        m_Callbacks[subId] = std::move(callback);
        m_WatchIdToSubs[watchId].push_back(subId);
        m_SubToWatchId[subId] = watchId;

        CORE_LOG("FileWatcherSystem [Watch]: registered subId=", subId,
                 " -> watchId=", watchId,
                 " (total subs for this watchId: ", m_WatchIdToSubs[watchId].size(), ")");

        return subId;
    }

    void Unwatch(const int subscriptionId) {
        std::lock_guard lock(m_Mutex);

        const auto subIt = m_SubToWatchId.find(subscriptionId);
        if (subIt == m_SubToWatchId.end()) {
            CORE_WARN("FileWatcherSystem [Unwatch]: subId=", subscriptionId, " not found, ignoring.");
            return;
        }

        const efsw::WatchID watchId = subIt->second;
        CORE_LOG("FileWatcherSystem [Unwatch]: removing subId=", subscriptionId,
                 " from watchId=", watchId);

        m_Callbacks.erase(subscriptionId);
        m_SubToWatchId.erase(subIt);

        auto& subs = m_WatchIdToSubs[watchId];
        std::erase(subs, subscriptionId);

        if (subs.empty()) {
            CORE_LOG("FileWatcherSystem [Unwatch]: no more subs for watchId=", watchId, ", removing efsw watch.");
            m_FileWatcher.removeWatch(watchId);
            m_WatchIdToSubs.erase(watchId);

            for (auto it = m_DirToWatchId.begin(); it != m_DirToWatchId.end(); ++it) {
                if (it->second == watchId) {
                    CORE_LOG("FileWatcherSystem [Unwatch]: removed dir entry '", it->first, "'");
                    m_DirToWatchId.erase(it);
                    break;
                }
            }
        }
    }

    void Init() {
        if (m_Started) {
            CORE_WARN("FileWatcherSystem [Init]: already started, skipping.");
            return;
        }

        m_Started = true;
        m_FileWatcher.watch();
        CORE_LOG("FileWatcherSystem [Init]: efsw watcher started.");
    }

    void ProcessPendingEvents() {
        std::queue<InternalEvent> eventsCopy;
        {
            std::lock_guard lock(m_Mutex);
            std::swap(eventsCopy, m_PendingEvents);
        }

        if (eventsCopy.empty()) {
            return;
        }

        CORE_LOG("FileWatcherSystem [Process]: draining ", eventsCopy.size(), " event(s).");

        while (!eventsCopy.empty()) {
            const auto& [watchId, path, type] = eventsCopy.front();

            CORE_LOG("FileWatcherSystem [Process]: event path='", path.string(),
                     "' type=", static_cast<int>(type),
                     " watchId=", watchId);

            auto watchIt = m_WatchIdToSubs.find(watchId);
            if (watchIt != m_WatchIdToSubs.end()) {
                CORE_LOG("FileWatcherSystem [Process]: dispatching to ",
                         watchIt->second.size(), " subscriber(s).");

                FileChangeEvent change { path, type };
                for (int subId : watchIt->second) {
                    auto cbIt = m_Callbacks.find(subId);
                    if (cbIt != m_Callbacks.end()) {
                        cbIt->second(change);
                    }
                    else {
                        CORE_WARN("FileWatcherSystem [Process]: subId=", subId,
                                  " has no callback, skipping.");
                    }
                }
            }
            else {
                CORE_WARN("FileWatcherSystem [Process]: no subscribers for watchId=", watchId,
                          " path='", path.string(), "'. Dropping event.");
            }

            eventsCopy.pop();
        }
    }

private:
    struct InternalEvent {
        efsw::WatchID  watchId;
        fs::path       path;
        FileChangeType type;
    };

    class Listener : public efsw::FileWatchListener {
    public:
        explicit Listener(FileWatcherSystem* owner) : m_Owner(owner) {}

        void handleFileAction(
            const efsw::WatchID watchId,
            const std::string& dir,
            const std::string& filename,
            const efsw::Action action,
            const std::string& oldFilename
        ) override {
            CORE_LOG("FileWatcherSystem [efsw raw]: watchId=", watchId,
                     " dir='", dir,
                     "' file='", filename,
                     "' action=", static_cast<int>(action),
                     " oldFile='", oldFilename, "'");

            FileChangeType type;
            switch (action) {
                case efsw::Actions::Add:      type = FileChangeType::Added;    break;
                case efsw::Actions::Delete:   type = FileChangeType::Removed;  break;
                case efsw::Actions::Modified: type = FileChangeType::Modified; break;
                case efsw::Actions::Moved:    type = FileChangeType::Renamed;  break;
                default:
                    CORE_WARN("FileWatcherSystem [efsw raw]: unknown action=",
                              static_cast<int>(action), ", skipping.");
                    return;
            }

            const fs::path fullPath = (fs::path(dir) / filename).lexically_normal();

            std::lock_guard lock(m_Owner->m_Mutex);
            m_Owner->m_PendingEvents.push({ watchId, fullPath, type });

            CORE_LOG("FileWatcherSystem [efsw raw]: enqueued '", fullPath.string(),
                     "' (queue size now: ", m_Owner->m_PendingEvents.size(), ")");
        }

    private:
        FileWatcherSystem* m_Owner;
    };

    FileWatcherSystem() : m_Listener(this) {}

    efsw::FileWatcher m_FileWatcher;
    Listener          m_Listener;
    bool              m_Started = false;

    std::mutex                                           m_Mutex;
    std::queue<InternalEvent>                            m_PendingEvents;
    int                                                  m_NextSubscriptionId = 0;

    std::unordered_map<std::string, efsw::WatchID>      m_DirToWatchId;
    std::unordered_map<efsw::WatchID, std::vector<int>> m_WatchIdToSubs;
    std::unordered_map<int, efsw::WatchID>              m_SubToWatchId;
    std::unordered_map<int, FileChangeCallback>         m_Callbacks;
};

} // namespace Engine::Systems