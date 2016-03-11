/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "aio_collada_fx_uniform.h"
#include "../aio_collada_common.h"
#include "../aio_collada_param.h"
#include "../aio_collada_value.h"

int _assetio_hide
aio_dae_fxBindUniform(xmlTextReaderPtr __restrict reader,
                      aio_bind_uniform ** __restrict dest) {
  aio_bind_uniform *bind_uniform;
  aio_param        *last_param;
  const xmlChar *nodeName;
  int            nodeType;
  int            nodeRet;

  bind_uniform = aio_calloc(sizeof(*bind_uniform), 1);

  _xml_readAttr(bind_uniform->symbol, _s_dae_symbol);

  last_param = NULL;

  do {
    _xml_beginElement(_s_dae_bind_uniform);

    if (_xml_eqElm(_s_dae_param)) {
      aio_param * param;
      int         ret;

      ret = aio_dae_param(reader,
                          AIO_PARAM_TYPE_BASIC,
                          &param);

      if (ret == 0) {
        if (last_param)
          last_param->next = param;
        else
          bind_uniform->param = param;

        last_param = param;
      }
    } else {
      /* load once */
      if (!bind_uniform->val) {
        void           * val;
        aio_value_type   val_type;
        int              ret;

        ret = aio_dae_value(reader,
                            &val,
                            &val_type);

        if (ret == 0) {
          bind_uniform->val = val;
          bind_uniform->val_type = val_type;
        }
      }
    }

    /* end element */
    _xml_endElement;
  } while (nodeRet);

  *dest = bind_uniform;
  
  return 0;
}
