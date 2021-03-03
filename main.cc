#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <ctype.h>

#include "global_free.cc"
#include "parser.cc"

int main() {
  init_global_free();
  
  g_compiler_debug_mode = true;
  
  TokenizerState ts = tokenize("main.ts");

  Slice<Token> tokens = {};
  tokens.slice(ts.tokens);

  ParsingContext pc = {};
  pc.tokens = tokens;
  pc.ts = &ts;
  
  // _parse_function(&pc, NULL);
  parse_expr(&pc, NULL);
  free(ts.tokens.buff);
  global_free();
}

#include "native.cc"
