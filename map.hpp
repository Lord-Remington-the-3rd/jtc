#pragma once

#include <cstring>
#include <cassert>

typedef void (* FreeMem)(void *); 

u64 murmur64(const char * key);

template <typename T>
struct ClashList {
  u64 hash;
  T v;
  ClashList<T> *next;
};
  
template <typename T>
struct Map {
  ClashList<T> **buff = NULL;
  i64 cap = 0;
  Allocator list_alloc = NULL;
  void (*list_free)(void *) = NULL;
  
  void init(i64 size, Allocator list = malloc, FreeMem custom_free = free) {
    buff = (ClashList<T> **)calloc(size, sizeof(ClashList<T> *));
    assert(buff);

    list_alloc = list;
    list_free = custom_free;
    cap = size;
  }

  T *find(cstring key) {
    u64 hash = murmur64(key);
    u64 index = hash % cap;
   
    if (buff[index] == NULL)
      return NULL;

    ClashList<T> *cl = buff[index];
    do {
      if (cl->hash == hash) {
	return &cl->v;
      }
      cl = cl->next;
    } while (cl != NULL);

    return NULL;
  }

  void insert(cstring key, T v) {
    u64 hash = murmur64(key);
    u64 index = hash % cap;
    
    if (buff[index] == NULL) {
      buff[index] = (ClashList<T> *)list_alloc(sizeof(ClashList<T>));
      assert(buff[index]);

      buff[index]->next = NULL;
      buff[index]->v = v;
      buff[index]->hash = hash;

      return;
    }

    ClashList<T> *last = NULL;
    ClashList<T> *cl = buff[index];
    do {
      if (cl->hash == hash) {
	return;
      }
      last = cl;
      cl = cl->next;
    } while (cl != NULL);

    cl = (ClashList<T> *)list_alloc(sizeof(ClashList<T>));
    assert(cl);
    last->next = cl;

    cl->hash = hash;
    cl->v = v;
    cl->next = NULL;
  }

  bool remove(cstring key) {
    u64 hash = murmur64(key);
    u64 index = hash % cap;

    if (buff[index].hash == hash) {
      list_free(buff[index]);
      buff[index] = NULL;
      return true;
    }
    
    ClashList<T> *last = NULL;
    ClashList<T> *cl = buff[index];
    do {
      if (cl->hash == hash) {       
	last->next = cl->next;
	list_free(cl);
	return true;
      }
      last = cl;
      cl = cl->next;
    } while (cl != NULL);

    return false;
  }

  void destroy() {
    ClashList<T> *next;
    for (i64 i=0; i < cap; i++) {
      if (buff[i] == NULL) continue;
      
      next = buff[i]->next;
      list_free(buff[i]);

      while (next != NULL) {
	auto tmp = next->next;
	list_free(next);
	next = tmp;
      }
    }

    free(buff);
  }
};

u64 murmur64(const char *key) {
  u64 h(525201411107845655ull);
  for (;*key;++key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

void test_map() {
  Map<i64> m;
  m.init(500);

  m.insert("sdf", 32);
  m.insert("adf", 999);

  i64 *v = m.find("sdf");
  i64 *v2 = m.find("adf");
  
  assert( v != NULL && *v == 32);
  assert(v2 != NULL && *v2 == 999);
  
  m.destroy();  
}
