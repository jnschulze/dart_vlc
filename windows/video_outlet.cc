#include "video_outlet.h"

#include <iostream>

struct FrameBuffer {
  explicit FrameBuffer(size_t size) : buffer_size(size) {
    data.reset(new uint8_t[size]);
    flutter_pixel_buffer.buffer = data.get();
  }

  FlutterDesktopPixelBuffer flutter_pixel_buffer;
  size_t buffer_size;
  std::unique_ptr<uint8_t[]> data;
  // TODO: Wait for https://github.com/flutter/engine/pull/28298 to get merged
  // std::mutex mutex;
};

VideoOutlet::VideoOutlet(flutter::TextureRegistrar* texture_registrar)
    : texture_registrar_(texture_registrar) {
  texture_ =
      std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
          [=](size_t width, size_t height) -> const FlutterDesktopPixelBuffer* {
            return CopyPixelBuffer(width, height);
          }));
  texture_id_ = texture_registrar_->RegisterTexture(texture_.get());
}

const FlutterDesktopPixelBuffer* VideoOutlet::CopyPixelBuffer(size_t width,
                                                              size_t height) {
  const std::lock_guard<std::mutex> lock(mutex_);

  // We have an old buffer that can now be released
  if (last_buffer_) {
    last_buffer_.reset();
  }

  if (current_buffer_) {
    return &current_buffer_->flutter_pixel_buffer;
  }

  return nullptr;
}

void* VideoOutlet::LockBuffer(void** buffer,
                              const VideoDimensions& dimensions) {
  assert(dimensions.bytes_per_row == dimensions.width * 4);
  size_t required_size = dimensions.bytes_per_row * dimensions.height;

  {
    const std::lock_guard<std::mutex> lock(mutex_);
    if (!current_buffer_ || current_buffer_->buffer_size < required_size) {
      // The old buffer must not be released
      // yet as it might be in use by Flutter.
      // So we store it in last_buffer_ and release
      // it in the CopyPixelBuffer callback.
      current_buffer_.swap(last_buffer_);
      current_buffer_ = std::make_unique<FrameBuffer>(required_size);
    }
    current_buffer_->flutter_pixel_buffer.width = dimensions.width;
    current_buffer_->flutter_pixel_buffer.height = dimensions.height;
  }

  *buffer = current_buffer_->data.get();
  // current_buffer_->mutex.lock();
  return current_buffer_.get();
}

void VideoOutlet::UnlockBuffer(void* user_data) {
  // auto buffer = reinterpret_cast<FrameBuffer*>(user_data);
  // buffer->mutex.unlock();
}

void VideoOutlet::PresentBuffer(const VideoDimensions& dimensions,
                                void* user_data) {
  texture_registrar_->MarkTextureFrameAvailable(texture_id_);
}

VideoOutlet::~VideoOutlet() {
  texture_registrar_->UnregisterTexture(texture_id_);
}
