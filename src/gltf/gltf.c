/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "gltf.h"
#include "../ak_id.h"
#include "../../include/ak-path.h"
#include "core/gltf_asset.h"
#include "core/gltf_buffer.h"
#include "core/gltf_mesh.h"
#include "core/gltf_node.h"

AkResult _assetkit_hide
ak_gltf_doc(AkDoc     ** __restrict dest,
            const char * __restrict filepath) {
  AkHeap      *heap;
  AkDoc       *doc;
  AkAssetInf  *ainf;
  AkGLTFState  gstVal, *gst;
  json_error_t error;
  AkResult     ret;

  heap = ak_heap_new(NULL, NULL, NULL);
  doc  = ak_heap_calloc(heap, NULL, sizeof(*doc));

  doc->inf = ak_heap_calloc(heap, doc, sizeof(*doc->inf));
  doc->inf->name = filepath;
  doc->inf->dir  = ak_path_dir(heap, doc, filepath);

  if (doc->inf->dir)
    doc->inf->dirlen = strlen(doc->inf->dir);

  ak_heap_setdata(heap, doc);
  ak_id_newheap(heap);

  memset(&gstVal, 0, sizeof(gstVal));

  gst         = &gstVal;
  gstVal.doc  = doc;
  gstVal.heap = heap;
  gstVal.root = json_load_file(filepath, 0, &error);

  ainf = NULL;
  ret  = gltf_asset(gst, doc, ainf);

  /* probably unsupportted version or verion is missing */
  if (ret == AK_EBADF) {
    ak_free(doc);
    return ret;
  }

  gst->bufferViews = rb_newtree(ds_allocator(),
                                ds_cmp_i32p,
                                NULL);

  gltf_buffers(gst);
  gltf_meshes(gst);
  gltf_nodes(gst);

  *dest = doc;

  return AK_OK;
}
