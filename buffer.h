#pragma once

#include <limits>
#include <vector>

class Buffer {
 public:
  Buffer();
  Buffer(Buffer&& other) : data_(std::move(other.data_)), pos_(other.pos_) {}

  std::pair<const char*, size_t> GetReadableBuffer();
  void Consume(size_t size);
  Buffer Split(size_t size);
  void MaybeShrink();

  std::pair<char*, size_t> GetWritableBuffer();
  std::pair<char*, size_t> AllocWritableBuffer(
      size_t min_size_needed, bool align = false);
  void Commit(size_t size);

 private:
  std::vector<char> data_;
  size_t pos_;

 private:
  Buffer(const Buffer&) = delete;
  void operator= (const Buffer&) = delete;
};