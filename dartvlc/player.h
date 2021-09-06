/*
 * dart_vlc: A media playback library for Dart & Flutter. Based on libVLC &
 * libVLC++.
 *
 * Hitesh Kumar Saini
 * https://github.com/alexmercerind
 * saini123hitesh@gmail.com; alexmercerind@gmail.com
 *
 * GNU Lesser General Public License v2.1
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <memory>
#include <mutex>

#include "internal/setters.h"
#include "internal/video_output.h"

class Player : public PlayerSetters {
 public:
  Player(const std::vector<std::string>& cmd_arguments);

  void SetVideoOutput(std::unique_ptr<VideoOutput> output);
  VideoOutput* GetVideoOutput() const { return video_output_.get(); }
  bool HasVideoOutput() {
    if (video_output_) {
      return true;
    }
    return false;
  }

  int64_t id() const { return reinterpret_cast<int64_t>(this); }

  void OnVideoDimensionsChanged(int32_t width, int32_t height);
  ~Player();

 private:
  std::unique_ptr<VideoOutput> video_output_;
};

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

  void Dispose(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    players_.erase(id);
  }

 private:
  std::mutex mutex_;
  std::map<int64_t, std::unique_ptr<Player>> players_;
};

extern std::unique_ptr<Players> g_players;

#endif
