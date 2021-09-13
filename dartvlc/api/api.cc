/*
 * dart_vlc: A media playback library for Dart & Flutter. Based on libVLC &
 * libVLC++.
 *
 * Hitesh Kumar Saini
 * https://github.com/alexmercerind
 * alexmercerind@gmail.com
 *
 * GNU Lesser General Public License v2.1
 */

#include "api.h"

#include <memory>

#include "broadcast.h"
#include "chromecast.h"
#include "device.h"
#include "equalizer.h"
#include "player.h"
#include "player_registry.h"
#include "record.h"
#include "video_frame_adapter.h"

namespace DartObjects {

struct DeviceList {
  // The device list that gets exposed to Dart.
  DartDeviceList dart_object;

  // Backing data
  std::vector<DartDeviceList::Device> device_infos;
  std::vector<Device> devices;
};

struct Equalizer {
  // The equalizer that gets exposed to Dart.
  DartEqualizer dart_object;

  // Backing data
  std::vector<float> bands;
  std::vector<float> amps;
};

static void DestroyObject(void*, void* peer) { delete peer; }

}  // namespace DartObjects

#ifdef __cplusplus
extern "C" {
#endif

int64_t PlayerCreate(int32_t video_width, int32_t video_height,
                     int32_t commandLineArgumentsCount,
                     const char** commandLineArguments) {
  std::vector<std::string> args{};
  for (int32_t index = 0; index < commandLineArgumentsCount; index++)
    args.emplace_back(commandLineArguments[index]);
  Player* player = g_players->Create(args);
  if (!player) {
    return 0;
  }
  auto id = player->id();
  if (video_width > 0 && video_height > 0) {
    player->SetVideoWidth(video_width);
    player->SetVideoHeight(video_height);
  }

  player->OnOpen([=](std::shared_ptr<Media> media, int32_t index) {
    std::cerr << "CURRENT MEDIA CHANGED" << std::endl;

    std::cerr << "MEDIA LOCATION " << media->location() << std::endl;

    OnPlaylistUpdated(id, player->playlist(), true, index);
  });

  player->OnPlaybackStateChanged([=](PlaybackState state) {
    OnPlaybackStateChanged(id, state, true);
    if (state == PlaybackState::kStopped) {
      auto position = player->position();
      OnPositionChanged(id, position, player->duration());
    }
  });

  player->OnPosition([=](double position) -> void {
    OnPositionChanged(id, position, player->duration());
  });
  player->OnRate([=](double rate) -> void { OnRateChanged(id, rate); });
  player->OnVolume([=](float volume) -> void { OnVolumeChanged(id, volume); });

  // ??
  // player->OnPlaylist([=]() -> void { OnOpen(id, player->state()); });

  player->OnVideoDimensions(
      [=](int32_t video_width, int32_t video_height) -> void {
        OnVideoDimensions(id, video_width, video_height);
      });

#ifdef _WIN32
/* Windows: Texture & flutter::TextureRegistrar */
#else
  /* Linux: decodeImageFromPixels & NativePorts */

  auto adapter = std::make_unique<VideoFrameAdapter>();
  adapter->OnFrameArrived(
      [=](const uint8_t* buffer, const VideoDimensions& dimensions) {
        OnVideo(id, buffer, dimensions.bytes_per_row * dimensions.height,
                dimensions, PixelFormat::kFormatRGBA);
      });

  player->SetVideoOutput(std::make_unique<PixelBufferOutput>(
      std::move(adapter), PixelFormat::kFormatRGBA));
#endif
  return id;
}

void PlayerDispose(int64_t id) { g_players->Dispose(id); }

void PlayerOpen(int64_t id, bool auto_start, const char** source,
                int32_t source_size) {
  g_players->Invoke(id, [=](Player* player) {
    auto playlist = player->CreatePlaylist();
    for (int32_t index = 0; index < 2 * source_size; index += 2) {
      std::shared_ptr<Media> media;
      const char* type = source[index];
      const char* resource = source[index + 1];
      playlist->Add(Media::create(type, resource));
    }

    player->Open(std::move(playlist), auto_start);
  });
}

void PlayerPlay(int64_t id) {
  g_players->Invoke(id, [](Player* player) { player->Play(); });
}

void PlayerPause(int64_t id) {
  g_players->Invoke(id, [](Player* player) { player->Pause(); });
}

void PlayerPlayOrPause(int64_t id) {
  g_players->Invoke(id, [](Player* player) { player->PlayOrPause(); });
}

void PlayerStop(int64_t id) {
  g_players->Invoke(id, [](Player* player) { player->Stop(); });
}

void PlayerNext(int64_t id) {
  g_players->Invoke(id, [](Player* player) { player->Next(); });
}

void PlayerBack(int64_t id) {
  g_players->Invoke(id, [](Player* player) { player->Back(); });
}

void PlayerJump(int64_t id, int32_t index) {
  g_players->Invoke(id, [=](Player* player) { player->Jump(index); });
}

void PlayerSeek(int64_t id, int32_t position) {
  g_players->Invoke(id, [=](Player* player) { player->Seek(position); });
}

void PlayerSetVolume(int64_t id, float volume) {
  g_players->Invoke(id, [=](Player* player) { player->SetVolume(volume); });
}

void PlayerSetRate(int64_t id, float rate) {
  g_players->Invoke(id, [=](Player* player) { player->SetRate(rate); });
}

void PlayerSetUserAgent(int64_t id, const char* user_agent) {
  g_players->Invoke(id,
                    [=](Player* player) { player->SetUserAgent(user_agent); });
}

void PlayerSetDevice(int64_t id, const char* device_id,
                     const char* device_name) {
  g_players->Invoke(id, [=](Player* player) {
    Device device(device_id, device_name);
    player->SetAudioDevice(device);
  });
}

void PlayerSetEqualizer(int64_t id, int32_t equalizer_id) {
  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }
  Equalizer* equalizer = g_equalizers->Get(equalizer_id);
  player->SetEqualizer(*equalizer);
}

void PlayerSetPlaylistMode(int64_t id, const char* mode) {
  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }
  PlaylistMode playlistMode;
  if (strcmp(mode, "PlaylistMode.repeat") == 0)
    playlistMode = PlaylistMode::repeat;
  else if (strcmp(mode, "PlaylistMode.loop") == 0)
    playlistMode = PlaylistMode::loop;
  else
    playlistMode = PlaylistMode::single;
  player->SetPlaylistMode(playlistMode);
}

void PlayerAdd(int64_t id, const char* type, const char* resource) {
  std::cerr << "PL ADD" << std::endl;

  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }

  auto playlist = player->playlist();
  if (!playlist) {
    return;
  }

  auto media = Media::create(type, resource);
  playlist->Add(std::move(media));
}

void PlayerRemove(int64_t id, int32_t index) {
  std::cerr << "PL REMOVE" << std::endl;

  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }
  auto playlist = player->playlist();
  if (!playlist) {
    return;
  }
  playlist->Remove(index);
}

void PlayerInsert(int64_t id, int32_t index, const char* type,
                  const char* resource) {
  std::cerr << "PL INSERT" << std::endl;

  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }
  auto playlist = player->playlist();
  if (!playlist) {
    return;
  }
  auto media = Media::create(type, resource);
  playlist->Insert(index, std::move(media));
}

void PlayerMove(int64_t id, int32_t initial_index, int32_t final_index) {
  std::cerr << "PL MOVE" << std::endl;

  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }
  auto playlist = player->playlist();
  if (!playlist) {
    return;
  }
  playlist->Move(initial_index, final_index);
}

void PlayerTakeSnapshot(int64_t id, const char* file_path, int32_t width,
                        int32_t height) {
  Player* player = g_players->Get(id);
  if (!player) {
    return;
  }
  player->TakeSnapshot(file_path, width, height);
}

void MediaClearMap(void*, void* peer) {
  delete reinterpret_cast<std::map<std::string, std::string>*>(peer);
}

void MediaClearVector(void*, void* peer) {
  delete reinterpret_cast<std::vector<const char*>*>(peer);
}

const char** MediaParse(Dart_Handle object, const char* type,
                        const char* resource, int32_t timeout) {
  std::shared_ptr<Media> media = Media::create(type, resource, true, timeout);
  auto metas = new std::map<std::string, std::string>(media->metas());
  auto values = new std::vector<const char*>();
  Dart_NewFinalizableHandle_DL(
      object, reinterpret_cast<void*>(metas), sizeof(metas),
      static_cast<Dart_HandleFinalizer>(MediaClearMap));
  Dart_NewFinalizableHandle_DL(
      object, reinterpret_cast<void*>(values), sizeof(values),
      static_cast<Dart_HandleFinalizer>(MediaClearVector));
  for (const auto& [key, value] : *metas) {
    values->emplace_back(value.c_str());
  }
  return values->data();
}

void BroadcastCreate(int32_t id, const char* type, const char* resource,
                     const char* access, const char* mux, const char* dst,
                     const char* vcodec, int32_t vb, const char* acodec,
                     int32_t ab) {
  std::shared_ptr<Media> media = Media::create(type, resource);

  std::unique_ptr<BroadcastConfiguration> configuration =
      std::make_unique<BroadcastConfiguration>(access, mux, dst, vcodec, vb,
                                               acodec, ab);
  g_broadcasts->Get(id, std::move(media), std::move(configuration));
}

void BroadcastStart(int32_t id) {
  Broadcast* broadcast = g_broadcasts->Get(id, nullptr, nullptr);
  broadcast->Start();
}

void BroadcastDispose(int32_t id) { g_broadcasts->Dispose(id); }

void ChromecastCreate(int32_t id, const char* type, const char* resource,
                      const char* ip_address) {
  std::shared_ptr<Media> media = Media::create(type, resource);
  chromecasts->Get(id, std::move(media), ip_address);
}

void ChromecastStart(int32_t id) {
  Chromecast* chromecast = chromecasts->Get(id, nullptr, "");
  chromecast->Start();
}

void ChromecastDispose(int32_t id) { chromecasts->Dispose(id); }

void RecordCreate(int32_t id, const char* saving_file, const char* type,
                  const char* resource) {
  std::shared_ptr<Media> media = Media::create(type, resource);
  g_records->Get(id, media, saving_file);
}

void RecordStart(int32_t id) {
  Record* record = g_records->Get(id, nullptr, "");
  record->Start();
}

void RecordDispose(int32_t id) { g_records->Dispose(id); }

DartDeviceList* DevicesAll(Dart_Handle object) {
  auto wrapper = new DartObjects::DeviceList();
  wrapper->devices = Devices::All();

  for (const auto& device : wrapper->devices) {
    wrapper->device_infos.emplace_back(device.name().c_str(),
                                       device.id().c_str());
  }

  wrapper->dart_object.size = wrapper->device_infos.size();
  wrapper->dart_object.device_infos = wrapper->device_infos.data();

  Dart_NewFinalizableHandle_DL(
      object, wrapper, sizeof(*wrapper),
      static_cast<Dart_HandleFinalizer>(DartObjects::DestroyObject));
  return &wrapper->dart_object;
}

static DartEqualizer* EqualizerToDart(const Equalizer* equalizer, int32_t id,
                                      Dart_Handle dart_handle) {
  auto wrapper = new DartObjects::Equalizer();
  for (const auto& [band, amp] : equalizer->band_amps()) {
    wrapper->bands.emplace_back(band);
    wrapper->amps.emplace_back(amp);
  }

  wrapper->dart_object.id = id;
  wrapper->dart_object.pre_amp = equalizer->pre_amp();
  wrapper->dart_object.bands = wrapper->bands.data();
  wrapper->dart_object.amps = wrapper->amps.data();
  wrapper->dart_object.size = wrapper->amps.size();

  Dart_NewFinalizableHandle_DL(
      dart_handle, wrapper, sizeof(*wrapper),
      static_cast<Dart_HandleFinalizer>(DartObjects::DestroyObject));

  return &wrapper->dart_object;
}

struct DartEqualizer* EqualizerCreateEmpty(Dart_Handle object) {
  int32_t id = g_equalizers->CreateEmpty();
  Equalizer* equalizer = g_equalizers->Get(id);
  return EqualizerToDart(equalizer, id, object);
}

struct DartEqualizer* EqualizerCreateMode(Dart_Handle object, int32_t mode) {
  int32_t id = g_equalizers->CreateMode(static_cast<EqualizerMode>(mode));
  Equalizer* equalizer = g_equalizers->Get(id);
  return EqualizerToDart(equalizer, id, object);
}

void EqualizerSetBandAmp(int32_t id, float band, float amp) {
  g_equalizers->Get(id)->SetBandAmp(band, amp);
}

void EqualizerSetPreAmp(int32_t id, float amp) {
  g_equalizers->Get(id)->SetPreAmp(amp);
}

#ifdef __cplusplus
}
#endif