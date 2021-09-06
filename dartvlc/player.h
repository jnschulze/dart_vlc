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
  void OnVideoDimensionsChanged(int32_t width, int32_t height);
  ~Player();

 private:
  std::unique_ptr<VideoOutput> video_output_;
};

class Players {
 public:
  Player* Get(int32_t id, std::vector<std::string> cmd_arguments = {}) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto [it, added] = players_.try_emplace(id, nullptr);
    if (added) {
      it->second = std::make_unique<Player>(cmd_arguments);
    }
    return it->second.get();
  }

  void Dispose(int32_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    players_.erase(id);
  }

 private:
  std::mutex mutex_;
  std::map<int32_t, std::unique_ptr<Player>> players_;
};

extern std::unique_ptr<Players> g_players;

#endif
