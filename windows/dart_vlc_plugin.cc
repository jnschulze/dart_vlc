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

#include "include/dart_vlc/dart_vlc_plugin.h"

#include <unordered_map>

#include "player_registry.h"
#include "video_outlet.h"

namespace {

VideoOutlet* GetOutlet(const Player* player) {
  auto output = dynamic_cast<PixelBufferOutput*>(player->GetVideoOutput());
  if (!output) {
    return nullptr;
  }

  return dynamic_cast<VideoOutlet*>(output->output_delegate());
}

class DartVlcPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  flutter::MethodChannel<flutter::EncodableValue>* channel() const {
    return channel_.get();
  }

  DartVlcPlugin(
      std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel,
      flutter::TextureRegistrar* texture_registrar);

  virtual ~DartVlcPlugin();

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  flutter::TextureRegistrar* texture_registrar_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
};

void DartVlcPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto plugin = std::make_unique<DartVlcPlugin>(
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "dart_vlc",
          &flutter::StandardMethodCodec::GetInstance()),
      registrar->texture_registrar());
  plugin->channel()->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });
  registrar->AddPlugin(std::move(plugin));
}

DartVlcPlugin::DartVlcPlugin(
    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel,
    flutter::TextureRegistrar* texture_registrar)
    : channel_(std::move(channel)), texture_registrar_(texture_registrar) {}

DartVlcPlugin::~DartVlcPlugin() {}

void DartVlcPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name() == "PlayerRegisterTexture") {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    auto player_id =
        std::get<int64_t>(arguments[flutter::EncodableValue("playerId")]);

    Player* player = g_players->Get(player_id);
    if (!player) {
      return result->Error("invalid_id", "No such player");
    }

    int64_t texture_id = -1;
    if (!player->HasVideoOutput()) {
      auto outlet = std::make_unique<VideoOutlet>(texture_registrar_);
      texture_id = outlet->texture_id();
      player->SetVideoOutput(std::make_unique<PixelBufferOutput>(
          std::move(outlet), PixelFormat::kFormatRGBA));
    } else {
      auto outlet = GetOutlet(player);
      if (outlet) {
        texture_id = outlet->texture_id();
      }
    }

    return result->Success(flutter::EncodableValue(texture_id));
  } else {
    result->NotImplemented();
  }
}
}  // namespace

void DartVlcPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  DartVlcPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
