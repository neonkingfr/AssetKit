/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "ak_collada_brep_surface.h"
#include "ak_collada_brep_nurbs.h"
#include "ak_collada_brep_curve.h"
#include "../../ak_array.h"

#define k_s_dae_orient 101
#define k_s_dae_origin 102

static ak_enumpair surfaceMap[] = {
  {_s_dae_cone,          AK_SURFACE_ELEMENT_TYPE_CONE},
  {_s_dae_plane,         AK_SURFACE_ELEMENT_TYPE_PLANE},
  {_s_dae_cylinder,      AK_SURFACE_ELEMENT_TYPE_CYLINDER},
  {_s_dae_nurbs_surface, AK_SURFACE_ELEMENT_TYPE_NURBS_SURFACE},
  {_s_dae_sphere,        AK_SURFACE_ELEMENT_TYPE_SPHERE},
  {_s_dae_torus,         AK_SURFACE_ELEMENT_TYPE_TORUS},
  {_s_dae_swept_surface, AK_SURFACE_ELEMENT_TYPE_SWEPT_SURFACE},
  {_s_dae_orient,        k_s_dae_orient},
  {_s_dae_origin,        k_s_dae_origin}
};

static size_t surfaceMapLen = 0;

AkResult _assetkit_hide
ak_dae_surface(AkDaeState * __restrict daestate,
               void * __restrict memParent,
               AkSurface ** __restrict dest) {
  AkSurface      *surface;
  AkDoubleArrayL *last_orient;

  surface = ak_heap_calloc(daestate->heap,
                           memParent,
                           sizeof(*surface),
                           false);


  _xml_readAttr(surface, surface->sid, _s_dae_sid);
  _xml_readAttr(surface, surface->name, _s_dae_name);

  if (surfaceMapLen == 0) {
    surfaceMapLen = AK_ARRAY_LEN(surfaceMap);
    qsort(surfaceMap,
          surfaceMapLen,
          sizeof(surfaceMap[0]),
          ak_enumpair_cmp);
  }

  last_orient = NULL;

  do {
    const ak_enumpair *found;

    _xml_beginElement(_s_dae_surface);

    found = bsearch(daestate->nodeName,
                    surfaceMap,
                    surfaceMapLen,
                    sizeof(surfaceMap[0]),
                    ak_enumpair_cmp2);
    switch (found->val) {
      case AK_SURFACE_ELEMENT_TYPE_CONE: {
        AkObject *obj;
        AkCone   *cone;

        obj = ak_objAlloc(daestate->heap,
                          surface,
                          sizeof(*cone),
                          AK_SURFACE_ELEMENT_TYPE_CONE,
                          true,
                          false);

        cone = ak_objGet(obj);

        do {
          _xml_beginElement(_s_dae_surface);

          if (_xml_eqElm(_s_dae_radius)) {
            _xml_readTextUsingFn(cone->angle,
                                 strtof, NULL);
          } else if (_xml_eqElm(_s_dae_angle)) {
            _xml_readTextUsingFn(cone->radius,
                                 strtof, NULL);
          } else if (_xml_eqElm(_s_dae_extra)) {
            xmlNodePtr nodePtr;
            AkTree    *tree;

            nodePtr = xmlTextReaderExpand(daestate->reader);
            tree = NULL;

            ak_tree_fromXmlNode(daestate->heap,
                                obj,
                                nodePtr,
                                &tree,
                                NULL);
            cone->extra = tree;

            _xml_skipElement;
            break;
          } else {
            _xml_skipElement;
          }

          /* end element */
          _xml_endElement;
        } while (daestate->nodeRet);
        
        surface->surface = obj;
        break;
      }
      case AK_SURFACE_ELEMENT_TYPE_PLANE: {
        AkObject *obj;
        AkPlane  *plane;

        obj = ak_objAlloc(daestate->heap,
                          surface,
                          sizeof(*plane),
                          AK_SURFACE_ELEMENT_TYPE_CONE,
                          true,
                          false);

        plane = ak_objGet(obj);

        do {
          _xml_beginElement(_s_dae_plane);

          if (_xml_eqElm(_s_dae_radius)) {
            char *content;
            _xml_readMutText(content);

            if (content) {
              ak_strtod(&content, (AkDouble *)&plane->equation, 4);
              xmlFree(content);
            }
          } else if (_xml_eqElm(_s_dae_extra)) {
            xmlNodePtr nodePtr;
            AkTree    *tree;

            nodePtr = xmlTextReaderExpand(daestate->reader);
            tree = NULL;

            ak_tree_fromXmlNode(daestate->heap,
                                obj,
                                nodePtr,
                                &tree,
                                NULL);
            plane->extra = tree;

            _xml_skipElement;
            break;
          } else {
            _xml_skipElement;
          }

          /* end element */
          _xml_endElement;
        } while (daestate->nodeRet);
        
        surface->surface = obj;
        break;
      }
      case AK_SURFACE_ELEMENT_TYPE_CYLINDER: {
        AkObject   *obj;
        AkCylinder *cylinder;

        obj = ak_objAlloc(daestate->heap,
                          surface,
                          sizeof(*cylinder),
                          AK_SURFACE_ELEMENT_TYPE_CYLINDER,
                          true,
                          false);

        cylinder = ak_objGet(obj);

        do {
          _xml_beginElement(_s_dae_cylinder);

          if (_xml_eqElm(_s_dae_radius)) {
            char *content;
            _xml_readMutText(content);

            if (content) {
              ak_strtof(&content, (AkFloat *)&cylinder->radius, 2);
              xmlFree(content);
            }
          } else if (_xml_eqElm(_s_dae_extra)) {
            xmlNodePtr nodePtr;
            AkTree    *tree;

            nodePtr = xmlTextReaderExpand(daestate->reader);
            tree = NULL;

            ak_tree_fromXmlNode(daestate->heap,
                                obj,
                                nodePtr,
                                &tree,
                                NULL);
            cylinder->extra = tree;

            _xml_skipElement;
            break;
          } else {
            _xml_skipElement;
          }

          /* end element */
          _xml_endElement;
        } while (daestate->nodeRet);
        
        surface->surface = obj;
        break;
      }
      case AK_SURFACE_ELEMENT_TYPE_NURBS_SURFACE: {
        AkNurbsSurface *nurbsSurface;
        AkResult ret;

        ret = ak_dae_nurbs_surface(daestate,
                                   surface,
                                   true,
                                   &nurbsSurface);
        if (ret == AK_OK) {
          surface->surface = ak_objFrom(nurbsSurface);
          surface->surface->type = AK_SURFACE_ELEMENT_TYPE_NURBS_SURFACE;
        }

        break;
      }
      case AK_SURFACE_ELEMENT_TYPE_SPHERE: {
        AkObject *obj;
        AkSphere *sphere;

        obj = ak_objAlloc(daestate->heap,
                          surface,
                          sizeof(*sphere),
                          AK_SURFACE_ELEMENT_TYPE_SPHERE,
                          true,
                          false);

        sphere = ak_objGet(obj);

        do {
          _xml_beginElement(_s_dae_sphere);

          if (_xml_eqElm(_s_dae_radius)) {
            _xml_readTextUsingFn(sphere->radius,
                                 strtof, NULL);
          } else if (_xml_eqElm(_s_dae_extra)) {
            xmlNodePtr nodePtr;
            AkTree    *tree;

            nodePtr = xmlTextReaderExpand(daestate->reader);
            tree = NULL;

            ak_tree_fromXmlNode(daestate->heap,
                                obj,
                                nodePtr,
                                &tree,
                                NULL);
            sphere->extra = tree;

            _xml_skipElement;
            break;
          } else {
            _xml_skipElement;
          }

          /* end element */
          _xml_endElement;
        } while (daestate->nodeRet);
        
        surface->surface = obj;
        break;
      }
      case AK_SURFACE_ELEMENT_TYPE_TORUS: {
        AkObject *obj;
        AkTorus  *torus;

        obj = ak_objAlloc(daestate->heap,
                          surface,
                          sizeof(*torus),
                          AK_SURFACE_ELEMENT_TYPE_TORUS,
                          true,
                          false);

        torus = ak_objGet(obj);

        do {
          _xml_beginElement(_s_dae_torus);

          if (_xml_eqElm(_s_dae_radius)) {
            char *content;
            _xml_readMutText(content);

            if (content) {
              ak_strtof(&content, (AkFloat *)&torus->radius, 2);
              xmlFree(content);
            }
          } else if (_xml_eqElm(_s_dae_extra)) {
            xmlNodePtr nodePtr;
            AkTree    *tree;

            nodePtr = xmlTextReaderExpand(daestate->reader);
            tree = NULL;

            ak_tree_fromXmlNode(daestate->heap,
                                obj,
                                nodePtr,
                                &tree,
                                NULL);
            torus->extra = tree;

            _xml_skipElement;
            break;
          } else {
            _xml_skipElement;
          }

          /* end element */
          _xml_endElement;
        } while (daestate->nodeRet);
        
        surface->surface = obj;
        break;
      }
      case AK_SURFACE_ELEMENT_TYPE_SWEPT_SURFACE: {
        AkObject       *obj;
        AkSweptSurface *sweptSurface;

        obj = ak_objAlloc(daestate->heap,
                          surface,
                          sizeof(*sweptSurface),
                          AK_SURFACE_ELEMENT_TYPE_SWEPT_SURFACE,
                          true,
                          false);

        sweptSurface = ak_objGet(obj);

        do {
          _xml_beginElement(_s_dae_swept_surface);

          if (_xml_eqElm(_s_dae_curve)) {
            AkCurve *curve;
            AkResult ret;

            ret = ak_dae_curve(daestate,
                               surface,
                               false,
                               &curve);
            if (ret == AK_OK)
              sweptSurface->curve = curve;
          } else if (_xml_eqElm(_s_dae_direction)) {
            char *content;
            _xml_readMutText(content);

            if (content) {
              ak_strtof(&content, (AkFloat *)&sweptSurface->direction, 3);
              xmlFree(content);
            }
          } else if (_xml_eqElm(_s_dae_origin)) {
            char *content;
            _xml_readMutText(content);

            if (content) {
              ak_strtof(&content, (AkFloat *)&sweptSurface->origin, 3);
              xmlFree(content);
            }
          } else if (_xml_eqElm(_s_dae_axis)) {
            char *content;
            _xml_readMutText(content);

            if (content) {
              ak_strtof(&content, (AkFloat *)&sweptSurface->axis, 3);
              xmlFree(content);
            }
          } else if (_xml_eqElm(_s_dae_extra)) {
            xmlNodePtr nodePtr;
            AkTree    *tree;

            nodePtr = xmlTextReaderExpand(daestate->reader);
            tree = NULL;

            ak_tree_fromXmlNode(daestate->heap,
                                obj,
                                nodePtr,
                                &tree,
                                NULL);
            sweptSurface->extra = tree;

            _xml_skipElement;
            break;
          } else {
            _xml_skipElement;
          }

          /* end element */
          _xml_endElement;
        } while (daestate->nodeRet);
        
        surface->surface = obj;
        break;
      }
      case k_s_dae_orient: {
        char *content;
        _xml_readMutText(content);

        if (content) {
          AkDoubleArrayL *orient;
          AkResult ret;

          ret = ak_strtod_arrayL(daestate->heap,
                                 surface,
                                 content,
                                 &orient);
          if (ret == AK_OK) {
            if (last_orient)
              last_orient->next = orient;
            else
              surface->orient = orient;

            last_orient = orient;
          }

          xmlFree(content);
        }

        break;
      }
      case k_s_dae_origin: {
        char *content;
        _xml_readMutText(content);

        if (content)
          ak_strtod(&content, surface->origin, 3);
        
        break;
      }
      default:
        _xml_skipElement;
        break;
    }
    
    /* end element */
    _xml_endElement;
  } while (daestate->nodeRet);
  
  *dest = surface;
  
  return AK_OK;
}

AkResult _assetkit_hide
ak_dae_surfaces(AkDaeState * __restrict daestate,
                void * __restrict memParent,
                AkSurfaces ** __restrict dest) {
  AkSurfaces    *surfaces;
  AkSurface     *last_surface;

  surfaces = ak_heap_calloc(daestate->heap,
                            memParent,
                            sizeof(*surfaces),
                            false);

  last_surface= NULL;

  do {
    _xml_beginElement(_s_dae_surfaces);

    if (_xml_eqElm(_s_dae_surface)) {
      AkSurface *surface;
      AkResult ret;

      ret = ak_dae_surface(daestate, surfaces, &surface);
      if (ret == AK_OK) {
        if (last_surface)
          last_surface->next = surface;
        else
          surfaces->surface = surface;

        last_surface = surface;
      }
    } else if (_xml_eqElm(_s_dae_extra)) {
      xmlNodePtr nodePtr;
      AkTree    *tree;

      nodePtr = xmlTextReaderExpand(daestate->reader);
      tree = NULL;

      ak_tree_fromXmlNode(daestate->heap,
                          surfaces,
                          nodePtr,
                          &tree,
                          NULL);
      surfaces->extra = tree;

      _xml_skipElement;
    }

    /* end element */
    _xml_endElement;
  } while (daestate->nodeRet);
  
  *dest = surfaces;
  
  return AK_OK;
}
