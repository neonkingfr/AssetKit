/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "gltf_mesh.h"
#include "gltf_enums.h"
#include "gltf_accesor.h"
#include "gltf_buffer.h"
#include "../../ak_accessor.h"

/*
  glTF meshes      -> AkGeometry > AkMesh
  glTF primitives  -> AkMeshPrimitive
 */

void _assetkit_hide
gltf_meshes(AkGLTFState * __restrict gst) {
  AkHeap     *heap;
  AkDoc      *doc;
  json_t     *jmeshes, *jaccessors;
  AkLibItem  *lib;
  AkGeometry *last_geom;
  int32_t     jmeshCount, i;

  heap        = gst->heap;
  doc         = gst->doc;
  lib         = ak_heap_calloc(heap, doc, sizeof(*lib));
  last_geom   = NULL;

  jmeshes    = json_object_get(gst->root, _s_gltf_meshes);
  jmeshCount = (int32_t)json_array_size(jmeshes);
  jaccessors = json_object_get(gst->root, _s_gltf_accessors);

  for (i = jmeshCount - 1; i >= 0; i--) {
    AkGeometry *geom;
    AkObject   *last_mesh;

    json_t     *jmesh, *jprims;
    int32_t     jprimCount, j;

    geom       = ak_heap_calloc(heap, lib, sizeof(*geom));

    jmesh      = json_array_get(jmeshes, i);
    jprims     = json_object_get(jmesh, _s_gltf_primitives);
    jprimCount = (int32_t)json_array_size(jprims);
    last_mesh  = NULL;

    for (j = 0; j < jprimCount; j++) {
      AkObject        *meshObj;
      AkMesh          *mesh;
      AkMeshPrimitive *prim;
      AkInput         *last_inp, *last_vert_inp;
      json_t          *jprim, *jattribs, *jval, *jindices;
      const char      *jkey;

      meshObj = ak_objAlloc(heap,
                            geom,
                            sizeof(AkMesh),
                            AK_GEOMETRY_TYPE_MESH,
                            true);
      mesh       = ak_objGet(meshObj);
      mesh->geom = geom;

      prim  = ak_heap_calloc(heap, meshObj, sizeof(*prim));
      jprim = json_array_get(jprims, i);

      prim->vertices = ak_heap_calloc(heap,
                                      prim,
                                      sizeof(*prim->vertices));

      last_inp = last_vert_inp = NULL;
      jattribs = json_object_get(jprim, _s_gltf_attributes);
      json_object_foreach(jattribs, jkey, jval) {
        AkInput    *inp;
        const char *semantic;
        AkSource   *source;
        AkAccessor *acc;
        json_t     *jacc;

        inp      = ak_heap_calloc(heap, prim, sizeof(*inp));
        semantic = strchr(jkey, '_');

        if (!semantic) {
          inp->base.semanticRaw = ak_heap_strdup(heap,
                                                 inp,
                                                 jkey);
        }

        /* ARRAYs e.g. TEXTURE_0, TEXTURE_1 */
        else {
          inp->base.semanticRaw = ak_heap_strndup(heap,
                                                  inp,
                                                  jkey,
                                                  semantic - jkey);
        }

        source = ak_heap_calloc(heap, prim, sizeof(*source));
        acc    = ak_heap_calloc(heap, source, sizeof(*acc));
        jacc   = json_array_get(jaccessors, json_integer_value(jval));

        inp->base.semantic   = gltf_enumInputSemantic(inp->base.semanticRaw);
        inp->base.source.ptr = gltf_accessor(gst, prim, jacc);

        /* TODO: check morph targets to insert them to <vertices> */
        if (inp->base.semantic == AK_INPUT_SEMANTIC_POSITION) {
          if (last_vert_inp)
            last_vert_inp->base.next = &inp->base;
          else
            prim->vertices->input = &inp->base;

          last_vert_inp = inp;
          prim->vertices->inputCount++;
        } else {
          if (last_inp)
            last_inp->base.next = &inp->base;
          else
            prim->input = inp;

          last_inp = inp;
          prim->inputCount++;
        }
      }

      if ((jindices = json_object_get(jprim, _s_gltf_indices))) {
        AkAccessor *indicesAcc;
        AkBuffer   *indicesBuff;
        json_t     *jacc;
        size_t      count, i, itemSize;

        jacc        = json_array_get(jaccessors, json_integer_value(jindices));
        indicesAcc  = gltf_accessor(gst, prim, jacc);
        indicesBuff = indicesAcc->source.ptr;
        itemSize    = indicesAcc->type->size;
        count       = indicesAcc->count;

        if (indicesBuff) {
          AkUIntArray *indices;
          AkUInt      *it1;
          char        *it2;

          indices = ak_heap_alloc(heap,
                                  prim,
                                  sizeof(*indices)
                                    + sizeof(AkUInt) * count);
          indices->count = count;
          it1 = indices->items;
          it2 = indicesBuff->data;

          /* we cannot use memcpy here, because we will promote short, byte
             type to int32 (for now)
           */
          for (i = 0; i < count; i++) {
            it1[i] = *(AkUInt *)(it2 + itemSize * i);
          }

          prim->indices     = indices;
          prim->indexStride = 1;
        }
      }

      gltf_setPrimMode(prim, json_int32(jprim, _s_gltf_mode));

      prim->mesh        = mesh;
      mesh->primitive   = prim;
      if (last_mesh)
        last_mesh->next = meshObj;
      else
        geom->gdata = meshObj;

      last_mesh = meshObj;
    }

    if (last_geom)
      last_geom->next = geom;
    else
      lib->chld = geom;

    last_geom = geom;
    lib->count++;
  }

  doc->lib.geometries = lib;
}

void _assetkit_hide
gltf_setPrimMode(AkMeshPrimitive *prim, int mode) {
  switch (mode) {
    case 0:
      prim->type = AK_MESH_PRIMITIVE_TYPE_POINTS;
      break;
    case 1: {
      AkLines *lines;
      lines = (AkLines *)prim;
      prim->type  = AK_MESH_PRIMITIVE_TYPE_LINES;
      lines->mode = AK_LINE_MODE_LINES;
      break;
    }
    case 2: {
      AkLines *lines;
      lines = (AkLines *)prim;
      prim->type  = AK_MESH_PRIMITIVE_TYPE_LINES;
      lines->mode = AK_LINE_MODE_LINE_LOOP;
      break;
    }
    case 3: {
      AkLines *lines;
      lines = (AkLines *)prim;
      prim->type  = AK_MESH_PRIMITIVE_TYPE_LINES;
      lines->mode = AK_LINE_MODE_LINE_STRIP;
      break;
    }
    case 4: {
      AkTriangles *tri;
      tri = (AkTriangles *)prim;
      prim->type = AK_MESH_PRIMITIVE_TYPE_LINES;
      tri->mode  = AK_TRIANGLE_MODE_TRIANGLES;
      break;
    }
    case 5: {
      AkTriangles *tri;
      tri = (AkTriangles *)prim;
      prim->type = AK_MESH_PRIMITIVE_TYPE_LINES;
      tri->mode  = AK_TRIANGLE_MODE_TRIANGLE_STRIP;
      break;
    }
    case 6: {
      AkTriangles *tri;
      tri = (AkTriangles *)prim;
      prim->type = AK_MESH_PRIMITIVE_TYPE_LINES;
      tri->mode  = AK_TRIANGLE_MODE_TRIANGLE_FAN;
      break;
    }
  }
}