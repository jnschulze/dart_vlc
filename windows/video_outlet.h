#ifndef VIDEO_OUTLET_H_
#define VIDEO_OUTLET_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <iostream>
#include <list>
#include <mutex>

#include "internal/video_output.h"

struct FrameBuffer;

class VideoOutlet : public PixelBufferOutputDelegate {
 public:
  VideoOutlet(flutter::TextureRegistrar* texture_registrar);

  int64_t texture_id() const { return texture_id_; }

  virtual void* LockBuffer(void** buffer,
                           const VideoDimensions& dimensions) override;
  virtual void UnlockBuffer(void* user_data) override;
  virtual void PresentBuffer(const VideoDimensions& dimensions, void* user_data) override;

  virtual ~VideoOutlet();

 private:
  std::unique_ptr<FrameBuffer> current_buffer_;
  std::unique_ptr<FrameBuffer> last_buffer_;

  flutter::TextureRegistrar* texture_registrar_ = nullptr;
  std::unique_ptr<flutter::TextureVariant> texture_ = nullptr;
  int64_t texture_id_;
  mutable std::mutex mutex_;

  const FlutterDesktopPixelBuffer* CopyPixelBuffer(size_t width, size_t height);
};

#endif
