/*
 * Copyright (C) 2020 Recep Aslantas
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "group.h"
#include "util.h"

_assetkit_hide
void
wobj_finishObject(WOState * __restrict wst) {
  AkHeap             *heap;
  AkInstanceGeometry *instGeom;
  AkGeometry         *geom;
  AkMesh             *mesh;
  AkPolygon          *poly;

  /* TODO: Release resources */

  if (!wst->obj.geom)
    return;

  /* Buffer > Accessor > Input > Prim > Mesh > Geom > InstanceGeom > Node */

  heap = wst->heap;
  geom = wst->obj.geom;
  mesh = ak_objGet(geom->gdata);
  poly = (void *)mesh->primitive;

  /* add to library */
  geom->base.next     = wst->lib_geom->chld;
  wst->lib_geom->chld = &geom->base;
  
  /* make instance geeometry and attach to the root node  */
  instGeom = ak_instanceMakeGeom(wst->heap, wst->node, wst->obj.geom);
  
  if (wst->node->geometry) {
    wst->node->geometry->base.prev = (void *)instGeom;
    instGeom->base.next            = &wst->node->geometry->base;
  }

  instGeom->base.next = (void *)wst->node->geometry;
  wst->node->geometry = instGeom;

  poly->vcount = ak_heap_calloc(heap,
                                poly,
                                sizeof(*poly->vcount)
                                + wst->obj.dc_vcount->usedsize);
  poly->vcount->count = wst->obj.dc_vcount->itemcount;
  ak_data_join(wst->obj.dc_vcount, poly->vcount->items, 0, 0);

  poly->base.inputCount  = 1;
  poly->base.type        = AK_PRIMITIVE_POLYGONS;
  poly->base.count       = 6;
  
  poly->base.pos =
  wobj_addInput(wst, wst->obj.dc_pos,
                (void *)poly,
                AK_INPUT_SEMANTIC_POSITION,
                "POSITION",
                AK_COMPONENT_SIZE_VEC3,
                AKT_FLOAT,
                0);
  
  if (wst->obj.dc_nor->itemcount > 0) {
    wobj_addInput(wst,
                  wst->obj.dc_nor,
                  (void *)poly,
                  AK_INPUT_SEMANTIC_NORMAL,
                  "NORMAL",
                  AK_COMPONENT_SIZE_VEC3,
                  AKT_FLOAT,
                  1);
  }
  
  if (wst->obj.dc_tex->itemcount > 0) {
    wobj_addInput(wst,
                  wst->obj.dc_tex,
                  (void *)poly,
                  AK_INPUT_SEMANTIC_TEXCOORD,
                  "TEXCOORD",
                  AK_COMPONENT_SIZE_VEC3,
                  AKT_FLOAT,
                  2);
  }

  wobj_joinIndices(wst, (void *)poly);
  wobj_fixIndices((void *)poly);

  /* cleanup */
  if (wst->obj.dc_indv) {
    ak_free(wst->obj.dc_indv);
    ak_free(wst->obj.dc_indt);
    ak_free(wst->obj.dc_indn);
    ak_free(wst->obj.dc_pos);
    ak_free(wst->obj.dc_tex);
    ak_free(wst->obj.dc_nor);
    ak_free(wst->obj.dc_vcount);
  }
  
  memset(&wst->obj, 0, sizeof(wst->obj));
}

_assetkit_hide
void
wobj_switchObject(WOState * __restrict wst) {
  AkGeometry *geom;
  AkMesh     *mesh;
  AkPolygon  *poly;
  
  wobj_finishObject(wst);

  mesh = ak_allocMesh(wst->heap, wst->lib_geom, &geom);
  poly = ak_heap_calloc(wst->heap, ak_objFrom(mesh), sizeof(*poly));
  
  poly->base.type      = AK_PRIMITIVE_POLYGONS;
  poly->base.mesh      = mesh;

  mesh->primitive      = (AkMeshPrimitive *)poly;
  mesh->primitiveCount = 1;
  
  /* set current geometry */
  wst->obj.geom = geom;
  
  /* vertex index */
  wst->obj.dc_indv = ak_data_new(wst->doc, 128, sizeof(int32_t), NULL);
  wst->obj.dc_indt = ak_data_new(wst->doc, 128, sizeof(int32_t), NULL);
  wst->obj.dc_indn = ak_data_new(wst->doc, 128, sizeof(int32_t), NULL);

  /* vertex data */
  wst->obj.dc_pos    = ak_data_new(wst->doc, 128, sizeof(vec3),    NULL);
  wst->obj.dc_tex    = ak_data_new(wst->doc, 128, sizeof(vec3),    NULL);
  wst->obj.dc_nor    = ak_data_new(wst->doc, 128, sizeof(vec3),    NULL);
  wst->obj.dc_vcount = ak_data_new(wst->doc, 128, sizeof(int32_t), NULL);
}

_assetkit_hide
void
wobj_switchGroup(WOState * __restrict wst) {
  wobj_switchObject(wst);
}

_assetkit_hide
void
wobj_finishGroup(WOState * __restrict wst) {
  wobj_finishObject(wst);
}
