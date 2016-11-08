/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "ak_collada_fx_float_or_param.h"
#include "../core/ak_collada_param.h"

AkResult _assetkit_hide
ak_dae_floatOrParam(AkDaeState * __restrict daestate,
                    void * __restrict memParent,
                    const char * elm,
                    AkFxFloatOrParam ** __restrict dest) {
  AkFxFloatOrParam *floatOrParam;
  AkParam          *last_param;

  floatOrParam = ak_heap_calloc(daestate->heap,
                                memParent,
                                sizeof(*floatOrParam),
                                false);
  last_param = NULL;

  do {
    _xml_beginElement(elm);

    if (_xml_eqElm(_s_dae_float)) {
      ak_basic_attrf * valuef;
      const char * floatStr;

      valuef = ak_heap_calloc(daestate->heap,
                              floatOrParam,
                              sizeof(*valuef),
                              false);
      _xml_readAttr(valuef, valuef->sid, _s_dae_sid);
      _xml_readConstText(floatStr);
      if (floatStr)
        valuef->val = strtof(floatStr, NULL);

      floatOrParam->val = valuef;
    } else if (_xml_eqElm(_s_dae_param)) {
      AkParam * param;
      AkResult   ret;

      ret = ak_dae_param(daestate,
                         floatOrParam,
                         AK_PARAM_TYPE_BASIC,
                         &param);

      if (ret == AK_OK) {
        if (last_param)
          last_param->next = param;
        else
          floatOrParam->param = param;

        last_param = param;
      }
    } else {
      _xml_skipElement;
    }

    /* end element */
    _xml_endElement;
  } while (daestate->nodeRet);
  
  *dest = floatOrParam;

  return AK_OK;
}
