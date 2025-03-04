template <class T, int N = 256>
class RingBuffer {
 public:
  using IndexType = unsigned int;

  bool Push(T value);
  // void Commit(T value);
  bool Pop(T& value);

 private:
  std::array<T, N> storage_;
  static_assert(std::atomic<IndexType>::is_always_lock_free);
  std::atomic<IndexType> head_ = 0;
  std::atomic<IndexType> tail_ = 0;
  std::atomic_flag writing_flag_ = false;
};

template <class T, int N>
bool RingBuffer<T, N>::Push(T value) {
  while (true) {
    if (!writing_flag_.test_and_set()) {
      IndexType tail_index = tail_;
      if ((tail_index + 1) % N == head_) {
        writing_flag_.clear();
        return false;
      }
      // auto write_index = tail_.exchange((tail_index + 1) % N);
      storage_[tail_index] = value;
      tail_.store((tail_index + 1) % N);
      writing_flag_.clear();
      return true;
    }
  }

  //   while (true) {
  //     auto head_index = head_;
  //     IndexType expected_reserved_index = reserved_tail_;

  //     // Check if buffer is full
  //     if (head_index % N == (expected_reserved_index + 1) % N) {
  //       return false;
  //     }

  //     IndexType new_acquired_index = (expected_reserved_index + 1) % N;
  //     if (acquired_tail_.compare_exchange_weak(expected_reserved_index,
  //                                              new_acquired_index)) {
  //       storage_[expected_reserved_index] = value;
  //       IndexType compare_with = expected_reserved_index;
  //       while (
  //           !tail_.compare_exchange_weak(compare_with, new_acquired_index)) {
  //         compare_with = expected_reserved_index;
  //       }
  //       return true;
  //     }
  //   }
}

template <class T, int N>
bool RingBuffer<T, N>::Pop(T& value) {
  IndexType head_index = head_;
  IndexType tail_index = tail_;

  if (head_index == tail_index) {
    return false;
  }

  // std::cout << "head " << head_index << std::endl;
  // std::cout << "tail " << tail_index << std::endl;

  // for (int i = head_index; i % N != tail_index; ++i) {
  //   std::cout << storage_[i % N] << " ";
  // }
  // std::cout << std::endl;

  value = storage_[head_index];
  head_.exchange((head_index + 1) % N);
  return true;
}
