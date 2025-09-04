#include "json.h"
#include "hashmap.h"
#include "vector.h"

static const char WS[] = " \n\r\t";
#define WS_SIZE sizeof(char) * 5;

void json_print_r(JSON json, int ident) {
  switch (json.type) {
  case OBJECT:
    hashmap_json_print(json.map, json_print_r, ident + 1);
    break;
  case ARRAY:
    vector_json_print(json.vec, json_print_r, ident + 1);
    break;
  case STRING:
    printf("\"%s\"", json.str);
    break;
  case NUMBER:
    printf("%f", json.d);
    break;
  case BOOLEAN:
    printf("%s", json.b ? "true" : "false");
    break;
  }
}

void json_print(JSON json) {
  json_print_r(json, 0);
  printf("\n");
}

static inline char next(Parser *p) { return p->source[p->offset++]; }

static inline char match(Parser *p, const char *accept, bool skip_ws) {
  if (skip_ws) {
    p->offset += strspn(p->source + p->offset, WS);
  }

  if (!strchr(accept, p->source[p->offset])) {
    return -1;
  }

  return p->source[p->offset];
}

Token scan_token(Parser *p) {
  char c = next(p);

  switch (c) {
  case ' ':
  case '\n':
  case '\r':
  case '\t':
    return (Token){.type = TOK_WHITESPACE};
  case '{':
    return (Token){.type = TOK_BRACE_LEFT};
  case '}':
    return (Token){.type = TOK_BRACE_RIGHT};
  case '[':
    return (Token){.type = TOK_BRACKET_LEFT};
  case ']':
    return (Token){.type = TOK_BRACKET_RIGHT};
  case ',':
    return (Token){.type = TOK_COMMA};
  case ':':
    return (Token){.type = TOK_COLON};
  case '"': {
    const char *start = p->source + p->offset;
    const char *end = strchr(start, '"');
    size_t len = end - start;

    char *str = malloc(sizeof(char) * len);
    memcpy(str, start, len);
    str[len] = '\0';

    // set offset to the character after the last quote
    p->offset += len + 1;

    return (Token){.type = TOK_STRING, .str = str};
  }
  default: {
    const char *start = p->source + p->offset - 1;
    char *endptr;
    double value = strtod(start, &endptr);

    if (endptr != start) {
      p->offset += endptr - (start + 1);
      return (Token){.type = TOK_NUMBER, .d = value};
    }

    break;
  }
  }

  return (Token){.type = TOK_NONE};
}

void json_merge_value(Parser *parser, JSON value) {
  Parser p = *parser;

  if (p.vec_ctx->len == 0) {
    parser->result = value;
    return;
  }

  JSON current = vector_json_at(p.vec_ctx, -1);
  switch (current.type) {
  case ARRAY:
    vector_json_push(current.vec, value);
    break;
  case OBJECT: {
    const char **key = vector_str_pop(p.keys);
    if (!key) {
      return;
    }
    hashmap_json_set(current.map, *key, value);
    break;
  }
  default:
    printf("[ERROR]: Unknown ctx type\n");
    break;
  }
}

JSON json_parse(const char *source) {
  Parser p = {
      .source = source,
      .offset = 0,
      .keys = vector_str_new(),
      .vec_ctx = vector_json_new(),
      .result = {.ok = true},
  };

  Token t;
  while ((t = scan_token(&p)), t.type != TOK_NONE) {
    switch (t.type) {
    case TOK_WHITESPACE:
    case TOK_COLON:
    case TOK_COMMA:
      continue;
    case TOK_BRACE_LEFT:
      vector_json_push(p.vec_ctx,
                       (JSON){.type = OBJECT, .map = hashmap_json_new()});
      break;
    case TOK_BRACE_RIGHT: {
      char next = match(&p, ",}]", true);

      if (next == -1) {
        printf("[ERROR]: Unterminated JSON objectx\n");
        p.result.ok = false;
        break;
      }

      json_merge_value(&p, *vector_json_pop(p.vec_ctx));

      break;
    }
    case TOK_BRACKET_LEFT:
      vector_json_push(p.vec_ctx,
                       (JSON){.type = ARRAY, .vec = vector_json_new()});
      break;
    case TOK_BRACKET_RIGHT:
      json_merge_value(&p, *vector_json_pop(p.vec_ctx));
      break;
    case TOK_STRING: {
      char next = match(&p, ":,}]", true);

      if (next == -1) {
        printf("[ERROR]: Unterminated JSON object\n");
        p.result.ok = false;
        break;
      }

      if (next == ':') {
        vector_str_push(p.keys, t.str);
      } else {
        json_merge_value(&p, (JSON){.type = STRING, .str = t.str});
      }

      break;
    }
    case TOK_NUMBER: {
      char next = match(&p, ",}]", true);

      if (next == -1) {
        printf("[ERROR]: Unterminated JSON object\n");
        p.result.ok = false;
        break;
      }

      json_merge_value(&p, (JSON){.type = NUMBER, .d = t.d});
      break;
    }
    case TOK_BOOLEAN:
      break;
    case TOK_NONE:
      p.result.ok = false;
      break;
    }
  }

  return p.result;
}

int main() {
  const char *json_str = "[{ \n"
                         "\"name\":\"John Doe\",\t\t\t"
                         "\"meta\":{\"tag\":589},"
                         "\"itemIds\": \t[10,20,30,40,50, \"literal\", [  { "
                         "\"nested\" :  {\"itemId\" : 0x68 }}  ]]"
                         " } , {   \t \"id\"   :           \n\t 12345}]";

  JSON json = json_parse(json_str);
  json_print(json);
}
