/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef ak_json_h
#define ak_json_h

#include <string.h>
#include <stdlib.h>

/* JSON parser */
#include <json/json.h>

AK_INLINE
char *
json_strdup(const json_t * __restrict jsonObject,
            AkHeap       * __restrict heap,
            void         * __restrict parent) {
  return ak_heap_strndup(heap,
                         parent,
                         json_string(jsonObject),
                         jsonObject->valSize);
}

AK_INLINE
void
json_array_set(void         * __restrict p,
               AkTypeId                  typeId,
               int                       index,
               const json_t * __restrict json) {
  switch (typeId) {
    case AKT_FLOAT:
      ((float *)p)[index] = json_float(json, 0.0f);
      break;
    case AKT_INT:
      ((int32_t *)p)[index] = json_int32(json, 0);
      break;
    case AKT_UINT:
      ((int32_t *)p)[index] = json_uint32(json, 0);
      break;
    case AKT_SHORT:
      ((int16_t *)p)[index] = json_int32(json, 0);
      break;
    case AKT_USHORT:
      ((uint16_t *)p)[index] = json_uint32(json, 0);
      break;
    case AKT_BYTE:
      ((char *)p)[index] = json_int32(json, 0);
      break;
    case AKT_UBYTE:
      ((unsigned char *)p)[index] = json_uint32(json, 0);
      break;
    default:
      break;
  }
}

const char*
json_cstr(json_t *jsn, const char *key);

int32_t
jsn_i32_def(json_t     *jsn,
               const char *key,
               int32_t     def);

int32_t
jsn_i32(json_t *jsn, const char *key);

int64_t
jsn_i64(json_t *jsn, const char *key);

float
jsn_flt(json_t *jsn, const char *key);

float
jsn_flt_at(json_t *jsn, int32_t index);

void
jsn_flt_if(json_t *jsn, const char *key, float *dest);

#endif /* ak_json_h */
