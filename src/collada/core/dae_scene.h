/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef __libassetkit__dae_scene_h_
#define __libassetkit__dae_scene_h_

#include "../dae_common.h"

AkResult _assetkit_hide
dae_scene(AkXmlState * __restrict xst,
          void * __restrict memParent,
          AkScene * __restrict dest);

#endif /* __libassetkit__dae_scene_h_ */
