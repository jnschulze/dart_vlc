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

#include <iostream>

#include "internal/getters.h"

typedef std::function<void(int32_t, int32_t)> VideoDimensionsChangedCallback;

class PlayerEvents : public PlayerGetters {
 public:
  void OnOpen(std::function<void(VLC::Media)> callback) {
    open_callback_ = callback;
    vlc_media_player_.eventManager().onMediaChanged(
        std::bind(&PlayerEvents::OnOpenCallback, this, std::placeholders::_1));
  }

  void OnPlay(std::function<void()> callback) {
    play_callback_ = callback;
    vlc_media_player_.eventManager().onPlaying(
        std::bind(&PlayerEvents::OnPlayCallback, this));
  }

  void OnVideoDimensions(VideoDimensionsChangedCallback callback) {
    video_dimension_callback_ = callback;
  }

  void OnPause(std::function<void()> callback) {
    pause_callback_ = callback;
    vlc_media_player_.eventManager().onPaused(
        std::bind(&PlayerEvents::OnPauseCallback, this));
  }

  void OnStop(std::function<void()> callback) {
    stop_callback_ = callback;
    vlc_media_player_.eventManager().onStopped(
        std::bind(&PlayerEvents::OnStopCallback, this));
  }

  void OnPosition(std::function<void(int32_t)> callback) {
    position_callback_ = callback;
    vlc_media_player_.eventManager().onPositionChanged(std::bind(
        &PlayerEvents::OnPositionCallback, this, std::placeholders::_1));
  }

  void OnSeekable(std::function<void(bool)> callback) {
    seekable_callback_ = callback;
    vlc_media_player_.eventManager().onSeekableChanged(std::bind(
        &PlayerEvents::OnSeekableCallback, this, std::placeholders::_1));
  }

  void OnComplete(std::function<void()> callback) {
    complete_callback_ = callback;
    vlc_media_player_.eventManager().onEndReached(
        std::bind(&PlayerEvents::OnCompleteCallback, this));
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

  void OnPlaylistCallback() {
    if (is_playlist_modified_) {
      vlc_media_list_player_.setMediaList(vlc_media_list_);
      if (!vlc_media_list_.count()) {
        state()->Reset();
        vlc_media_list_player_.stop();
        return;
      }
      if (state()->index_ > vlc_media_list_.count())
        state()->index_ = vlc_media_list_.count() - 1;
      is_playlist_modified_ = false;
      playlist_callback_();
    };
  }

  std::function<void(VLC::Media)> open_callback_ = [=](VLC::Media) -> void {};

  void OnOpenCallback(VLC::MediaPtr vlc_media_ptr) {
    state()->is_playing_ = vlc_media_player_.isPlaying();
    state()->is_valid_ = vlc_media_player_.isValid();
    if (duration() > 0) {
      state()->is_completed_ = false;
      state()->position_ = position();
      state()->duration_ = duration();
    } else {
      state()->is_completed_ = false;
      state()->position_ = 0;
      state()->duration_ = 0;
    }
    state()->index_ = vlc_media_list_.indexOfItem(*vlc_media_ptr.get());
    open_callback_(*vlc_media_ptr.get());
  }

  VideoDimensionsChangedCallback video_dimension_callback_;

  std::function<void()> play_callback_ = [=]() -> void {};

  void OnPlayCallback() {
    state()->is_playing_ = vlc_media_player_.isPlaying();
    if (duration() > 0) {
      state()->is_valid_ = vlc_media_player_.isValid();
      state()->is_completed_ = false;
      state()->position_ = position();
      state()->duration_ = duration();
    }
    play_callback_();
  }

  std::function<void()> pause_callback_ = [=]() -> void {};

  void OnPauseCallback() {
    state()->is_playing_ = vlc_media_player_.isPlaying();
    if (duration() > 0) {
      state()->position_ = position();
      state()->is_valid_ = vlc_media_player_.isValid();
      state()->duration_ = duration();
    }
    pause_callback_();
  }

  std::function<void()> stop_callback_ = [=]() -> void {};

  void OnStopCallback() {
    state()->is_playing_ = vlc_media_player_.isPlaying();
    state()->is_valid_ = vlc_media_player_.isValid();
    state()->position_ = 0;
    state()->duration_ = 0;
    stop_callback_();
  }

  std::function<void(int32_t)> position_callback_ =
      [=](int32_t position) -> void {};

  void OnPositionCallback(float relative_position) {
    state()->is_playing_ = vlc_media_player_.isPlaying();
    if (duration() > 0) {
      state()->position_ = position();
      state()->is_valid_ = vlc_media_player_.isValid();
      state()->duration_ = duration();
    }
    position_callback_(
        static_cast<int32_t>(relative_position * vlc_media_player_.length()));
  }

  std::function<void(bool)> seekable_callback_ = [=](bool) -> void {};

  void OnSeekableCallback(bool isSeekable) {
    if (duration() > 0) {
      state()->is_seekable_ = isSeekable;
      seekable_callback_(isSeekable);
    }
  }

  std::function<void()> complete_callback_ = [=]() -> void {};

  void OnCompleteCallback() {
    state()->is_playing_ = vlc_media_player_.isPlaying();
    if (duration() > 0) {
      state()->is_valid_ = vlc_media_player_.isValid();
      state()->is_completed_ = true;
      state()->position_ = position();
      state()->duration_ = duration();
      OnPlaylistCallback();
      complete_callback_();
    } else {
      state()->position_ = 0;
      state()->duration_ = 0;
    }
  }

  std::function<void(float)> volume_callback_ = [=](float) -> void {};

  std::function<void(float)> rate_callback_ = [=](float) -> void {};
};
