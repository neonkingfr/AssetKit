/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef __libassetkit__ak_collada_lines_h_
#define __libassetkit__ak_collada_lines_h_

#include "../ak_collada_common.h"

AkResult _assetkit_hide
ak_dae_lines(AkDaeState * __restrict daestate,
             void * __restrict memParent,
             AkLineMode mode,
             AkLines ** __restrict dest);

#endif /* __libassetkit__ak_collada_lines_h_ */
