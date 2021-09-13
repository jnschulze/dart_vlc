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

#include "internal/events.h"
#include "internal/video_output.h"

class Player : public PlayerSetters, public PlayerGetters, public PlayerEvents {
 public:
  Player(const std::vector<std::string>& cmd_arguments);
  ~Player();

  void SetVideoOutput(std::unique_ptr<VideoOutput> output);
  VideoOutput* GetVideoOutput() const;
  bool HasVideoOutput() const { return GetVideoOutput() != nullptr; }

  int64_t id() const { return reinterpret_cast<int64_t>(this); }

  /// Events handlers

  void OnVideoDimensionsChanged(int32_t width, int32_t height);
  void OnOpen(OpenCallback callback) override;
  void OnVideoDimensions(VideoDimensionsChangedCallback callback) override;
  // void OnPlaybackStateChanged(PlaybackStateChangedCallback callback)
  // override;

  void OnPosition(std::function<void(double)> callback) override;
  void OnSeekable(std::function<void(bool)> callback) override;
  void OnComplete(std::function<void()> callback) override;
  void OnVolume(std::function<void(float)> callback) override;
  void OnRate(std::function<void(float)> callback) override;
  void OnPlaylist(std::function<void()> callback) override;
  void OnPlaybackStateChanged(PlaybackStateChangedCallback callback) override;

  // Getters
  // int32_t video_width() override;
  // int32_t video_height() override;
  // PlayerState* state() override;
  int32_t duration() override;
  double position() override;
  float volume() override;
  float rate() override;
  PlaybackState playback_state() override;

  std::unique_ptr<Playlist> CreatePlaylist();
  Playlist* playlist() const;

  /// Setters
  void Open(std::shared_ptr<Playlist> media_source,
            bool auto_start = true) override;
  void Play() override;
  void Pause() override;
  void PlayOrPause() override;
  void Stop() override;
  void Next() override;
  void Back() override;
  void Jump(int32_t index) override;
  void Seek(int32_t position) override;
  void SetVolume(float volume) override;
  void SetRate(float rate) override;
  void SetAudioDevice(const Device& device) override;
  void SetPlaylistMode(PlaylistMode mode) override;
  void SetEqualizer(Equalizer equalizer) override;
  void SetUserAgent(std::string userAgent) override;
  /*
  void Add(std::shared_ptr<Media> media) override;
  void Remove(int32_t index) override;
  void Insert(int32_t index, std::shared_ptr<Media> media) override;
  void Move(int32_t initial, int32_t final) override;
  */
  void TakeSnapshot(std::string file_path, int32_t width,
                    int32_t height) override;
  void SetVideoWidth(int32_t video_width) override;
  void SetVideoHeight(int32_t video_height) override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
