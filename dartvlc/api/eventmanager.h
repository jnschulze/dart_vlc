/*
 * dart_vlc: A media playback library for Dart & Flutter. Based on libVLC &
 * libVLC++.
 *
 * Hitesh Kumar Saini & contributors.
 * https://github.com/alexmercerind
 * alexmercerind@gmail.com
 *
 * GNU Lesser General Public License v2.1
 */

#ifndef API_EVENTMANAGER_H_
#define API_EVENTMANAGER_H_

#include <iostream>

#include "api/dartmanager.h"
#include "base.h"
#include "dart_api_dl.h"
#include "player.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*Dart_PostCObjectType)(Dart_Port port_id, Dart_CObject* message);

Dart_PostCObjectType g_dart_post_C_object;
Dart_Port g_callback_port;

DLLEXPORT void InitializeDartApi(Dart_PostCObjectType dart_post_C_object,
                                 Dart_Port callback_port, void* data) {
  g_dart_post_C_object = dart_post_C_object;
  g_callback_port = callback_port;
  Dart_InitializeApiDL(data);
}

// Needs to be kept in sync with the Dart-side enum.
enum EventType {
  kEventUnknown,
  kEventPlaylistUpdated,
  kEventPlaybackStateChanged,
  kEventPositionChanged,
  kEventRateChanged,
  kEventVolumeChanged,
  kEventVideoDimensionsChanged,
  kEventVideoFrameAvailable
};

struct EventEnvelope {
  Dart_CObject id_object;
  Dart_CObject type_object;
  Dart_CObject payload_object{};
  std::array<Dart_CObject*, 3> elements;
  Dart_CObject envelope;
  explicit EventEnvelope(int64_t id, EventType event_type)
      : elements({&id_object, &type_object, &payload_object}) {
    id_object.type = Dart_CObject_kInt64;
    id_object.value.as_int64 = id;

    type_object.type = Dart_CObject_kInt32;
    type_object.value.as_int32 = static_cast<int32_t>(event_type);

    envelope.type = Dart_CObject_kArray;
    envelope.value.as_array.values = elements.data();
    envelope.value.as_array.length = elements.size();
  }

  Dart_CObject* get() { return &envelope; }
};

struct PlaylistUpdated : public EventEnvelope {
  Dart_CObject index_object;
  Dart_CObject is_playlist_object;
  Dart_CObject types_object;
  Dart_CObject resources_object;

  std::unique_ptr<Dart_CObject[]> property_objects;
  std::unique_ptr<Dart_CObject*[]> property_object_refs;

  explicit PlaylistUpdated(int64_t player_id, const Playlist* playlist,
                           bool is_playlist, int32_t current_index)
      : EventEnvelope(player_id, EventType::kEventPlaylistUpdated) {
    index_object.type = Dart_CObject_kInt32;
    index_object.value.as_int32 = current_index;

    is_playlist_object.type = Dart_CObject_kBool;
    is_playlist_object.value.as_bool = is_playlist;

    const auto& media_items = playlist->medias();
    auto media_item_count = media_items.size();
    property_objects.reset(new Dart_CObject[media_item_count * 2]);
    property_object_refs.reset(new Dart_CObject*[media_item_count * 2]);

    for (size_t i = 0; i < media_items.size(); i++) {
      const auto& item = media_items[i];

      Dart_CObject* type_object = &property_objects[i];
      property_object_refs[i] = type_object;
      type_object->type = Dart_CObject_kString;
      type_object->value.as_string =
          const_cast<char*>(item->media_type().c_str());

      Dart_CObject* url_object = &property_objects[media_item_count + i];
      property_object_refs[media_item_count + i] = url_object;
      url_object->type = Dart_CObject_kString;
      url_object->value.as_string = const_cast<char*>(item->resource().c_str());
    }

    types_object.type = Dart_CObject_kArray;
    types_object.value.as_array.length = media_item_count;
    types_object.value.as_array.values = property_object_refs.get();

    resources_object.type = Dart_CObject_kArray;
    resources_object.value.as_array.length = media_item_count;
    resources_object.value.as_array.values =
        property_object_refs.get() + media_item_count;

    Dart_CObject* value_objects[] = {&index_object, &is_playlist_object,
                                     &types_object, &resources_object};
    payload_object.type = Dart_CObject_kArray;
    payload_object.value.as_array.values = value_objects;
    payload_object.value.as_array.length =
        sizeof(value_objects) / sizeof(value_objects[0]);
  }
};

struct PlaybackStateChanged : public EventEnvelope {
  Dart_CObject state_object;
  Dart_CObject is_seekable_object;

  explicit PlaybackStateChanged(int64_t player_id, PlaybackState state,
                                bool is_seekable)
      : EventEnvelope(player_id, EventType::kEventPlaybackStateChanged) {
    state_object.type = Dart_CObject_kInt32;
    state_object.value.as_int32 = static_cast<int32_t>(state);

    is_seekable_object.type = Dart_CObject_kBool;
    is_seekable_object.value.as_bool = is_seekable;

    Dart_CObject* value_objects[] = {&state_object, &is_seekable_object};
    payload_object.type = Dart_CObject_kArray;
    payload_object.value.as_array.values = value_objects;
    payload_object.value.as_array.length =
        sizeof(value_objects) / sizeof(value_objects[0]);
  }
};

struct PositionChanged : public EventEnvelope {
  Dart_CObject position_object;
  Dart_CObject duration_object;

  explicit PositionChanged(int64_t player_id, double position, int32_t duration)
      : EventEnvelope(player_id, EventType::kEventPositionChanged) {
    position_object.type = Dart_CObject_kDouble;
    position_object.value.as_double = position;

    duration_object.type = Dart_CObject_kInt32;
    duration_object.value.as_int32 = duration;

    Dart_CObject* value_objects[] = {&position_object, &duration_object};
    payload_object.type = Dart_CObject_kArray;
    payload_object.value.as_array.values = value_objects;
    payload_object.value.as_array.length =
        sizeof(value_objects) / sizeof(value_objects[0]);
  }
};

struct RateChanged : public EventEnvelope {
  explicit RateChanged(int64_t player_id, double rate)
      : EventEnvelope(player_id, EventType::kEventRateChanged) {
    payload_object.type = Dart_CObject_kDouble;
    payload_object.value.as_double = rate;
  }
};

struct VolumeChanged : public EventEnvelope {
  explicit VolumeChanged(int64_t player_id, double volume)
      : EventEnvelope(player_id, EventType::kEventVolumeChanged) {
    payload_object.type = Dart_CObject_kDouble;
    payload_object.value.as_double = volume;
  }
};

struct VideoDimensionsChanged : public EventEnvelope {
  Dart_CObject width_object;
  Dart_CObject height_object;

  explicit VideoDimensionsChanged(int64_t player_id, uint32_t width,
                                  uint32_t height)
      : EventEnvelope(player_id, EventType::kEventVideoDimensionsChanged) {
    width_object.type = Dart_CObject_kInt32;
    width_object.value.as_int32 = static_cast<int32_t>(width);
    height_object.type = Dart_CObject_kInt32;
    height_object.value.as_int32 = static_cast<int32_t>(height);

    Dart_CObject* value_objects[] = {&width_object, &height_object};
    payload_object.type = Dart_CObject_kArray;
    payload_object.value.as_array.values = value_objects;
    payload_object.value.as_array.length =
        sizeof(value_objects) / sizeof(value_objects[0]);
  }
};

struct VideoFrameAvailable : public EventEnvelope {
  Dart_CObject buffer_object;
  Dart_CObject width_object;
  Dart_CObject height_object;
  Dart_CObject pixel_format_object;
  Dart_CObject bytes_per_row_object;

  explicit VideoFrameAvailable(int64_t player_id, const uint8_t* frame,
                               size_t size, const VideoDimensions& dimensions,
                               PixelFormat pixel_format)
      : EventEnvelope(player_id, EventType::kEventVideoFrameAvailable) {
    buffer_object.type = Dart_CObject_kTypedData;
    buffer_object.value.as_typed_data.type = Dart_TypedData_kUint8;
    buffer_object.value.as_typed_data.values = const_cast<uint8_t*>(frame);
    buffer_object.value.as_typed_data.length = size;

    width_object.type = Dart_CObject_kInt32;
    width_object.value.as_int32 = static_cast<int32_t>(dimensions.width);

    height_object.type = Dart_CObject_kInt32;
    height_object.value.as_int32 = static_cast<int32_t>(dimensions.height);

    pixel_format_object.type = Dart_CObject_kInt32;
    pixel_format_object.value.as_int32 = static_cast<int32_t>(pixel_format);

    bytes_per_row_object.type = Dart_CObject_kInt32;
    bytes_per_row_object.value.as_int32 =
        static_cast<int32_t>(dimensions.bytes_per_row);

    Dart_CObject* value_objects[] = {&buffer_object, &width_object,
                                     &height_object, &pixel_format_object,
                                     &bytes_per_row_object};
    payload_object.type = Dart_CObject_kArray;
    payload_object.value.as_array.values = value_objects;
    payload_object.value.as_array.length =
        sizeof(value_objects) / sizeof(value_objects[0]);
  }
};

inline void OnPlaylistUpdated(int64_t id, const Playlist* playlist,
                              bool is_playlist, int32_t current_index) {
  PlaylistUpdated event(id, playlist, is_playlist, current_index);
  g_dart_post_C_object(g_callback_port, event.get());
}

inline void OnPlaybackStateChanged(int64_t id, PlaybackState state,
                                   bool is_seekable) {
  PlaybackStateChanged event(id, state, is_seekable);
  g_dart_post_C_object(g_callback_port, event.get());
}

inline void OnPositionChanged(int64_t id, double position, int32_t duration) {
  PositionChanged event(id, position, duration);
  g_dart_post_C_object(g_callback_port, event.get());
}

inline void OnRateChanged(int64_t id, double rate) {
  RateChanged event(id, rate);
  g_dart_post_C_object(g_callback_port, event.get());
}

inline void OnVolumeChanged(int64_t id, double volume) {
  VolumeChanged event(id, volume);
  g_dart_post_C_object(g_callback_port, event.get());
}

inline void OnVideoDimensions(int64_t id, int32_t video_width,
                              int32_t video_height) {
  VideoDimensionsChanged event(id, video_width, video_height);
  g_dart_post_C_object(g_callback_port, event.get());
}

inline void OnVideo(int64_t id, const uint8_t* frame, size_t size,
                    const VideoDimensions& dimensions,
                    PixelFormat pixel_format) {
  VideoFrameAvailable event(id, frame, size, dimensions, pixel_format);
  g_dart_post_C_object(g_callback_port, event.get());
}

#ifdef __cplusplus
}
#endif

#endif
