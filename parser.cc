#include "tokenizer.cc"

struct Object;
struct FunctionCall;
struct BinaryOp;
struct Scope;
struct ParsingContext;
struct Cons;
struct Expression;


enum ExpressionKind {
  JS_UNDEFINED,		  
  JS_VAR_NAME,
  JS_NUMBER,
  JS_LIST,
  JS_OBJECT,
  JS_STRING,
  JS_FN_CALL,
  JS_BINOP
};

struct Expression { 
  int kind;
  union {
    String var_name; // key to map that contains value
    String string;
    f64 number;
    Object *object;
    FunctionCall *fn_call;
    BinaryOp *binop;
  };
};

enum ListKind {
  NIL,
  CONS
};

struct List { // list builder
  int kind;
  Cons *cons;
}; 

struct Cons {
  Expression e;
  List *l;
}; 

struct FunctionCall {
  String name;
  Array<Expression> args;
};

struct Object {
  Array<String> keys;
  Array<Expression> values;
};

struct BinaryOp {
  char name;
  Expression *left; // should only be valid for functional calls, variable names, and numbers
  Expression *right;
};

// assigns value 
struct LetStatement {
  String var_name; // dont know if this is necessary?
  Expression *value;
};

struct ReturnStatement {
  Expression value;
};

struct Statements {
  int kind;
  union {
    LetStatement let_statement;
    ReturnStatement return_statement;
  };
};

// probably get rid of this actually?
// just have stuff declared with let be removed when } is found
struct CodeBlock {
  Scope *scope;
  Map<Expression *> locals;
  Array<LetStatement> statements; // will be array of Statements, but simplifing for now
  Array<CodeBlock> sub_blocks;
};

// remember that locals doesnt map to a js object
struct Function {
  String name; // name will be js_clojure or something for clojure types
  Scope *scope;
  Map<Expression> parameters; 
  CodeBlock body;
  Expression *returns; // might have to change to a pointer to a statement?
  // can be multiple returns in function tho? so this seems non sensical, will likely remove
};

enum ScopeKind {
  SCOPE_FUNCTION,
  SCOPE_CODEBLOCK
};

// TODO(me) make Scope and Expression kinds
struct Scope {
  int kind;
  union {
    Function function;
    CodeBlock codeblock;
  }; 
};


CodeBlock make_codeblock(Scope *s) {
  CodeBlock c;
  c.scope = s;
  c.locals.init(50);
  c.sub_blocks.init(50);
  return c;
}

Function make_function(Scope *scope) {
  Function f = {};
  f.name = "__COMP_JS_CLOJURE";
  f.scope = scope; 
  f.parameters.init(400, malloc);
  pointer_holder.push(f.parameters.buff); // adds to global array to free at end of program
  f.body = {}; // will be dealt with later in parsing 
  f.returns = NULL; 

  return f;
}

Scope make_scope(CodeBlock cb) {
  return {
    .kind = SCOPE_CODEBLOCK,
    .codeblock = cb,	  
  };
}

Scope make_scope(Function f) {
  return {
    .kind = SCOPE_FUNCTION,
    .function = f,	  
  };
}

struct ParsingContext {
  TokenizerState *ts;
  Slice<Token> tokens;
  
};

// make parsing context?
CodeBlock parse_codeblock(TokenizerState *ts, Slice<Token> tokens, Scope *scope);
  
void parse_error(TokenizerState *ts, Token t, cstring msg) {
  printf("%s:%d:%d: %s \n", ts->src_name, t.row, t.col, msg);
  exit(1);
}

// remember to add scopes shit
// pass scope through here? 
void parse_function(TokenizerState *ts, Slice<Token> tokens, Scope *scope) {
  Function fn = make_function(scope);
  Scope fn_scope = make_scope(fn);
  
  Token function_name = tokens.pop_front();
  if (function_name.kind == TOKEN_IDENT)
  { 
    fn.name = function_name.string(); // alias it instead of allocating?
    puts(fn.name.c_str); // delete later

    Token opening_paran = tokens.pop_front();    
    if (opening_paran.kind == '(')
    {      
      while (tokens[0].kind != ')')
      {
	Token arg = tokens.pop_front();
	if (arg.kind == TOKEN_IDENT)
	{
	  // handle default values here eventually?
	  String arg_name = arg.string();
	  if (arg_name == fn.name)
	  {
	    parse_error(ts, arg, "parameter name cannot be the same as function name");
	  }
	  else if (fn.parameters.find(arg_name.c_str))
	  {
	    parse_error(ts, arg, "duplicate paramater in parameter list");
	  }

	  fn.parameters.insert(arg_name.c_str, {JS_UNDEFINED}); // maybe refactor into function, also maybe get rid of all uninit memory
	  puts(arg_name.c_str); // delete later
	  
	  Token next = tokens[0];
	  if (next.kind == ',')
	  {
	    tokens.pop_front();
	    if (tokens[0].kind == ')')
	    {
	      parse_error(ts, next, "expected closing parens not comma");
	    }
	  }
	}
	else
	{
	  parse_error(ts, arg, "expected identifier");
	}
      }
      tokens.pop_front(); // gets rid of ) 

      Token open_bracket = tokens[0];
      if (open_bracket.kind == '{') {
	fn.body = parse_codeblock(ts, tokens, &fn_scope);
	assert(false);
      } else {
	parse_error(ts, open_bracket, "expected { after parameter list");
      }
    }
    else
    {
      parse_error(ts, opening_paran, "expected opening parens");
    }
  }
  else
  {
    // handle name taken error (allow shadowing?)
    // also handle if it is not TOKEN_IDENT
    assert((false)); // lambdas not supported
  }
  
}

Expression * make_number_expr(TokenizerState *ts, Token t) {
  Expression *e = (Expression *)calloc(1, sizeof(Expression));
  char *c; // ignore this, makes function below work
  cstring token_text = t.string().c_str;
  *e = {
    .kind = JS_NUMBER,
    .number = strtod(token_text, &c)
  };

  return e;
}

Expression * parse_expr(TokenizerState *ts, Slice<Token> tokens, Scope *scope) {
  Expression *e = (Expression *)calloc(1, sizeof(Expression));

  Token t = tokens.pop_front(); puts(t.string().c_str); // delete later
  if (t.kind == TOKEN_STRING)
  {
    *e = {
      .kind = JS_STRING,
      .string = t.string(),	    
    };
  }
  else if (t.kind == TOKEN_NUMBER)
  {
    e = make_number_expr(ts, t);
  }
  else
  {
    puts("UNIMPLEMETNED!"); exit(0);
  }

  Token next = tokens.pop_front();
  if (next.kind == '+') // add some func to detect all this math crap
  {
    Token num = tokens.pop_front();

    BinaryOp *bop = (BinaryOp *)calloc(1, sizeof(BinaryOp));
    bop->name = next.kind;
    bop->left = e;
    // bop->right = parse_expr(ts, tokens, scope);

    if (num.kind == JS_NUMBER)
    {
      bop->right = make_number_expr(ts, num);
    }
    puts(num.string().c_str);
    
    // remember to like fix the op prec as we go up
    Expression *p = (Expression *)calloc(1, sizeof(Expression));
    *p = {.kind = JS_BINOP, .binop = bop};
    
    return p;
  }

  if (next.kind != ';')
  {
    parse_error(ts, next, "expected semicolon at the end of statement");
  }

  return e;
}

// make a parsing context cuz this shit is outta hand, slice is not getting updated
// idk if scope is even valid for this? scope should only be relevant to codeblocks and functions
// and this can only be in codeblocks anyway
LetStatement parse_let_statement(TokenizerState *ts, Slice<Token> tokens, Scope *scope) {
  tokens.pop_front();
  
  LetStatement ls = {};

  Token name = tokens.pop_front();
  if (name.kind == TOKEN_IDENT) {
    ls.var_name = name.string();
  } else {
    parse_error(ts, name, "expected identifer after 'let' ");
  }

  Token equals_sign = tokens.pop_front();  
  if (equals_sign.kind != '=') {
    parse_error(ts, equals_sign, "expected '=' after identifier");
  }

  // parse right side shit
  ls.value = parse_expr(ts, tokens, scope);
  
  return ls;
}

CodeBlock parse_codeblock(TokenizerState *ts, Slice<Token> tokens, Scope *scope) {
  tokens.pop_front();
  
  CodeBlock c = make_codeblock(scope); 
  // make scope for codeblocks and functions
  
  while (tokens[0].kind != '}')
  {
    Token t = tokens[0]; // any statement starter, like let, for 
    if (t.kind == TOKEN_LET) {
      LetStatement ls = parse_let_statement(ts, tokens, scope);
      Token semi = tokens.pop_front();
      if (semi.kind != ';') { // put this for all valid statements after if else block
	puts(semi.string().c_str); 
	parse_error(ts, semi, "expected semicolon at the end of statement");
      }
	   
    } else {
      parse_error(ts, t, "expected start of statement (i.e, let, for, while...)");
    }

  }

  tokens.pop_front(); // remove '}' (the end of the block)
  
  // if let keyword is hit then do whatever, u know the deal
  // add keyword detection to tokenizer 
  return c;
}
