/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "ak_collada_camera.h"
#include "ak_collada_asset.h"
#include "ak_collada_technique.h"

AkResult _assetkit_hide
ak_dae_camera(AkDaeState * __restrict daestate,
              void * __restrict memParent,
              AkCamera ** __restrict  dest) {
  AkCamera *camera;

  camera = ak_heap_calloc(daestate->heap, memParent, sizeof(*camera), true);

  _xml_readId(camera);
  _xml_readAttr(camera, camera->name, _s_dae_name);

  do {
    _xml_beginElement(_s_dae_camera);

    if (_xml_eqElm(_s_dae_asset)) {
      AkAssetInf *assetInf;
      AkResult ret;

      assetInf = NULL;
      ret = ak_dae_assetInf(daestate, camera, &assetInf);
      if (ret == AK_OK)
        camera->inf = assetInf;

    } else if (_xml_eqElm(_s_dae_optics)) {
      AkOptics           *optics;
      AkTechnique        *last_tq;
      AkTechniqueCommon *last_tc;

      optics = ak_heap_calloc(daestate->heap, camera, sizeof(*optics), false);

      last_tq = optics->technique;
      last_tc = optics->techniqueCommon;

      do {
        _xml_beginElement(_s_dae_optics);

        if (_xml_eqElm(_s_dae_techniquec)) {
          AkTechniqueCommon *tc;
          AkResult ret;

          tc = NULL;
          ret = ak_dae_techniquec(daestate, optics, &tc);
          if (ret == AK_OK) {
            if (last_tc)
              last_tc->next = tc;
            else
              optics->techniqueCommon = tc;

            last_tc = tc;
          }

        } else if (_xml_eqElm(_s_dae_technique)) {
          AkTechnique *tq;
          AkResult ret;

          tq = NULL;
          ret = ak_dae_technique(daestate, optics, &tq);
          if (ret == AK_OK) {
            if (last_tq)
              last_tq->next = tq;
            else
              optics->technique = tq;

            last_tq = tq;
          }
        } else {
          _xml_skipElement;
        }

        /* end element */
        _xml_endElement;
      } while (daestate->nodeRet);

      camera->optics = optics;
    } else if (_xml_eqElm(_s_dae_imager)) {
      AkImager    *imager;
      AkTechnique *last_tq;

      imager  = ak_heap_calloc(daestate->heap, camera, sizeof(*imager), false);
      last_tq = imager->technique;

      do {
        _xml_beginElement(_s_dae_imager);

        if (_xml_eqElm(_s_dae_technique)) {
          AkTechnique *tq;
          AkResult ret;

          tq = NULL;
          ret = ak_dae_technique(daestate, imager, &tq);
          if (ret == AK_OK) {
            if (last_tq)
              last_tq->next = tq;
            else
              imager->technique = tq;

            last_tq = tq;
          }
        } else if (_xml_eqElm(_s_dae_extra)) {
          xmlNodePtr nodePtr;
          AkTree   *tree;

          nodePtr = xmlTextReaderExpand(daestate->reader);
          tree = NULL;

          ak_tree_fromXmlNode(daestate->heap,
                              imager,
                              nodePtr,
                              &tree,
                              NULL);
          imager->extra = tree;

          _xml_skipElement;
        } else {
          _xml_skipElement;
        }
        
        /* end element */
        _xml_endElement;
      } while (daestate->nodeRet);

      camera->imager = imager;
    } else if (_xml_eqElm(_s_dae_extra)) {
      xmlNodePtr nodePtr;
      AkTree   *tree;

      nodePtr = xmlTextReaderExpand(daestate->reader);
      tree = NULL;

      ak_tree_fromXmlNode(daestate->heap,
                          camera,
                          nodePtr,
                          &tree,
                          NULL);
      camera->extra = tree;

      _xml_skipElement;
    }

    /* end element */
    _xml_endElement;
  } while (daestate->nodeRet);

  *dest = camera;

  return AK_OK;
}
