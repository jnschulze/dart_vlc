#include "video_output.h"

#include <cassert>
#include <iostream>
#include <vlcpp/vlc.hpp>

class PixelBufferOutput::Impl {
 public:
  Impl(std::unique_ptr<PixelBufferOutputDelegate> delegate,
       PixelFormat pixel_format)
      : delegate_(std::move(delegate)), pixel_format_(pixel_format) {}
  void Attach(VLC::MediaPlayer vlc_player);
  void OnDimensionsChanged(VideoDimensionsCallback dimensions_callback) {
    dimensions_changed_ = dimensions_callback;
  }
  PixelBufferOutputDelegate* output_delegate() const { return delegate_.get(); }

 private:
  std::unique_ptr<PixelBufferOutputDelegate> delegate_;
  VLC::MediaPlayer player_;
  PixelFormat pixel_format_;
  int32_t SetupFormat(char* chroma, uint32_t* width, uint32_t* height,
                      uint32_t* pitch, uint32_t* lines);
  void* OnVideoLock(void** planes);
  void OnVideoUnlock(void* picture, void* const* planes);
  void OnVideoPicture(void* picture);

  VideoDimensions current_dimensions_{};
  VideoDimensionsCallback dimensions_changed_;
};

void PixelBufferOutput::Impl::Attach(VLC::MediaPlayer vlc_player) {
  player_ = vlc_player;

  player_.setVideoFormatCallbacks(
      std::bind(&PixelBufferOutput::Impl::SetupFormat, this,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3, std::placeholders::_4,
                std::placeholders::_5),
      nullptr);

  player_.setVideoCallbacks(
      std::bind(&PixelBufferOutput::Impl::OnVideoLock, this,
                std::placeholders::_1),
      std::bind(&PixelBufferOutput::Impl::OnVideoUnlock, this,
                std::placeholders::_1, std::placeholders::_2),
      std::bind(&PixelBufferOutput::Impl::OnVideoPicture, this,
                std::placeholders::_1));
}

int32_t PixelBufferOutput::Impl::SetupFormat(char* chroma, uint32_t* width,
                                             uint32_t* height, uint32_t* pitch,
                                             uint32_t* lines) {
  assert(player_.isValid());

  switch (pixel_format_) {
    case PixelFormat::kFormatBGRA:
      memcpy(chroma, "BGRA", 4);
    default:
      memcpy(chroma, "RGBA", 4);
  }

  uint32_t visible_width, visible_height;
  libvlc_video_get_size(player_.get(), 0, &visible_width, &visible_height);
  *pitch = visible_width * 4;
  *lines = visible_height;

  if (current_dimensions_.width != visible_width ||
      current_dimensions_.height != visible_height) {
    current_dimensions_.width = visible_width;
    current_dimensions_.height = visible_height;
    current_dimensions_.bytes_per_row = *pitch;
    dimensions_changed_(visible_width, visible_height);
  }

  return 1;
}

void* PixelBufferOutput::Impl::OnVideoLock(void** planes) {
  return delegate_->LockBuffer(&planes[0], current_dimensions_);
}

void PixelBufferOutput::Impl::OnVideoUnlock(void* user_data,
                                            void* const* planes) {
  delegate_->UnlockBuffer(user_data);
}

void PixelBufferOutput::Impl::OnVideoPicture(void* user_data) {
  delegate_->PresentBuffer(current_dimensions_, user_data);
}

PixelBufferOutput::PixelBufferOutput(
    std::unique_ptr<PixelBufferOutputDelegate> delegate, PixelFormat format)
    : impl_(std::make_unique<Impl>(std::move(delegate), format)) {}

void PixelBufferOutput::Attach(VLC::MediaPlayer vlc_player) {
  impl_->Attach(vlc_player);
}

void PixelBufferOutput::OnDimensionsChanged(
    VideoDimensionsCallback dimensions_callback) {
  impl_->OnDimensionsChanged(dimensions_callback);
}

PixelBufferOutputDelegate* PixelBufferOutput::output_delegate() const {
  return impl_->output_delegate();
}
