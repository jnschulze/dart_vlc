#ifndef PLAYER_IMPL_H_
#define PLAYER_IMPL_H_

#include <memory>
#include <mutex>
#include <optional>

#include "player.h"
#include "playlist_impl.h"

struct State {
  int32_t duration;
  double position;
  int32_t index;
  PlaybackState playback_state;
  std::optional<int64_t> pending_seek;
  VLC::MediaPtr current_item;
  bool is_seekable;

  State() { Reset(); }

  void Reset() {
    duration = 0;
    position = 0;
    index = 0;
    playback_state = PlaybackState::kNone;
    pending_seek.reset();
    current_item.reset();
    is_seekable = false;
  }
};

class PlayerCallbacks {
 public:
  void OnVideoDimensions(VideoDimensionsChangedCallback callback) {
    video_dimension_callback_ = callback;
  }

  void OnOpen(OpenCallback callback) { open_callback_ = callback; }

  void OnPlaybackStateChanged(PlaybackStateChangedCallback callback) {
    playback_state_changed_ = std::move(callback);
  }

  void OnPosition(std::function<void(double)> callback) {
    position_callback_ = callback;
  }

  void OnSeekable(std::function<void(bool)> callback) {
    seekable_callback_ = callback;
  }

  void OnComplete(std::function<void()> callback) {
    complete_callback_ = callback;
  }

  void OnVolume(std::function<void(float)> callback) {
    volume_callback_ = callback;
  }

  void OnRate(std::function<void(float)> callback) {
    rate_callback_ = callback;
  }

  void OnPlaylist(std::function<void()> callback) {
    playlist_callback_ = callback;
  }

 protected:
  std::function<void()> playlist_callback_ = [=]() -> void {};

  OpenCallback open_callback_;
  VideoDimensionsChangedCallback video_dimension_callback_;

  std::function<void(bool)> seekable_callback_ = [=](bool) -> void {};
  std::function<void(float)> volume_callback_ = [=](float) -> void {};
  std::function<void(float)> rate_callback_ = [=](float) -> void {};
  std::function<void()> complete_callback_ = [=]() -> void {};
  std::function<void(double)> position_callback_ =
      [=](double position) -> void {};
  PlaybackStateChangedCallback playback_state_changed_;
};

class Player::Impl : public PlayerCallbacks {
 public:
  Impl(const std::vector<std::string>& arguments);

  void SetVideoOutput(std::unique_ptr<VideoOutput> output);
  VideoOutput* GetVideoOutput() const;

  int32_t duration() {
    auto duration = static_cast<int32_t>(vlc_media_player_.length());
    return std::max(duration, 0);
  }

  double position() {
    // return static_cast<int32_t>(vlc_media_player_.length() *
    //                            vlc_media_player_.position());

    // return vlc_media_player_.position();

    return state_.position;
  }

  float volume() { return vlc_media_player_.volume() / 100.0f; }

  float rate() { return vlc_media_player_.rate(); }

  Playlist* playlist() const { return playlist_.get(); }
  PlaybackState state() const { return state_.playback_state; }

  std::unique_ptr<Playlist> CreatePlaylist();
  void Open(std::shared_ptr<Playlist> playlist);

  void Play();
  void Pause();
  void PlayOrPause();
  void Stop();
  void Next();
  void Back();
  void Jump(int32_t index);
  void Seek(int64_t position);
  void SetVolume(float volume);
  void SetRate(float rate);

 private:
  std::mutex state_mutex_;

  std::unique_ptr<VideoOutput> video_output_;
  std::shared_ptr<PlaylistImpl> playlist_;

  State state_;

  VLC::Instance vlc_instance_;
  VLC::MediaPlayer vlc_media_player_;
  VLC::MediaListPlayer vlc_media_list_player_;
  // std::unique_ptr<PlayerState> state_ = nullptr;
  int32_t video_width_ = 0;
  int32_t video_height_ = 0;
  std::optional<int32_t> preferred_video_width_ = std::nullopt;
  std::optional<int32_t> preferred_video_height_ = std::nullopt;

  void SetupEventHandlers();
  void HandleMediaChanged(VLC::MediaPtr vlc_media_ptr);
  void HandlePositionChanged(float relative_position);
  void HandleSeekableChanged(bool is_seekable);

  void HandleVlcState(PlaybackState);
  void OnPlaylistUpdated();
};

#endif