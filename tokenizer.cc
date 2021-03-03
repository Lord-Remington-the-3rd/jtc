
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
  KEYWORDS_LEN, // used for looping
  
  // other ident types 
  TOKEN_NIL,	    
  TOKEN_IDENT,		
  TOKEN_NUMBER,
  TOKEN_STRING,
};

struct Token;
struct TokenizerState;

struct Token {
  int kind; // can also be char like '}'
  i32 row, col;
  cstring index;
  i32 len;

  String string() {
    char *s = (char *)calloc(1000, 1);

    for (int i=0; i < len; i++) {
      s[i] = index[i];
    }

    String ret;
    ret = s;
    return ret;
  }

  void match_keyword() {
    String token_text = string();
    for (int i=0; i < KEYWORDS_LEN; i++) {
      if (token_text == keywords[i]) {
	this->kind = i;
      }
    }
  }
};

// (3 + (3 + (3)))
struct TokenizerState {
  cstring src_name;
  cstring src = "3 + 3 + 3; main(man, ass, an) { let x = 3 + 8; } \n";
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

void print_token(Token t) {
  printf("k:  %4d r:  %4d  c: %4d  i: %c  l: %2d -- ", t.kind, t.row, t.col, *t.index, t.len);

  for (int i=0; i < t.len; i++) {
    char c = t.index[i];
    printf("%c", c);
  }
  
  printf("\n");
}


TokenizerState tokenize(cstring filename) {
  TokenizerState ts = {};
  ts.init(filename);

  while (ts.peek() != 0) {
    char c = ts.peek();
    Token tok = {TOKEN_NIL, ts.row, ts.col, ts.src + ts.index, 0};
    
    if (isalpha(c) || c == '_') {
      tok.kind = TOKEN_IDENT;
      while (isalpha(c) || c == '_' || isdigit(c)) {
	c = ts.eat_and_peek();
	tok.len++;
      }
      tok.match_keyword(); 

    } else if (isdigit(c)) {
      tok.kind = TOKEN_NUMBER;
      while (isdigit(c)) {
	c = ts.eat_and_peek();
	tok.len++;
      }
    } else if (c == '"') {
      tok.kind = TOKEN_STRING;
      c = ts.eat_and_peek(); // move past "
      tok.index = ts.src + ts.index; // ignores the starting " of string
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
      print_token(ts.tokens[i]);
    }
  }

  ts.tokens.push({TOKEN_NIL});
  
  return ts;
}
