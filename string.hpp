#pragma once

#include <cstring>
#include <cassert>

struct String {
  cstring c_str = NULL;
  i64 len = 0;

  void alias(cstring str) {
    c_str = str;
    len = strlen(str);
  }

  void operator =(cstring str) {
    alias(str);
  }

  bool operator ==(String s) {
    return strcmp(c_str, s.c_str) == 0;
  }

  bool operator ==(cstring s) {
    return strcmp(c_str, s) == 0;
  }
};

struct StringBuilder {
  char *buff = NULL;
  i64 len = 0;
  i64 cap = 0;
  
  void init(i64 size, Allocator alloc = malloc) {
    buff = (char *)alloc(1 + size);
    assert(buff);
    
    len = 0;
    cap = size;
  }

  void write(String s) {
    assert(s.len + len <= cap);
    strcpy(buff + len, s.c_str);
    len += s.len;
  }

  void write(cstring s) {
    i64 s_len = strlen(s);
    assert(s_len + len <= cap);
    
    strcpy(buff + len, s);
    len += s_len;
  }

  String to_string() {
    return (String){buff, len};
  }
};

