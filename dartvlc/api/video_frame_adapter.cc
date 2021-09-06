#include "video_frame_adapter.h"

#include <cassert>

void* VideoFrameAdapter::LockBuffer(void** buffer,
                                    const VideoDimensions& dimensions) {
  size_t required_size = dimensions.bytes_per_row * dimensions.height;

  if (!buffer_ || buffer_size_ < required_size) {
    buffer_.reset(new uint8_t[required_size]);
  }

  *buffer = buffer_.get();
  return buffer_.get();
}

void VideoFrameAdapter::PresentBuffer(const VideoDimensions& dimensions,
                                      void* buffer) {
  if (frame_callback_) {
    frame_callback_(static_cast<uint8_t*>(buffer), dimensions);
  }
}
