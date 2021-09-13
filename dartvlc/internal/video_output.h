#ifndef VIDEO_OUTPUT_H_
#define VIDEO_OUTPUT_H_

#include <functional>
#include <memory>

namespace VLC {
class MediaPlayer;
}

typedef std::function<void(size_t width, size_t height)>
    VideoDimensionsCallback;

enum class PixelFormat { kNone = 0, kFormatRGBA, kFormatBGRA };

struct VideoDimensions {
  uint32_t width;
  uint32_t height;
  uint32_t bytes_per_row;

  VideoDimensions() : width(0), height(0), bytes_per_row(0) {}

  VideoDimensions(uint32_t width, uint32_t height, uint32_t bytes_per_row)
      : width(width), height(height), bytes_per_row(bytes_per_row) {}

  bool operator==(const VideoDimensions& other) const {
    return width == other.width && height == other.height &&
           bytes_per_row == other.bytes_per_row;
  }
  bool operator!=(const VideoDimensions& other) const {
    return !operator==(other);
  }
};

class VideoOutput {
 public:
  virtual ~VideoOutput() = default;
  virtual void Attach(VLC::MediaPlayer) = 0;
  virtual void OnDimensionsChanged(
      VideoDimensionsCallback dimensions_callback) = 0;
};

class PixelBufferOutputDelegate {
 public:
  virtual void* LockBuffer(void** buffer,
                           const VideoDimensions& dimensions) = 0;
  virtual void UnlockBuffer(void* user_data){};
  virtual void PresentBuffer(const VideoDimensions& dimensions,
                             void* user_data) = 0;
  virtual ~PixelBufferOutputDelegate() = default;
};

class PixelBufferOutput : public VideoOutput {
 public:
  PixelBufferOutput(std::unique_ptr<PixelBufferOutputDelegate> delegate,
                    PixelFormat format);
  void Attach(VLC::MediaPlayer) override;
  void OnDimensionsChanged(
      VideoDimensionsCallback dimensions_callback) override;
  PixelBufferOutputDelegate* output_delegate() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
