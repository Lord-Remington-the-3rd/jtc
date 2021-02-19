#include "tokenizer.cc"

enum JSValueKind {
  JS_UNDEFINED,		  
  JS_VAR_NAME,
  JS_NUMBER,
  JS_LIST,
  JS_OBJECT,
  JS_FN_CALL,
};

// likely should not exist? have shit be a string and just have js runtime serialize and shit
struct JSValue { 
  int kind;
  union {
    cstring var_name;
    f64 number;
    Array<JSValue> list;
    struct {
      Map<JSValue> map;
    } object;
    struct {
      String name;
      Array<String> args;
    } fn_call;
  };
  }; 

struct Scope;

struct CodeBlock {
  Scope *scope;
  Map<JSValue> locals;
  Array<CodeBlock> sub_blocks;
};

struct Function {
  String name; // name will be js_clojure or something for clojure types
  Scope *scope;
  Map<JSValue> parameters;
  CodeBlock body;
  String returns; // change into tagged union?
};

enum ScopeKind {
  SCOPE_FUNCTION,
  SCOPE_CODEBLOCK
};

// TODO(me) make Scope and JSValue kinds
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
  f.returns = "__COMP_RETURNS_NOTHING"; // maybe jsvalue

  return f;
}

// make parsing context?

void parse_error(cstring filename, Token t, cstring msg) {
  printf("%s:%d:%d: %s \n", filename, t.row, t.col, msg);
  exit(1);
}

// remember to add scopes shit
// pass scope through here? 
void parse_function(TokenizerState *ts, Slice<Token> tokens, Scope *scope) {
  Function fn = make_function(scope);
  cstring filename = ts->src_name;
    
  Token function_name = tokens.pop_front();
  if (function_name.kind == TOKEN_IDENT)
  { 
    fn.name = function_name.string(ts); // alias it instead of allocating?
    puts(fn.name.c_str);

    Token opening_paran = tokens.pop_front();    
    if (opening_paran.kind == '(')
    {      
      while (tokens[0].kind != ')')
      {
	Token arg = tokens.pop_front();
	if (arg.kind == TOKEN_IDENT)
	{
	  // handle default values here eventually?
	  String arg_name = arg.string(ts);
	  if (arg_name == fn.name)
	  {
	    parse_error(filename, arg, "parameter name cannot be the same as function name");
	  }
	  else if (fn.parameters.find(arg_name.c_str))
	  {
	    parse_error(filename, arg, "duplicate paramater in parameter list");
	  }

	  fn.parameters.insert(arg_name.c_str, {JS_UNDEFINED}); // maybe refactor into function, also maybe get rid of all uninit memory
	  puts(arg_name.c_str);
	  
	  Token next = tokens[0];
	  if (next.kind == ',')
	  {
	    tokens.pop_front();
	    if (tokens[0].kind == ')')
	    {
	      parse_error(filename, next, "expected closing parens not comma");
	    }
	  }

	  // parse_codeblock and remember to pass in scope bru
	}
	else
	{
	  parse_error(filename, arg, "expected identifier");
	}
      }
    }
    else
    {
      parse_error(filename, opening_paran, "expected opening parens");
    }
  }
  else
  {
    // handle name taken error (allow shadowing?)
    // also handle if it is not TOKEN_IDENT
    assert((false)); // lambdas not supported
  }
  
}

CodeBlock parse_codeblock(Scope *scope) {
  CodeBlock c = make_codeblock(scope);
  

  return c;
}
