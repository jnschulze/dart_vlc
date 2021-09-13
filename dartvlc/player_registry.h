#ifndef PLAYER_REGISTRY_H_
#define PLAYER_REGISTRY_H_

#include <mutex>
#include <unordered_map>
#include <vector>

#include "player.h"

class Players {
 public:
  Player* Create(std::vector<std::string> cmd_arguments = {}) {
    auto player = std::make_unique<Player>(cmd_arguments);
    auto ptr = player.get();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      players_[player->id()] = std::move(player);
    }
    return ptr;
  }

  Player* Get(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = players_.find(id);
    if (it != players_.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  void Invoke(int64_t id, std::function<void(Player* player)> callback) {
    auto player = Get(id);
    if (player != nullptr) {
      callback(player);
    }
  }

  void Dispose(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    players_.erase(id);
  }

 private:
  std::mutex mutex_;
  std::unordered_map<int64_t, std::unique_ptr<Player>> players_;
};

extern std::unique_ptr<Players> g_players;

#endif