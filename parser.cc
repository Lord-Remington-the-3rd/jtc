#include "tokenizer.cc"

struct Object;
struct FunctionCall;
struct BinaryOp;
struct Scope;
struct ParsingContext;
struct Cons;
struct Expression;
struct LetStatement;

void parse_function(ParsingContext *pc, Scope *scope);
void parse_codeblock(ParsingContext *pc, Scope *scope);
LetStatement parse_let_statement(ParsingContext *pc, Scope *scope);
Expression * parse_expr(ParsingContext *pc, Scope *scope);

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
  Map<LetStatement *> locals;
  Array<LetStatement> statements; // will be array of Statements, but simplifing for now
  Array<CodeBlock> sub_blocks;

  bool add_local(String name, LetStatement ls) {
    if (locals.find(name.c_str))
      return false;

    statements.push(ls);
    locals.insert(name.c_str, statements.buff + (statements.len - 1));
    
    return true;
  }
};

// remember that locals doesnt map to a js object
struct Function {
  String name; // name will be js_clojure or something for clojure types
  Scope *scope;
  Map<Expression> parameters; 
  CodeBlock body;
  Expression *returns; // might have to change to a pointer to a statement?
  // can be multiple returns in function tho? so this seems non sensical, will likely remove

  void add_param(String s, Expression v) {
    parameters.insert(s.c_str, v);
  }

  bool find_param(String s) {
    return parameters.find(s.c_str);
  }
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
  Token last;
  
  Token peek() {
    assert(tokens.len);
    return tokens[0];
  }

  Token eat() {
    assert(tokens.len);
    last = tokens.pop_front();
    return last;
  }

  void error(Token t, cstring msg) {
    printf("%s:%d:%d: %s \n", ts->src_name, t.row, t.col, msg);
    exit(1);
  }
};

void parse_function(ParsingContext *pc, Scope *scope) {
  Function fn = make_function(scope);
  Scope fn_scope = make_scope(fn);

  Token function_kwd = pc->peek();
  if (function_kwd.kind != TOKEN_FUNCTION) 
    pc->error(function_kwd, "compiler dev error, function keyword token not present");
   
  Token function_name = pc->peek();
  if (function_name.kind != TOKEN_IDENT && scope == NULL)
    pc->error(function_name, "can't declare clojures in global scope");
  else if (function_name.kind != TOKEN_IDENT)
    assert(true == false); // handle clojure

  fn.name = function_name.string();
  pc->eat(); // eat name

  Token opening_paran = pc->eat();
  if (opening_paran.kind != '(')
    pc->error(opening_paran, "expected '('"); 

  while (true) {
    Token peek = pc->peek();
    if (peek.kind == TOKEN_NIL)
      pc->error(peek, "reached end-of-file while parsing paramater list");
    else if (peek.kind == ')')
      break;

    Token param_name = pc->eat();
    if (param_name.kind != TOKEN_IDENT)
      pc->error(param_name, "expected identifier for argument");

    { // validate argument name, and insert if passes
      String param_name_string = param_name.string();
      if (param_name_string == fn.name)
	pc->error(param_name, "param cannot have the same name as function");
      else if (fn.find_param(param_name_string))
	pc->error(param_name, "duplicate parameter name"); // add map support for my strings

      fn.add_param(param_name_string, {JS_UNDEFINED});
      // add default values sometime
    }
    
    if (pc->peek().kind == ',') {
      pc->eat();
      Token maybe_rparan = pc->peek();
      if (maybe_rparan.kind == ')') 
	pc->error(maybe_rparan, "ended parameter list after comma, expected identifier");
    }
  }

  Token left_bracket = pc->peek();
  if (left_bracket.kind != '{') 
    pc->error(left_bracket, "expected '{' after parameter list");	      

  // parse codeblock
}

void parse_codeblock(ParsingContext *pc, Scope *scope) {
  CodeBlock block = make_codeblock(scope);
  Scope block_scope = make_scope(block);

  Token left_bracket = pc->eat();
  if (left_bracket.kind != '{')
    pc->error(left_bracket, "expecting '{' to begin codeblock");

  while (true) {
    Token peek = pc->peek();
    if (peek.kind == TOKEN_NIL)
      pc->error(left_bracket, "reached end-of-file while parsing codeblock, please match '{' with closing brace");
    else if (peek.kind == '}')
      break;

    // add other statements here and all later
    if (peek.kind != TOKEN_LET)
      pc->error(peek, "expected start of statement (i.e, let, for, while...)");

    LetStatement ls = parse_let_statement(pc, &block_scope);
    // check for semi colon here
      
  }
}

// only checks if function params are re declared in codeblock
// codeblocks check their local scope upon insertion of new let identifier
bool is_declared(String name, Scope *scope) {
  assert(scope); // get rid of this eventually cuz global scope has a null scope
  
  if (scope->kind == SCOPE_FUNCTION) {
    return scope->function.find_param(name);
  }
   
  return false;
}

LetStatement parse_let_statement(ParsingContext *pc, Scope *scope) {
  LetStatement ls = {};

  Token let = pc->eat();
  if (let.kind != TOKEN_LET)
    pc->error(let, "compiler dev error, let keyword not present");

  Token ident_name = pc->eat();
  if (ident_name.kind != TOKEN_IDENT)
    pc->error(ident_name, "expected ident after let keyword");

  ls.var_name = ident_name.string();
  
  Token equals_sign = pc->eat();
  if (equals_sign.kind != '=')
    pc->error(equals_sign, "expected '=' after ident");

  // also add expression later
  Expression *e = parse_expr(pc, scope);
  
  return ls;
}

Expression * parse_expr(ParsingContext *pc, Scope *scope) {
  Expression *ret = NULL;
  Expression *e = (Expression *)calloc(1, sizeof(Expression));

  Token t = pc->eat();
  if (t.kind == TOKEN_NUMBER) {
    char *c;
    cstring text = t.string().c_str;
    *e = {.kind=JS_NUMBER, .number=strtod(text, &c)};
  } else {
    pc->error(t, "invalid expression");
  }
  
  Token op = pc->peek();
  if (op.kind == ';') {
    return e;
  } else if (op.kind == '+') {
    pc->eat();
    BinaryOp *bop = (BinaryOp *)calloc(1, sizeof(BinaryOp));
    bop->name = op.kind;
    bop->left = e;
    bop->right = parse_expr(pc, scope);

   Expression *bop_expr = (Expression *)calloc(1, sizeof(Expression));
   *bop_expr = {.kind = JS_BINOP, .binop = bop};
   ret = bop_expr;
  } else {
    pc->error(op, "invalid expression");
  }

  return ret;
}
