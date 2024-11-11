#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_set>


class Signal {
public:
  using Slot = std::function<void()>;

  void connect(const std::weak_ptr<Slot> &s) {
    std::lock_guard<std::mutex> a(mutex_);
    watcher.insert(s);
  }

  void disconnect(const std::weak_ptr<Slot> &w) {
    std::lock_guard<std::mutex> a(mutex_);

    auto it = watcher.find(w);
    if (it != watcher.end()) {
        watcher.erase(it);
    }
  }

  void clear() {
    std::lock_guard<std::mutex> a(mutex_);
    watcher.clear();
  }

  void emit() {
    std::lock_guard<std::mutex> a(mutex_);
    for (auto it = watcher.begin(); it != watcher.end(); ) {
        auto func = it->lock();
        if (func) {
            func->operator()();
        }
        else {
            it = watcher.erase(it);
            continue;
        }
		it++;

    }
  }

private:
  struct Hash {
    size_t operator()(const std::weak_ptr<Slot>& k) const {
      auto sharedPtr = k.lock();
      if (sharedPtr) {
        return std::hash<void *>{}(sharedPtr.get());
      }
      return 0;
    }
  };

  struct Equal {
      bool operator()(const std::weak_ptr<Slot>&a, const std::weak_ptr<Slot>&b)const {
          return a.lock() == b.lock();
      }
  };

  std::mutex mutex_;
  std::unordered_set<std::weak_ptr<Slot>, Hash,Equal> watcher;
};
