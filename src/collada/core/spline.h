/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef __libassetkit__dae_spline_h_
#define __libassetkit__dae_spline_h_

#include "../common.h"

AkObject* _assetkit_hide
dae_spline(DAEState   * __restrict dst,
           xml_t      * __restrict xml,
           AkGeometry * __restrict geom);

#endif /* __libassetkit__dae_spline_h_ */