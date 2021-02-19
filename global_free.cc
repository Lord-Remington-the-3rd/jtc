
Array<void *> pointer_holder = {};

void init_global_free() {
  pointer_holder.init(5000);
}

void global_free() {
  for (long i = 0; i < pointer_holder.len; i++) {
    free(pointer_holder[i]);
  }
}

void * global_calloc(u64 bytes, u64 size) {
  void *p = calloc(bytes, size);
  pointer_holder.push(p);
  return p;
}

void * global_alloc(u64 bytes) {
  void *p = malloc(bytes);
  pointer_holder.push(p);
  return p;
}

#define calloc global_calloc
#define malloc global_alloc
