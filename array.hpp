#pragma once

#include <cstring>
#include <cassert>


template <typename T>
struct Array {
  T *buff = NULL;
  i64 len = 0;
  i64 cap = 0;

  void init(i64 p_cap, Allocator alloc = malloc) {
    buff = (T *)alloc(p_cap * sizeof(T));
    assert(buff);

    len = 0;
    cap = p_cap;
  }

  void push(T t) {
    assert(len < cap);

    buff[len] = t;
    len++;
  }

  T pop() {
    assert(len > 0);
    return buff[len--];
  }

  T operator[] (i64 i) {
    assert(i >= 0);
    assert(i < len);

    return buff[i];
  }
};

template <typename T>
struct Slice {
  T *buff;
  i64 len;

  void slice(Array<T> arr) {
    buff = arr.buff;
    len = arr.len;
  }

  T slice(Array<T> arr, i64 start, i64 end) {
    assert(start > 0);
    assert(end <= arr.len);

    return (Slice){arr.buff + start, end - 1};
  }

  T pop_front() {
    assert(len > 0);

    T ret = buff[0];
    buff++;
    len--;

    return ret;
  }

  void pop_back() {
    assert(len > 0);
    
    return buff[--len];
  }

  T operator[] (i64 i) {
    assert(i >= 0);
    assert(i < len);

    return buff[i];
  }
};

