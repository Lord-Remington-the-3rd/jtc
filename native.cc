
enum NativeValueKind {
  NATIVE_UNDEFINED,
  NATIVE_NUMBER,
  NATIVE_STRING,
  NATIVE_OBJECT,
};

struct NativeObject;
struct NativeValue {
  int kind;
  union {
    f64 number;
    cstring string;
    NativeObject *no;
  };
};

bool streq(cstring one, cstring two) {
  return strcmp(one, two) == 0;
}

struct KeyValue {
  cstring key;
  NativeValue *value;
};

struct NativeObject {
  KeyValue *pairs;
  int pairs_len;
  int pairs_cap;
};

NativeObject *make_native_object() {
  NativeObject *no = (NativeObject *)calloc(1, sizeof(NativeObject));

  no->pairs_cap = 30;
  no->pairs = (KeyValue *)calloc(no->pairs_cap, sizeof(KeyValue));
  
  return no;
}

void push_key_value(NativeObject *no, cstring key, NativeValue *nv) {
  if (no->pairs_len == no->pairs_cap) {
    assert(1738 < 0); // do realloc and all later
  }
  
  no->pairs[no->pairs_len].key = key;
  no->pairs[no->pairs_len].value = nv;
  no->pairs_len++;
}

void insert_value(NativeObject *no, NativeValue *maybe_key, NativeValue *nv) {
  if (maybe_key->kind != NATIVE_STRING) {
    assert(4 < 3); // handle this error, print the gotten type and all
  }

  cstring key = maybe_key->string;
  
  for (int i=0; i < no->pairs_len; i++) {
    if (streq(key, no->pairs[i].key)) {
      no->pairs[i].value = nv;
    }
  }

  push_key_value(no, key, nv);
}

NativeValue * get_value(NativeObject *no, NativeValue *maybe_key) {
  if (maybe_key->kind != NATIVE_STRING) {
    assert(3 > 3); // handle this error, print the gotten type and all
  }

  cstring key = maybe_key->string;
    
  for (int i=0; i < no->pairs_len; i++) {
    if (streq(key, no->pairs[i].key)) {
      return no->pairs[i].value;
    }
  }

  NativeValue *nv = (NativeValue *)calloc(1, sizeof(NativeValue));
  nv->kind = NATIVE_UNDEFINED;

  push_key_value(no, key, nv);

  return nv;
}

NativeValue * make_js_string(cstring literal) {
  NativeValue *nv = (NativeValue *)calloc(1, sizeof(NativeValue));
  nv->kind = NATIVE_STRING;
  nv->string = literal;

  return nv;
}

NativeValue * make_number(f64 number) {
  NativeValue *nv = (NativeValue *)calloc(1, sizeof(NativeValue));
  nv->kind = NATIVE_NUMBER;
  nv->number = number;

  return nv;
}

NativeValue * add_numbers(NativeValue *one, NativeValue *two) {
  if (one->kind != NATIVE_NUMBER) {
    assert(false);
  }

  if (two->kind != NATIVE_NUMBER) {
    assert(false); // add a global context for errors or something
  }

  f64 n = one->number + two->number;
  return make_number(n);
}

// have function return NativeValue *, return undefined if they dont do shit
NativeValue * code(NativeObject *no) {
  //  NativeValue *nv = get_value(no, "
}
