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

#ifndef MEDIASOURCE_PLAYLIST_H_
#define MEDIASOURCE_PLAYLIST_H_

#include <map>
#include <string>
#include <vector>

#include "mediasource/media.h"
#include "mediasource/mediasource.h"

enum PlaylistMode { single, loop, repeat };

class Playlist : public MediaSource {
 public:
  PlaylistMode& playlist_mode() { return playlist_mode_; }

  /*
    Playlist(std::vector<std::shared_ptr<Media>> medias,
             PlaylistMode playlist_mode = PlaylistMode::single)
        : medias_(medias), playlist_mode_(playlist_mode){};
        */

  std::string Type() { return "MediaSourceType.playlist"; }

  virtual const std::vector<std::shared_ptr<Media>>& medias() const = 0;
  virtual void Add(std::shared_ptr<Media> media) = 0;
  virtual void Remove(uint32_t index) = 0;
  virtual void Insert(uint32_t index, std::shared_ptr<Media> media) = 0;
  virtual void Move(uint32_t from, uint32_t to) = 0;
  virtual bool TryGetItem(uint32_t index,
                          std::shared_ptr<Media>& media_item) const = 0;
  virtual size_t length() const = 0;

 protected:
  PlaylistMode playlist_mode_;
};

#endif
