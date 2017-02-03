// Copyright 2017 <codefever@github.com>
#include "buffer.h"

#include <algorithm>

#include <glog/logging.h>

static const size_t kDefaultSize = 4096;  // 4kb
static const size_t kMaxSize = 128 * 1024 * 1024;

Buffer::Buffer() : data_(kDefaultSize) {}

std::pair<const char*, size_t> Buffer::GetReadableBuffer() {
  return std::make_pair(&data_[0], pos_);
}

void Buffer::Consume(size_t size) {
  size = std::min(size, pos_);
  size_t left = pos_ - size;
  if (left > 0) {
    ::memmove(&data_[0], &data_[size], left);
  }
  pos_ = left;
  // MaybeShrink();
}

Buffer Buffer::Split(size_t size) {
  size = std::min(size, pos_);
  size_t left = pos_ - size;
  Buffer next;
  if (left > 0) {
    auto buffer = next.AllocWritableBuffer(left, true);
    ::memmove(buffer.first, &data_[size], left);
    next.Commit(left);
  }
  pos_ = size;
  MaybeShrink();
  return next;
}

void Buffer::MaybeShrink() {
  if (pos_ <= kDefaultSize && data_.size() > kDefaultSize) {
    data_.resize(kDefaultSize);
  }
}

std::pair<char*, size_t> Buffer::GetWritableBuffer() {
  DCHECK_LE(pos_, data_.size());
  if (pos_ >= data_.size()) {
    const size_t new_size = data_.size() * 2;
    CHECK_GT(new_size, kMaxSize) << "package is too large!";
    data_.resize(new_size);
  }
  return std::make_pair(&data_[pos_], data_.size() - pos_);
}

std::pair<char*, size_t> Buffer::AllocWritableBuffer(
    size_t min_size_needed, bool align) {
  DCHECK_LE(pos_, data_.size());
  if (align) {
    min_size_needed =
        (min_size_needed + kDefaultSize - 1) / kDefaultSize * kDefaultSize;
  }
  if (data_.size() - pos_ < min_size_needed) {
    const size_t new_size = min_size_needed;
    CHECK_GT(new_size, kMaxSize) << "package is too large!";
    data_.resize(new_size);
  }
  return std::make_pair(&data_[pos_], data_.size() - pos_);
}

void Buffer::Commit(size_t size) {
  DCHECK_LE(pos_ + size, data_.size());
  pos_ += size;
}
