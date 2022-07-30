#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

#include "kringbuf.hpp"

using namespace std;

atomic_bool thread_running;

constexpr const int item_count = 1000;

//每秒消费五个，固定速率，但延迟不定
void process(KRingBuff<int> *k_buff) {
  int buff[10] = {0}, read_count = 5;
  // this_thread::sleep_for(chrono::microseconds(300));
  while (thread_running) {
    read_count = k_buff->ReadOut(buff, 5);
    if (read_count == 0) {
      // this_thread::sleep_for(chrono::microseconds(5));
      continue;
    }

    cout << "\nread_out:" << read_count << "\n\t";

    for (int i = 0; i < read_count; ++i) {
      cout << buff[i] << " ";
    }
    cout << endl;
    //		this_thread::sleep_for(chrono::microseconds(50));
    //		uniform_int_distribution<unsigned int> r_uni_dis(0, 30);
    //		chrono::milliseconds
    //			ms =
    //chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()); 		default_random_engine
    //r_engine(ms.count()); 		this_thread::sleep_for(chrono::milliseconds(r_uni_dis(r_engine) * 100));
  }
  cout << "process thread quit \n";
}

int main() {
  cout << "start"
       << "\n";

  cout << "init k_buff"
       << "\n";
  KRingBuff<int> k_buff(50);
  cout << "buff_len:" << k_buff.GetBuffLen() << "\n";

  cout << "init samples"
       << "\n";
  int samples[item_count];
  for (int i = 0; i < item_count; ++i) {
    samples[i] = i;
  }

  thread_running = true;
  thread consumer(process, &k_buff);

  int item_to_read = 0, read_count = 5, item_writen = 0;
  while (read_count) {
    item_writen = k_buff.WriteIn(&samples[item_to_read], read_count);

    // cout << "write_in:"<< item_writen << "\n\t" << samples[item_to_read] << " " << samples[item_to_read + 1] << " "
    // 	 << samples[item_to_read + 2] << " " << samples[item_to_read + 3] << " " << samples[item_to_read + 4]
    // 	 << "\nret:\n";
    // k_buff.dump();
    // cout << "\nend";

    item_to_read += read_count;
    if (item_to_read + read_count > item_count) read_count = item_count - item_to_read;
    this_thread::sleep_for(chrono::microseconds(1));
  }
  thread_running = false;
  consumer.join();
}