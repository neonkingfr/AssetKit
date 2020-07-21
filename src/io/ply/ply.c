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

/*
 Resources:
   http://people.math.sc.edu/Burkardt/data/ply/ply.txt
   http://paulbourke.net/dataformats/ply/
   https://en.wikipedia.org/wiki/PLY_(file_format)
*/

#include "ply.h"
#include "common.h"
//#include "postscript.h"
#include "strpool.h"
#include "../../id.h"
#include "../../data.h"
#include "../../../include/ak/path.h"
#include "../common/util.h"

AkResult AK_HIDE
ply_ply(AkDoc     ** __restrict dest,
        const char * __restrict filepath) {
  AkHeap        *heap;
  AkDoc         *doc;
  void          *plystr;
  char          *p, *b, *e;
  AkLibrary     *lib_geom, *lib_vscene;
  AkVisualScene *scene;
  PLYElement    *elem;
  PLYProperty   *prop, *pit;
  PLYState       pstVal = {0}, *pst;
  size_t         plystrSize, off;
  uint32_t       i;
  AkResult       ret;
  bool           isAscii, isBigEndian;
  char           c;

  if ((ret = ak_readfile(filepath, "rb", &plystr, &plystrSize)) != AK_OK
      || !((p = plystr) && *p != '\0'))
    return AK_ERR;

  if (!(p[0] == 'p' && p[1] == 'l' && p[2] == 'y'))
    return AK_ERR;
  
  p += 3;
  c  = *p;

  NEXT_LINE

  elem = NULL;
  prop = NULL;
  heap = ak_heap_new(NULL, NULL, NULL);
  doc  = ak_heap_calloc(heap, NULL, sizeof(*doc));

  doc->inf                = ak_heap_calloc(heap, doc, sizeof(*doc->inf));
  doc->inf->name          = filepath;
  doc->inf->dir           = ak_path_dir(heap, doc, filepath);
  doc->inf->flipImage     = true;
  doc->inf->ftype         = AK_FILE_TYPE_PLY;
  doc->inf->base.coordSys = AK_YUP;
  doc->coordSys           = AK_YUP; /* Default */
  
  ak_heap_setdata(heap, doc);
  ak_id_newheap(heap);

  /* libraries */
  doc->lib.geometries = ak_heap_calloc(heap, doc, sizeof(*lib_geom));
  lib_vscene          = ak_heap_calloc(heap, doc, sizeof(*lib_vscene));

  /* default scene */
  scene                  = ak_heap_calloc(heap, doc, sizeof(*scene));
  scene->node            = ak_heap_calloc(heap, doc, sizeof(*scene->node));
  lib_vscene->chld       = &scene->base;
  lib_vscene->count      = 1;
  doc->lib.visualScenes  = lib_vscene;
  doc->scene.visualScene = ak_instanceMake(heap, doc, scene);

  /* parse state */
  memset(&pstVal, 0, sizeof(pstVal));
  pst              = &pstVal;
  pstVal.doc       = doc;
  pstVal.heap      = heap;
  pstVal.tmp = ak_heap_alloc(heap, doc, sizeof(void*));
  pstVal.node      = scene->node;
  pstVal.lib_geom  = doc->lib.geometries;

  pst->dc_ind    = ak_data_new(pst->tmp, 128, sizeof(int32_t), NULL);
  pst->dc_pos    = ak_data_new(pst->tmp, 128, sizeof(vec3),    NULL);
  pst->dc_nor    = ak_data_new(pst->tmp, 128, sizeof(vec3),    NULL);
  pst->dc_vcount = ak_data_new(pst->tmp, 128, sizeof(int32_t), NULL);

  isAscii     = false;
  isBigEndian = false;

  /* parse header */
  do {
    /* skip spaces */
    SKIP_SPACES

    /* parse format but ignore version (for now maybe) */
    if (EQ6('f', 'o', 'r', 'm', 'a', 't')) {
      p += 7;

      SKIP_SPACES

      if (EQ5('a', 's', 'c', 'i', 'i')) {
        isAscii = true;
      } else if (strncmp(p, _s_ply_binary_little_endian, 21) == 0) {
        isBigEndian = false;
      } else if (strncmp(p, _s_ply_binary_big_endian, 18) == 0) {
        isBigEndian = true;
      } else {
        goto err; /* unknown format */
      }
    } else if (EQ7('e', 'l', 'e', 'm', 'e', 'n', 't')) {
      p += 8;
      
      elem = ak_heap_calloc(heap, pst->tmp, sizeof(*elem));
      
      if (!pst->element)
        pst->element = elem;

      if (pst->lastElement)
        pst->lastElement->next = elem;
      pst->lastElement = elem;

      if (EQ6('v', 'e', 'r', 't', 'e', 'x')) {
        p += 7;
        SKIP_SPACES
        elem->count = strtoul(p, &p, 10);
        elem->type  = PLY_ELEM_VERTEX;
      } else if (EQ4('f', 'a', 'c', 'e')) {
        p += 5;
        SKIP_SPACES
        elem->count = strtoul(p, &p, 10);
        elem->type  = PLY_ELEM_FACE;
      }
    } else if (EQ8('p', 'r', 'o', 'p', 'e', 'r', 't', 'y')) {
      p += 9;
      SKIP_SPACES
      
      prop = ak_heap_calloc(heap, pst->tmp, sizeof(*prop));
      
      /* 1. type */
      
      b = p;
      while ((c = *++p) != '\0' && !AK_ARRAY_SPACE_CHECK);
      e = p;
      
      prop->islist = b[0] == 'l'
                  && b[1] == 'i'
                  && b[2] == 's'
                  && b[3] == 't';
      
      if (!prop->islist) {
        prop->typestr  = ak_heap_strndup(heap, doc, b, e - b);
        prop->typeDesc = ak_typeDescByName(prop->typestr);
      } else {
        /* 1.1 count type */
        SKIP_SPACES
        
        b = p;
        while ((c = *++p) != '\0' && !AK_ARRAY_SEP_CHECK);
        e = p;
        
        prop->listCountType     = ak_heap_strndup(heap, doc, b, e - b);
        prop->listCountTypeDesc = ak_typeDescByName(prop->listCountType);

        /* 1.2 type */
        SKIP_SPACES
        
        b = p;
        while ((c = *++p) != '\0' && !AK_ARRAY_SEP_CHECK);
        e = p;
        
        prop->typestr = ak_heap_strndup(heap, doc, b, e - b);
      }
      
      /* 2. name */
      
      SKIP_SPACES
      
      b = p;
      while ((c = *++p) != '\0' && !AK_ARRAY_SEP_CHECK);
      e = p;

      prop->name = ak_heap_strndup(heap, doc, b, e - b);
      
      if (prop->typeDesc)
        elem->buffsize += prop->typeDesc->size;

      if (e - b == 1) {
        switch (b[0]) {
          case 'x': prop->semantic = PLY_PROP_X; break;
          case 'y': prop->semantic = PLY_PROP_Y; break;
          case 'z': prop->semantic = PLY_PROP_Z; break;
          case 's':
          case 'u': prop->semantic = PLY_PROP_S; break;
          case 't':
          case 'v': prop->semantic = PLY_PROP_T; break;
          case 'r': prop->semantic = PLY_PROP_R; break;
          case 'g': prop->semantic = PLY_PROP_G; break;
          case 'b': prop->semantic = PLY_PROP_B; break;
          default:
            prop->semantic   = PLY_PROP_UNSUPPORTED;
            prop->ignore = true;
            break;
        }
      } else if (e - b == 2) {
        switch (b[0]) {
          case 'n':
            switch (b[1]) {
              case 'x': prop->semantic = PLY_PROP_NX; break;
              case 'y': prop->semantic = PLY_PROP_NY; break;
              case 'z': prop->semantic = PLY_PROP_NZ; break;
              default:
                prop->semantic   = PLY_PROP_UNSUPPORTED;
                prop->ignore = true;
                break;
            }
            break;
          default:
            prop->semantic   = PLY_PROP_UNSUPPORTED;
            prop->ignore = true;
            break;
        }
      }
      
      if (!elem->property) {
        elem->property = prop;
      } else {
        PLYProperty *last_prop;
        
        /* insert propety by ORDER */
        last_prop = pit = elem->property;
        while (pit) {
          if ((int)prop->semantic < (int)pit->semantic) {
            if (pit->prev) {
              pit->prev->next = prop;
              prop->prev      = pit->prev;
            }

            prop->next = pit;
            pit->prev  = prop;
            
            if (pit == elem->property)
              elem->property = prop;
            
            break;
          }

          last_prop = pit;
          pit       = pit->next;
        }
        
        /* couldn't add, so add to last */
        if (!pit && last_prop) {
          last_prop->next = prop;
          prop->prev      = last_prop;
        }
      }
    } else if (EQT7('e', 'n', 'd', '_', 'h', 'e', 'a')) {
      NEXT_LINE
      break;
    }

    NEXT_LINE
  } while (p && p[0] != '\0'/* && (c = *++p) != '\0'*/);

  /* prepare property offsets/slots */
  i    = off = 0;
  elem = pst->element;
  while (elem) {
    pit = elem->property;
    if (!pit->islist) {
      while (pit) {
        if (!pit->ignore) {
          pit->slot = i++;
          pit->off  = off;
          /* TODO: currently all are floats */
          off      += sizeof(float); /* pit->typeDesc->size; */
          
          if (pit->typeDesc)
            elem->knownByteStride += pit->typeDesc->size;
          elem->knownCount++;
        }
        
        if (pit->typeDesc)
          elem->byteStride += pit->typeDesc->size;
        pit = pit->next;
      }
    }

    /* alloc buffer */
    elem->buff         = ak_heap_calloc(heap, pst->tmp, sizeof(*elem->buff));
    elem->buff->length = off * elem->count;
    elem->buff->data   = ak_heap_alloc(heap, elem->buff, elem->buff->length);

    elem = elem->next;
  }
  
  /* parse */
  if (isAscii) {
    ply_ascii(p, pst);
  }

  *dest = doc;

  /* cleanup */
  ak_free(pst->tmp);

  return AK_OK;
  
err:
  ak_free(pst->tmp);
  ak_free(doc);
  return AK_ERR;
}

AK_HIDE
void
ply_ascii(char * __restrict src, PLYState * __restrict pst) {
  char        *p;
  float       *b;
  PLYElement  *elem;
  PLYProperty *prop;
  AkBuffer    *buff;
  char         c;
  uint32_t     i, stride;
  
  p    = src;
  c    = *p;
  elem = pst->element;
  i    = 0;

  while (elem) {
    if (elem->type == PLY_ELEM_VERTEX) {
      buff   = elem->buff;
      b      = buff->data; /* TODO: all vertices are floats for now */
      stride = elem->knownCount;

      do {
        SKIP_SPACES

        prop = elem->property;
        while (prop) {
          if (!prop->ignore)
            b[prop->slot] = strtof(p, &p);
          prop = prop->next;
        }

        b += stride;

        NEXT_LINE

        if (++i >= elem->count)
          break;
      } while (p && p[0] != '\0');
    }
    elem = elem->next;
  }
}

AK_HIDE
void
ply_finish(PLYState * __restrict pst) {
  AkHeap             *heap;
  AkGeometry         *geom;
  AkMesh             *mesh;
  AkMeshPrimitive    *prim;
  AkInstanceGeometry *instGeom;
  AkTriangles        *tri;

  /* Buffer > Accessor > Input > Prim > Mesh > Geom > InstanceGeom > Node */
  
  heap = pst->heap;
  mesh = ak_allocMesh(pst->heap, pst->lib_geom, &geom);

  tri = ak_heap_calloc(pst->heap, ak_objFrom(mesh), sizeof(*tri));
  tri->mode      = AK_TRIANGLE_FAN;
  tri->base.type = AK_PRIMITIVE_TRIANGLES;
  prim = (AkMeshPrimitive *)tri;

  prim->count          = pst->count;
  prim->mesh           = mesh;
  mesh->primitive      = prim;
  mesh->primitiveCount = 1;

  /* add to library */
  geom->base.next     = pst->lib_geom->chld;
  pst->lib_geom->chld = &geom->base;
  
  /* make instance geeometry and attach to the root node  */
  instGeom = ak_instanceMakeGeom(heap, pst->node, geom);
  if (pst->node->geometry) {
    pst->node->geometry->base.prev = (void *)instGeom;
    instGeom->base.next            = &pst->node->geometry->base;
  }

  instGeom->base.next = (void *)pst->node->geometry;
  pst->node->geometry = instGeom;
  
  prim->pos = io_addInput(heap, pst->dc_pos, prim, AK_INPUT_SEMANTIC_POSITION,
                          "POSITION", AK_COMPONENT_SIZE_VEC3, AKT_FLOAT, 0);

  if (pst->dc_nor->itemcount > 0) {
    io_addInput(heap, pst->dc_nor, prim, AK_INPUT_SEMANTIC_NORMAL,
                "NORMAL", AK_COMPONENT_SIZE_VEC3, AKT_FLOAT, 1);
  }

  /* cleanup */
  if (pst->dc_ind) {
    ak_free(pst->dc_ind);
    ak_free(pst->dc_pos);
    ak_free(pst->dc_nor);
    ak_free(pst->dc_vcount);
  }
  
  pst->dc_ind    = NULL;
  pst->dc_pos    = NULL;
  pst->dc_nor    = NULL;
  pst->dc_vcount = NULL;
}
