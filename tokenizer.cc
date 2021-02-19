
bool g_compiler_debug_mode = false;

const cstring keywords[] = {
  "for",
  "if",
  "else",
  "let",
  "const",
  "function",
};

enum TokenKind {
  // Keywords listed first to index into array
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_LET,
  TOKEN_CONST,
  TOKEN_FUNCTION,

  // other ident types 
  TOKEN_NIL,	    
  TOKEN_IDENT,		
  TOKEN_NUMBER,
  TOKEN_STRING,
};

struct Token;
struct TokenizerState;
cstring token_to_string(Token *t, TokenizerState *ts);

struct Token {
  int kind; // can also be char like 'c'
  i32 row, col;
  i32 index, len;

  String string(TokenizerState *ts) {
    String s = {};
    s = token_to_string(this, ts); // change into string alias instead of allocing
    return s;
  }
};

struct TokenizerState {
  cstring src_name;
  cstring src = "main(man, ass, an) { \n" " str s = \"sdfdsf\"; \n";
  i32 row, col;
  i32 index;
  Array<Token> tokens;

  
  void init(cstring filename) {
    src_name = filename;
    row = 1;
    col = 1;
    index = 0;
    tokens.init(5000);
  }
  
  char peek() {
    return src[index];
  }

  void eat() {
    if (src[index] == '\n') {
      row++;
      col = 1;
    } else {
      col++;
    }
    
    index++;
  }

  char eat_and_peek() {
    eat();
    return peek();
  }
  
  void error(Token t, cstring msg) {
    printf("%s:%d:%d: %s \n", src_name, t.row, t.col, msg);
    exit(-1);
  }
};

cstring token_to_string(Token *t, TokenizerState *ts) {
  char *s = (char *)calloc(1000, 1);

  for (int i=0; i < t->len; i++) {
    s[i] = ts->src[t->index + i];
  }

  return s;
}

void print_token(TokenizerState *ts, Token t) {
  printf("k:  %4d r:  %4d  c: %4d  i: %4d  l: %2d -- ", t.kind, t.row, t.col, t.index, t.len);

  for (int i=0; i < t.len; i++) {
    char c = ts->src[t.index + i];
    printf("%c", c);
  }
  
  printf("\n");
}


TokenizerState tokenize(cstring filename) {
  TokenizerState ts = {};
  ts.init(filename);

  while (ts.peek() != 0) {
    char c = ts.peek();
    Token tok = {TOKEN_NIL, ts.row, ts.col, ts.index, 0};
    
    if (isalpha(c) || c == '_') {
      tok.kind = TOKEN_IDENT;
      while (isalpha(c) || c == '_' || isdigit(c)) {
	c = ts.eat_and_peek();
	tok.len++;
      }
     
    } else if (isdigit(c)) {
      tok.kind = TOKEN_NUMBER;
      while (isdigit(c)) {
	c = ts.eat_and_peek();
	tok.len++;
      }
    } else if (c == '"') {
      tok.kind = TOKEN_STRING;
      c = ts.eat_and_peek(); // move past "
      tok.index = ts.index; // ignores the starting " of string
      while (c != '"' && c != 0) {
	c = ts.eat_and_peek();
	tok.len++;
      }

      if (c == 0) {
	ts.error(tok, "reached EOF while parsing string");
      }

      ts.eat();
      
    } else if (!isspace(c)) {
      tok.kind = c;
      tok.len = 1;
      ts.eat();
      
    } else {
      ts.eat();
    }
    
    if (tok.kind != TOKEN_NIL) {	    
      ts.tokens.push(tok);
    }
  }

  printf("Test Code: \n%s\n", ts.src);
  
  if (g_compiler_debug_mode) {
    for (int i=0; i < ts.tokens.len; i++) {
      print_token(&ts, ts.tokens[i]);
    }
  }

  ts.tokens.push({TOKEN_NIL});
  
  return ts;
}
