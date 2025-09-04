#include "murmurhash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HASHMAP_H
#define HASHMAP_H

uint32_t hash(const char *key) {
  const uint32_t seed = 0xAFAFAF;
  return murmurhash(key, (uint32_t)strlen(key), seed);
}

#define DEFINE_HASHMAP(Name, name, type, free_func)                            \
  typedef struct {                                                             \
    const char *key;                                                           \
    type value;                                                                \
  } Bucket##Name;                                                              \
                                                                               \
  struct HashMap##Name {                                                       \
    size_t cap;                                                                \
    size_t size;                                                               \
    Bucket##Name **values;                                                     \
  };                                                                           \
                                                                               \
  struct HashMap##Name *hashmap_##name##_new() {                               \
    struct HashMap##Name *hashmap = malloc(sizeof(struct HashMap##Name));      \
    if (hashmap == NULL) {                                                     \
      return NULL;                                                             \
    }                                                                          \
                                                                               \
    hashmap->cap = 64;                                                         \
    hashmap->size = 0;                                                         \
    hashmap->values = calloc(hashmap->cap, sizeof(Bucket##Name *));            \
    if (hashmap->values == NULL) {                                             \
      free(hashmap);                                                           \
      return NULL;                                                             \
    }                                                                          \
                                                                               \
    return hashmap;                                                            \
  }                                                                            \
                                                                               \
  void hashmap_##name##_free(struct HashMap##Name *hashmap) {                  \
    if (hashmap == NULL) {                                                     \
      return;                                                                  \
    }                                                                          \
                                                                               \
    void (*free_fn)(type) = free_func;                                         \
    for (size_t i = 0; i < hashmap->cap; i++) {                                \
      free((void *)hashmap->values[i]->key);                                   \
      if (free_fn) {                                                           \
        free_fn(hashmap->values[i]->value);                                    \
      }                                                                        \
      free(hashmap->values[i]);                                                \
    }                                                                          \
                                                                               \
    free(hashmap->values);                                                     \
    free(hashmap);                                                             \
  }                                                                            \
                                                                               \
  void hashmap_##name##_set(struct HashMap##Name *hashmap, const char *key,    \
                            type value) {                                      \
    if (hashmap->size >= hashmap->cap * 0.75) {                                \
      size_t new_cap = hashmap->cap * 2;                                       \
      Bucket##Name **new_values = calloc(new_cap, sizeof(Bucket##Name *));     \
      if (new_values == NULL) {                                                \
        return;                                                                \
      }                                                                        \
                                                                               \
      for (size_t i = 0; i < hashmap->cap; i++) {                              \
        Bucket##Name *b = hashmap->values[i];                                  \
                                                                               \
        if (b == NULL) {                                                       \
          continue;                                                            \
        }                                                                      \
                                                                               \
        size_t index = hash(b->key) % new_cap;                                 \
        for (size_t j = 0; j < new_cap; j++) {                                 \
          size_t new_index = (index + j * j) % new_cap;                        \
          if (new_values[new_index] == NULL) {                                 \
            new_values[new_index] = b;                                         \
            break;                                                             \
          }                                                                    \
        }                                                                      \
      }                                                                        \
                                                                               \
      free(hashmap->values);                                                   \
      hashmap->values = new_values;                                            \
      hashmap->cap = new_cap;                                                  \
    }                                                                          \
                                                                               \
    size_t index = hash(key) % hashmap->cap;                                   \
    Bucket##Name *bucket = malloc(sizeof(Bucket##Name));                       \
    if (bucket == NULL) {                                                      \
      return;                                                                  \
    }                                                                          \
                                                                               \
    bucket->key = strdup(key);                                                 \
    bucket->value = value;                                                     \
                                                                               \
    for (size_t i = 0; i < hashmap->cap; i++) {                                \
      size_t new_index = (index + i * i) % hashmap->cap;                       \
      if (hashmap->values[new_index] == NULL) {                                \
        hashmap->values[new_index] = bucket;                                   \
        hashmap->size++;                                                       \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  type hashmap_##name##_get(struct HashMap##Name *hashmap, const char *key) {  \
    size_t index = hash(key) % hashmap->cap;                                   \
                                                                               \
    for (size_t i = 0; i < hashmap->cap; i++) {                                \
      size_t new_index = (index + i * i) % hashmap->cap;                       \
      Bucket##Name *b = hashmap->values[new_index];                            \
                                                                               \
      if (b == NULL) {                                                         \
        continue;                                                              \
      }                                                                        \
                                                                               \
      if (strcmp(b->key, key) == 0) {                                          \
        return b->value;                                                       \
      }                                                                        \
    }                                                                          \
                                                                               \
    return *(type *)NULL;                                                      \
  }                                                                            \
                                                                               \
  void hashmap_##name##_print(struct HashMap##Name *hashmap,                   \
                              void (*print)(type, int ident), int ident) {     \
    struct HashMap##Name map = *hashmap;                                       \
                                                                               \
    printf("{\n");                                                             \
    for (size_t i = 0, j = 0; i < map.cap; i++) {                              \
      Bucket##Name *b = map.values[i];                                         \
      if (!b) {                                                                \
        continue;                                                              \
      }                                                                        \
      printf("%*s\"%s\": ", ident * 2, "", b->key);                            \
      print(b->value, ident);                                                  \
      if (j++ < map.size - 1) {                                                \
        printf(",");                                                           \
      }                                                                        \
      printf("\n");                                                            \
    }                                                                          \
    printf("%*s}", (ident - 1) * 2, "");                                       \
  }

#endif
