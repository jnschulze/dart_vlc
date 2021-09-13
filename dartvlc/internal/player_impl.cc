#include "player_impl.h"

#include <future>
#include <iostream>

namespace {

static auto TO_CHARARRAY(const std::vector<std::string>& vector) {
  size_t size = vector.size();
  auto array = std::unique_ptr<const char*[]>(new const char*[size]);

  for (auto i = 0; i < size; i++) {
    array[i] = vector[i].c_str();
  }

  return array;
}

PlaybackState GetPlaybackState(libvlc_media_player_t* player) {
  auto state = libvlc_media_player_get_state(player);
  switch (state) {
    case libvlc_Opening:
      return PlaybackState::kOpening;
    case libvlc_Buffering:
      return PlaybackState::kBuffering;
    case libvlc_Playing:
      return PlaybackState::kPlaying;
    case libvlc_Paused:
      return PlaybackState::kPaused;
    case libvlc_Stopped:
      return PlaybackState::kStopped;
    case libvlc_Ended:
      return PlaybackState::kEnded;
    case libvlc_Error:
      return PlaybackState::kError;
    default:
      return PlaybackState::kNone;
  }
}

}  // namespace

Player::Impl::Impl(const std::vector<std::string>& cmd_arguments) {
  if (cmd_arguments.empty()) {
    vlc_instance_ = VLC::Instance(0, nullptr);
  } else {
    auto args = TO_CHARARRAY(cmd_arguments);
    vlc_instance_ =
        VLC::Instance(static_cast<int32_t>(cmd_arguments.size()), args.get());
  }
  vlc_media_player_ = VLC::MediaPlayer(vlc_instance_);
  vlc_media_list_player_ = VLC::MediaListPlayer(vlc_instance_);
  vlc_media_list_player_.setMediaPlayer(vlc_media_player_);
  vlc_media_player_.setVolume(100);

  SetupEventHandlers();

  libvlc_log_set(vlc_instance_.get(), [](void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list arg) {

  }, nullptr);
}

void Player::Impl::SetVideoOutput(std::unique_ptr<VideoOutput> output) {
  video_output_ = std::move(output);
  video_output_->OnDimensionsChanged([&](size_t width, size_t height) {
    video_dimension_callback_(width, height);
  });
  video_output_->Attach(vlc_media_player_);
}

VideoOutput* Player::Impl::GetVideoOutput() const {
  return video_output_.get();
}

void Player::Impl::SetupEventHandlers() {
  auto& event_manager = vlc_media_player_.eventManager();

  event_manager.onOpening(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kOpening));

  event_manager.onBuffering([=](float p) {
    /*
    bool is_buffering = p < 100.0f;
    if (is_buffering) {
      HandleVlcState(PlaybackState::kBuffering);
    } else {
      auto state = GetPlaybackState(vlc_media_player_.get());
      HandleVlcState(state);
    }
    */
  });

  event_manager.onNothingSpecial(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kNone));
  event_manager.onOpening(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kOpening));
  event_manager.onPlaying(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kPlaying));
  event_manager.onPaused(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kPaused));
  event_manager.onStopped(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kStopped));
  event_manager.onEndReached(
      std::bind(&Player::Impl::HandleVlcState, this, PlaybackState::kEnded));
  event_manager.onMediaChanged(std::bind(&Player::Impl::HandleMediaChanged,
                                         this, std::placeholders::_1));
  event_manager.onPositionChanged(std::bind(
      &Player::Impl::HandlePositionChanged, this, std::placeholders::_1));
  event_manager.onSeekableChanged(std::bind(
      &Player::Impl::HandleSeekableChanged, this, std::placeholders::_1));
}

void Player::Impl::HandleVlcState(PlaybackState state) {
  //std::lock_guard<std::mutex> lock(state_mutex_);

  auto pending_seek_position = state_.pending_seek.value_or(-1);
  if (state == PlaybackState::kPlaying && pending_seek_position != -1) {
    state_.pending_seek.reset();

    auto duration = vlc_media_player_.length();
    if (duration != -1) {
      // for some reason, setTime() will deadlock. So use setPosition instead
      vlc_media_player_.setPosition(static_cast<float>(pending_seek_position) /
                                    static_cast<float>(duration));
    }
  }

  if (state_.playback_state != state) {
    state_.playback_state = state;

    switch (state) {
      case PlaybackState::kStopped:
        state_.duration = 0;
        state_.position = 0;
        break;
      default:
        break;
    }
    if (playback_state_changed_) {
      playback_state_changed_(state);
    }
  }
}

std::unique_ptr<Playlist> Player::Impl::CreatePlaylist() {
  return std::make_unique<PlaylistImpl>(vlc_instance_);
}

void Player::Impl::OnPlaylistUpdated() {
  // std::lock_guard<std::mutex> lock(state_mutex_);

  std::cerr << "PLAYLIST WAS UPDATED" << std::endl;

  vlc_media_list_player_.setMediaList(playlist_->vlc_media_list());
  auto length = playlist_->length();

  if (!length) {
    state_.Reset();
    Stop();
    return;
  }

  state_.index = std::min(state_.index, static_cast<int32_t>(length) - 1);

  // playlist_callback_();
}

void Player::Impl::Open(std::shared_ptr<Playlist> playlist) {
  std::lock_guard<std::mutex> lock(state_mutex_);

  auto impl = std::dynamic_pointer_cast<PlaylistImpl>(playlist);
  if (!impl) {
    return;
  }

  if (playlist_) {
    playlist_->OnUpdate(nullptr);
  }

  playlist_ = std::move(impl);
  playlist_->OnUpdate(std::bind(&Player::Impl::OnPlaylistUpdated, this));

  vlc_media_list_player_.setPlaybackMode(libvlc_playback_mode_loop);

  OnPlaylistUpdated();

  const auto& medias = playlist_->medias();
  if (!medias.empty() && open_callback_) {
    open_callback_(medias[0], 0);
  }
}

void Player::Impl::Play() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  vlc_media_list_player_.play();
}

void Player::Impl::Pause() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  vlc_media_list_player_.pause();
}

void Player::Impl::PlayOrPause() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  if (vlc_media_list_player_.isPlaying()) {
    Pause();
  } else {
    Play();
  }
}

void Player::Impl::Stop() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  vlc_media_list_player_.stop();
}

void Player::Impl::Jump(int32_t index) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  if (index >= 0 && index <= playlist_->length()) {
    vlc_media_list_player_.playItemAtIndex(index);
  }
}

void Player::Impl::Next() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  if (state_.index < playlist_->length()) {
    vlc_media_list_player_.playItemAtIndex(++state_.index);
  }
}

void Player::Impl::Back() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  if (state_.index > 0) {
    vlc_media_list_player_.playItemAtIndex(--state_.index);
  }
}

void Player::Impl::Seek(int64_t position) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  auto state = state_.playback_state;

  switch (state) {
    case PlaybackState::kPlaying:
      vlc_media_player_.setTime(position);
    case PlaybackState::kNone:
    case PlaybackState::kOpening:
    case PlaybackState::kBuffering:
    case PlaybackState::kPaused:
    case PlaybackState::kStopped:
    case PlaybackState::kEnded:
      state_.pending_seek = position;
    default:
      break;
  }
}

void Player::Impl::SetVolume(float volume) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  vlc_media_player_.setVolume(static_cast<int32_t>(volume * 100));
  if (volume_callback_) {
    volume_callback_(volume);
  }
}

void Player::Impl::SetRate(float rate) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  vlc_media_player_.setRate(rate);
  if (rate_callback_) {
    rate_callback_(rate);
  }
}

void Player::Impl::HandleMediaChanged(VLC::MediaPtr vlc_media_ptr) {
  if (!playlist_) {
    return;
  }

  auto index = playlist_->GetIndexOfItem(*vlc_media_ptr.get());
  if (!index.has_value()) {
    return;
  }

  state_.index = index.value();
  state_.current_item = vlc_media_ptr;
  std::shared_ptr<Media> media;

  std::cerr << "!!! MEDIA ITEM CHANGED" << state_.index << std::endl;

  if (open_callback_ && playlist_->TryGetItem(state_.index, media)) {
    open_callback_(std::move(media), state_.index);
  }
}

void Player::Impl::HandlePositionChanged(float position) {
  // state_.position = static_cast<int32_t>(relative_position *
  // vlc_media_player_.length());

  double pos = std::clamp(static_cast<double>(position), 0.0, 1.0);
  state_.position = pos;
  if (position_callback_) {
    position_callback_(pos);
  }
}

void Player::Impl::HandleSeekableChanged(bool is_seekable) {
  state_.is_seekable = is_seekable;
}
