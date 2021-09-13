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

#include <functional>

#include "device.h"
#include "equalizer.h"
#include "mediasource/media.h"
#include "mediasource/mediasource.h"
#include "mediasource/playlist.h"

enum class PlaybackState {
  kNone,
  kOpening,
  kBuffering,
  kPlaying,
  kPaused,
  kStopped,
  kEnded,
  kError
};

typedef std::function<void()> VoidCallback;
typedef std::function<void(PlaybackState)> PlaybackStateChangedCallback;
typedef std::function<void(int32_t, int32_t)> VideoDimensionsChangedCallback;
typedef std::function<void(std::shared_ptr<Media> media, int32_t index)>
    OpenCallback;

class PlayerEvents {
 public:
  virtual void OnOpen(OpenCallback) = 0;

  virtual void OnVideoDimensions(VideoDimensionsChangedCallback callback) = 0;
  virtual void OnPlaybackStateChanged(
      PlaybackStateChangedCallback callback) = 0;

  virtual void OnPosition(std::function<void(double)> callback) = 0;
  virtual void OnSeekable(std::function<void(bool)> callback) = 0;
  virtual void OnComplete(VoidCallback callback) = 0;
  virtual void OnVolume(std::function<void(float)> callback) = 0;
  virtual void OnRate(std::function<void(float)> callback) = 0;
  virtual void OnPlaylist(VoidCallback callback) = 0;
};

class PlayerGetters {
 public:
  // virtual int32_t video_width() = 0;
  // virtual int32_t video_height() = 0;
  // PlayerState* state() const { return state_.get(); }
  virtual int32_t duration() = 0;
  virtual double position() = 0;
  virtual float volume() = 0;
  virtual float rate() = 0;
  virtual PlaybackState playback_state() = 0;
};

class PlayerSetters {
 public:
  virtual void Open(std::shared_ptr<Playlist> media_source,
                    bool auto_start = true) = 0;
  virtual void Play() = 0;
  virtual void Pause() = 0;
  virtual void PlayOrPause() = 0;
  virtual void Stop() = 0;
  virtual void Next() = 0;
  virtual void Back() = 0;
  virtual void Jump(int32_t index) = 0;
  virtual void Seek(int32_t position) = 0;
  virtual void SetVolume(float volume) = 0;
  virtual void SetRate(float rate) = 0;
  virtual void SetAudioDevice(const Device& device) = 0;
  virtual void SetPlaylistMode(PlaylistMode mode) = 0;
  virtual void SetEqualizer(Equalizer equalizer) = 0;
  virtual void SetUserAgent(std::string userAgent) = 0;
  /*
  virtual void Add(std::shared_ptr<Media> media) = 0;
  virtual void Remove(int32_t index) = 0;
  virtual void Insert(int32_t index, std::shared_ptr<Media> media) = 0;
  virtual void Move(int32_t initial, int32_t final) = 0;
  */
  virtual void TakeSnapshot(std::string file_path, int32_t width,
                            int32_t height) = 0;
  virtual void SetVideoWidth(int32_t video_width) = 0;
  virtual void SetVideoHeight(int32_t video_height) = 0;
};
