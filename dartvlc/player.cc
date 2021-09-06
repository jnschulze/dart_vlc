#include "player.h"

static auto TO_CHARARRAY(const std::vector<std::string>& vector) {
  size_t size = vector.size();
  auto array = std::unique_ptr<const char*[]>(new const char*[size]);

  for (auto i = 0; i < size; i++) {
    array[i] = vector[i].c_str();
  }

  return array;
}

Player::Player(const std::vector<std::string>& cmd_arguments) {
  if (cmd_arguments.empty()) {
    vlc_instance_ = VLC::Instance(0, nullptr);
  } else {
    auto args = TO_CHARARRAY(cmd_arguments);
    vlc_instance_ =
        VLC::Instance(static_cast<int32_t>(cmd_arguments.size()), args.get());
  }
  vlc_media_player_ = VLC::MediaPlayer(vlc_instance_);
  vlc_media_list_player_ = VLC::MediaListPlayer(vlc_instance_);
  vlc_media_list_ = VLC::MediaList(vlc_instance_);
  vlc_media_list_player_.setMediaPlayer(vlc_media_player_);
  state_ = std::make_unique<PlayerState>();
  vlc_media_player_.setVolume(100);
}

void Player::SetVideoOutput(std::unique_ptr<VideoOutput> output) {
  video_output_ = std::move(output);
  video_output_->Attach(vlc_media_player_);
  video_output_->OnDimensionsChanged([&](size_t width, size_t height) {
    video_dimension_callback_(width, height);
  });
}

Player::~Player() { vlc_media_player_.stop(); }
