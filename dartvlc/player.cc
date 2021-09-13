#include "player.h"

#include <iostream>

#include "internal/player_impl.h"

Player::Player(const std::vector<std::string>& cmd_arguments)
    : impl_(std::make_unique<Impl>(cmd_arguments)) {}

void Player::SetVideoOutput(std::unique_ptr<VideoOutput> output) {
  impl_->SetVideoOutput(std::move(output));
}

VideoOutput* Player::GetVideoOutput() const { return impl_->GetVideoOutput(); }

void Player::OnOpen(OpenCallback callback) {
  impl_->OnOpen(std::move(callback));
}

void Player::OnVideoDimensions(VideoDimensionsChangedCallback callback) {
  impl_->OnVideoDimensions(std::move(callback));
}

void Player::OnPosition(std::function<void(double)> callback) {
  impl_->OnPosition(std::move(callback));
}

void Player::OnSeekable(std::function<void(bool)> callback) {
  impl_->OnSeekable(std::move(callback));
}

void Player::OnComplete(std::function<void()> callback) {
  impl_->OnComplete(std::move(callback));
}

void Player::OnVolume(std::function<void(float)> callback) {
  impl_->OnVolume(std::move(callback));
}

void Player::OnRate(std::function<void(float)> callback) {
  impl_->OnRate(std::move(callback));
}

void Player::OnPlaylist(std::function<void()> callback) {
  impl_->OnPlaylist(std::move(callback));
}

void Player::OnPlaybackStateChanged(PlaybackStateChangedCallback callback) {
  impl_->OnPlaybackStateChanged(std::move(callback));
}

PlaybackState Player::playback_state() { return PlaybackState::kNone; }

// int32_t Player::video_width() { return video_width_; }

// int32_t video_height() const { return video_height_; }

int32_t Player::duration() { return impl_->duration(); }

double Player::position() { return impl_->position(); }

float Player::volume() { return impl_->volume(); }

float Player::rate() { return impl_->rate(); }

// PlaybackState Player::playback_state() { return impl_->playback_state(); }

Playlist* Player::playlist() const { return impl_->playlist(); }

void Player::Open(std::shared_ptr<Playlist> media_source, bool auto_start) {
  std::cerr << "OPEN" << std::endl;
  impl_->Open(std::move(media_source));
}

void Player::Play() { impl_->Play(); }
void Player::PlayOrPause() { impl_->PlayOrPause(); }
void Player::Pause() { impl_->Pause(); }
void Player::Stop() { impl_->Stop(); }

void Player::Next() { impl_->Next(); }
void Player::Back() { impl_->Back(); }
void Player::Jump(int i) { impl_->Jump(i); }
void Player::Seek(int position) { impl_->Seek(position); }
void Player::SetVolume(float level) { impl_->SetVolume(level); }
void Player::SetRate(float rate) { impl_->SetRate(rate); }
void Player::SetAudioDevice(const Device& device) {}

void Player::SetPlaylistMode(PlaylistMode p) {}

void Player::SetEqualizer(Equalizer x) {}

void Player::SetUserAgent(std::string userAgent) {}

void Player::TakeSnapshot(std::string file_path, int32_t width,
                          int32_t height) {}
void Player::SetVideoWidth(int32_t video_width) {}
void Player::SetVideoHeight(int32_t video_height) {}

std::unique_ptr<Playlist> Player::CreatePlaylist() {
  return impl_->CreatePlaylist();
}

Player::~Player() { /*vlc_media_player_.stop();*/
}
