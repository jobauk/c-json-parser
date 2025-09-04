#include <stdio.h>
#include <stdlib.h>

#ifndef VECTOR_H
#define VECTOR_H

#define DEFINE_VECTOR(Name, name, type, free_func)                             \
  struct Vector##Name {                                                        \
    size_t len;                                                                \
    size_t cap;                                                                \
    type *items;                                                               \
  };                                                                           \
                                                                               \
  struct Vector##Name *vector_##name##_new() {                                 \
    struct Vector##Name *v = malloc(sizeof(struct Vector##Name));              \
    if (!v) {                                                                  \
      return NULL;                                                             \
    }                                                                          \
                                                                               \
    v->len = 0;                                                                \
    v->cap = 32;                                                               \
    v->items = calloc(v->cap, sizeof(type));                                   \
    if (!v->items) {                                                           \
      free(v);                                                                 \
      return NULL;                                                             \
    }                                                                          \
                                                                               \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  static inline void vector_##name##_free(struct Vector##Name *v) {            \
    if (v) {                                                                   \
      void (*free_fn)(type) = free_func;                                       \
      if (free_fn) {                                                           \
        for (size_t i = 0; i < v->len; i++) {                                  \
          free_fn(v->items[i]);                                                \
        }                                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    free(v->items);                                                            \
    free(v);                                                                   \
  }                                                                            \
                                                                               \
  static inline void vector_##name##_push(struct Vector##Name *v,              \
                                          type value) {                        \
    if (v->len >= v->cap) {                                                    \
      v->cap *= 2;                                                             \
      type *new_items = realloc(v->items, sizeof(type) * v->cap);              \
      if (!new_items) {                                                        \
        return;                                                                \
      }                                                                        \
      v->items = new_items;                                                    \
    }                                                                          \
                                                                               \
    v->items[v->len++] = value;                                                \
  }                                                                            \
                                                                               \
  static inline type *vector_##name##_pop(struct Vector##Name *v) {            \
    if (v->len == 0) {                                                         \
      return NULL;                                                             \
    }                                                                          \
                                                                               \
    return &v->items[--v->len];                                                \
  }                                                                            \
                                                                               \
  static inline type vector_##name##_at(struct Vector##Name *v,                \
                                        size_t index) {                        \
    if (index > v->len - 1) {                                                  \
      return v->items[v->len - 1];                                             \
    }                                                                          \
                                                                               \
    if (index >= 0) {                                                          \
      return v->items[index];                                                  \
    }                                                                          \
                                                                               \
    size_t _index = v->len - index - 1;                                        \
    return v->items[_index < 0 ? 0 : _index];                                  \
  }                                                                            \
                                                                               \
  static inline void vector_##name##_print(                                    \
      struct Vector##Name *v, void (*print)(type, int ident), int ident) {     \
    printf("[");                                                               \
    for (size_t i = 0; i < v->len; i++) {                                      \
      if (i > 0) {                                                             \
        printf(", ");                                                          \
      }                                                                        \
      print(v->items[i], ident);                                               \
    }                                                                          \
    printf("]");                                                               \
  }

#endif
