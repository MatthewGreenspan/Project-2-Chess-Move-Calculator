#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using namespace std;
template <typename V>
class StringHashMap {
 public:
  struct Entry {
    // one stored key/value pair
    string key;
    V value;
  };

  explicit StringHashMap(size_t bucketCount = 4096) : buckets_(bucketCount) {}

  size_t size() const { return size_; }

  void clear() {
    for (auto& b : buckets_) b.clear();
    size_ = 0;
  }

  V* find(const string& key) {
    size_t h = hash_(key) % buckets_.size();
    for (auto& e : buckets_[h])
      if (e.key == key) return &e.value;
    return nullptr;
  }

  const V* find(const string& key) const {
    return const_cast<StringHashMap*>(this)->find(key);
  }

  V& operator[](const string& key) {
    size_t h = hash_(key) % buckets_.size();
    for (auto& e : buckets_[h])
      if (e.key == key) return e.value;
    buckets_[h].push_back({key, V{}});
    ++size_;
    return buckets_[h].back().value;
  }

  // erases by swapping with the last one first
  void erase(const string& key) {
    size_t h = hash_(key) % buckets_.size();
    auto& b = buckets_[h];
    for (size_t i = 0; i < b.size(); ++i) {
      if (b[i].key == key) {
        b[i] = static_cast<Entry&&>(b.back());
        b.pop_back();
        --size_;
        return;
      }
    }
  }

  template <typename Fn>
  void forEach(Fn&& fn) {
    for (auto& bucket : buckets_) {
      for (auto& e : bucket) fn(e.key, e.value);
    }
  }

  template <typename Fn>
  void forEach(Fn&& fn) const {
    for (const auto& bucket : buckets_) {
      for (const auto& e : bucket) fn(e.key, e.value);
    }
  }

 private:
  // basic hash for spreading strings into buckets
  static size_t hash_(const string& s) {
    size_t h = 14695981039346656037ull;
    for (unsigned char c : s) h = (h ^ c) * 1099511628211ull;
    return h;
  }

  // bucket table for all stored entries
  vector<vector<Entry>> buckets_;
  size_t size_ = 0;
};
