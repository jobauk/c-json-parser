#include "hashmap.h"
#include "vector.h"
#include <stdbool.h>

typedef struct {
  bool ok;
  enum JSONType { OBJECT, ARRAY, STRING, NUMBER, BOOLEAN } type;
  union {
    bool b;
    double d;
    const char *str;
    struct HashMapJSON *map;
    struct VectorJSON *vec;
  };
} JSON;

DEFINE_HASHMAP(JSON, json, JSON, NULL)
DEFINE_VECTOR(JSON, json, JSON, NULL)
DEFINE_VECTOR(JSONPTR, jsonp, JSON *, NULL)
DEFINE_VECTOR(String, str, const char *, NULL)

typedef enum {
  TOK_WHITESPACE,
  TOK_BRACE_LEFT,
  TOK_BRACE_RIGHT,
  TOK_BRACKET_LEFT,
  TOK_BRACKET_RIGHT,
  TOK_COLON,
  TOK_COMMA,

  TOK_STRING,
  TOK_NUMBER,
  TOK_BOOLEAN,

  TOK_NONE
} TokenType;

typedef struct {
  TokenType type;
  union {
    bool b;
    double d;
    const char *str;
  };
} Token;

typedef struct {
  size_t offset;
  struct VectorString *keys;
  struct VectorJSON *vec_ctx;
  const char *source;
  JSON result;
} Parser;
