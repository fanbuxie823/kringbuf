#ifndef K_RING_BUFF_H_
#define K_RING_BUFF_H_
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <mutex>
#ifdef DEBUG_
#include <mutex>
#include <iostream>
#endif

#define MIN(A, B) ((A) < (B) ? (A):(B))

template<typename DATATYPE>
class KRingBuff {
 private:
  uint mask_;
  std::atomic<uint> write_offset_;
  std::atomic<uint> read_offset_;
  DATATYPE *buff_;

  void AllocateBuff(uint size) {
	  delete[] buff_;
	  mask_ = size - 1;
	  if (!IsPowerOf2(size)) {
		  mask_ = RoundUpPowerOf2(size) - 1;
	  }
	  buff_ = new DATATYPE[mask_ + 1];
	  memset(buff_, 0, (mask_ + 1) * sizeof(DATATYPE));
  }

  uint RoundUpPowerOf2(uint v) {
	  v--;
	  v |= v >> 1;
	  v |= v >> 2;
	  v |= v >> 4;
	  v |= v >> 8;
	  v |= v >> 16;
	  v++;
	  return v;
  }

  static bool IsPowerOf2(uint n) {
	  return (n != 0 && ((n & (n - 1)) == 0));
  }

 public:
  explicit KRingBuff(uint min_buff_size) : mask_(0), write_offset_(0), read_offset_(0), buff_(nullptr) {
	  AllocateBuff(min_buff_size);
  }

  ~KRingBuff() {
	  delete[] buff_;
	  buff_ = nullptr;
  }

  KRingBuff() = delete;
  KRingBuff(const KRingBuff &) = delete; //不可移动
  KRingBuff &operator=(const KRingBuff &) = delete; //不可拷贝

  uint WriteIn(const DATATYPE *in_buff, uint in_size) {
  	uint free_len = GetFreeLen();
	  uint write_len = MIN(free_len, in_size);
	  uint write_start_pos = write_offset_.load(std::memory_order_acquire) & mask_;

	  // write first part of in_buff, which should put between write_offset and end_pos of buff;
	  auto first_len = MIN(write_len, mask_ - write_start_pos + 1);
	  std::copy(in_buff, in_buff + first_len, buff_ + write_start_pos);
	  // write rest part of in_buff, for the start pos of buff;
	  std::copy(in_buff + first_len, in_buff + write_len, buff_);

	  // make sure all write is done, before refresh the write_offset
	  write_offset_.store(write_offset_ + write_len, std::memory_order_release);
	  return write_len;
  }

  uint ReadOut(DATATYPE *out_buff, uint out_size) {
  	uint used_len = GetUsedLen();
	  uint read_len = MIN(used_len, out_size);
	  uint read_start_pos = read_offset_.load(std::memory_order_acquire) & mask_;

	  // read first part of in_buff, which should locate between write_offset and end_buff;
	  auto first_len = MIN(read_len, (mask_ + 1) - read_start_pos);
	  auto buff_pos = buff_ + read_start_pos;
	  std::copy(buff_pos, buff_pos + first_len, out_buff);
	  // write rest part of in_buff, for the start_buff;
	  std::copy(buff_, buff_ + (read_len - first_len), out_buff + first_len);

	  // make sure all read is done, before refresh the write_offset
	  read_offset_.store(read_offset_ + read_len, std::memory_order_release);
	  return read_len;
  }

  uint GetBuffLen() const {
	  return mask_ + 1;
  }

  uint GetUsedLen() const {
	  //load value and make sure all write is done
	  return write_offset_.load(std::memory_order_acquire) - read_offset_.load(std::memory_order_acquire);
  }

  uint GetFreeLen() const {
	  return (mask_ + 1)
		  - (write_offset_.load(std::memory_order_acquire) - read_offset_.load(std::memory_order_acquire));
  }

  void Reset(uint size) {
	  if (size)
		  AllocateBuff(size);
	  write_offset_ = 0;
	  read_offset_ = 0;
  }

#ifdef DEBUG_
  std::mutex mtx_;
  void dump() {
	  std::lock_guard<std::mutex> lck(mtx_);
	  for (int i = 0; i < GetBuffLen(); ++i) {
		  std::cout << buff_[i] << " ";
		  if ((i + 1) % 16 == 0)
			  std::cout << "\n";
	  }
  }
#endif

};

#endif