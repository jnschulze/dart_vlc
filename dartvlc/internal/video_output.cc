#include "video_output.h"

#include <cassert>
#include <iostream>
#include <vlcpp/vlc.hpp>

namespace {
constexpr static char* kVLCFormatBGRA = "BGRA";
constexpr static char* kVLCFormatRGBA = "RGBA";
}  // namespace

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
  void Reset();
  void* OnVideoLock(void** planes);
  void OnVideoUnlock(void* picture, void* const* planes);
  void OnVideoPicture(void* picture);
  void SetDimensions(VideoDimensions&);
  void SendEmptyFrame();

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
      std::bind(&PixelBufferOutput::Impl::Reset, this));

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
  {
    const char* vlc_format;
    switch (pixel_format_) {
      case PixelFormat::kFormatBGRA:
        vlc_format = kVLCFormatBGRA;
      default:
        vlc_format = kVLCFormatRGBA;
    }
    auto len = std::char_traits<char>::length(vlc_format);
    assert(len == 4);
    memcpy(chroma, vlc_format, len);
  }

  uint32_t visible_width = 0, visible_height = 0;
  int video_track = libvlc_video_get_track(player_.get());
  if (video_track >= 0) {
    if (libvlc_video_get_size(player_.get(), video_track, &visible_width,
                              &visible_height) != 0) {
      std::cerr << "get size failed " << std::endl;
    }
  } else {
    std::cerr << "no video track" << std::endl;
  }

  *pitch = visible_width * 4;
  *lines = visible_height;

  std::cerr << "VISIBLE WIDTH is " << visible_width << std::endl;

  SetDimensions(VideoDimensions(visible_width, visible_height, *pitch));

  return 1;
}

void PixelBufferOutput::Impl::Reset() {
  SendEmptyFrame();
  SetDimensions(VideoDimensions());
}

void PixelBufferOutput::Impl::SendEmptyFrame() {
  void* buffer;
  auto user_data = delegate_->LockBuffer(&buffer, current_dimensions_);
  assert(buffer);
  memset(buffer, 0,
         current_dimensions_.bytes_per_row * current_dimensions_.height);
  delegate_->UnlockBuffer(user_data);
  delegate_->PresentBuffer(current_dimensions_, user_data);
}

void PixelBufferOutput::Impl::SetDimensions(VideoDimensions& dimensions) {
  if (current_dimensions_ != dimensions) {
    current_dimensions_ = dimensions;
    dimensions_changed_(dimensions.width, dimensions.height);
  }
}

void* PixelBufferOutput::Impl::OnVideoLock(void** planes) {
  auto user_data = delegate_->LockBuffer(&planes[0], current_dimensions_);
  assert(planes[0]);
  return user_data;
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
