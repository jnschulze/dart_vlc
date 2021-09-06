#ifndef VIDEO_ADAPTER_H_
#define VIDEO_ADAPTER_H_

#include <iostream>
#include <list>
#include <mutex>

#include "internal/video_output.h"

typedef std::function<void(const uint8_t*, const VideoDimensions&)>
    VideoFrameCallback;

class VideoFrameAdapter : public PixelBufferOutputDelegate {
 public:
  VideoFrameAdapter() = default;

  void OnFrameArrived(VideoFrameCallback callback) {
    frame_callback_ = callback;
  }

  // |PixelBufferOutputDelegate|
  virtual void* LockBuffer(void** buffer,
                           const VideoDimensions& dimensions) override;
  // |PixelBufferOutputDelegate|
  virtual void PresentBuffer(const VideoDimensions& dimensions,
                             void* user_data) override;

  virtual ~VideoFrameAdapter();

 private:
  std::mutex mutex_;
  std::unique_ptr<uint8_t[]> buffer_;
  size_t buffer_size_ = 0;
  VideoFrameCallback frame_callback_;
};

#endif
