/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "ak_collada_morph.h"
#include "ak_collada_source.h"
#include "ak_collada_enums.h"
#include "../../ak_array.h"

AkResult _assetkit_hide
ak_dae_morph(AkDaeState * __restrict daestate,
             void * __restrict memParent,
             bool asObject,
             AkMorph ** __restrict dest) {
  AkObject *obj;
  AkMorph  *morph;
  AkSource *last_source;
  void     *memPtr;

  if (asObject) {
    obj = ak_objAlloc(daestate->heap,
                      memParent,
                      sizeof(*morph),
                      0,
                      true,
                      false);

    morph = ak_objGet(obj);

    memPtr = obj;
  } else {
    morph = ak_heap_calloc(daestate->heap, memParent, sizeof(*morph), false);
    memPtr = morph;
  }

  _xml_readAttr(memPtr, morph->baseMesh, _s_dae_source);
  _xml_readAttrAsEnumWithDef(morph->method,
                             _s_dae_method,
                             ak_dae_enumMorphMethod,
                             AK_MORPH_METHOD_NORMALIZED);

  last_source = NULL;

  do {
    _xml_beginElement(_s_dae_morph);

    if (_xml_eqElm(_s_dae_source)) {
      AkSource *source;
      AkResult ret;

      ret = ak_dae_source(daestate, memPtr, &source);
      if (ret == AK_OK) {
        if (last_source)
          last_source->next = source;
        else
          morph->source = source;

        last_source = source;
      }
    } else if (_xml_eqElm(_s_dae_targets)) {
      AkTargets    *targets;
      AkInputBasic *last_input;

      targets = ak_heap_calloc(daestate->heap,
                               memPtr,
                               sizeof(*targets),
                               false);

      last_input = NULL;

      do {
        _xml_beginElement(_s_dae_targets);

        if (_xml_eqElm(_s_dae_input)) {
          AkInputBasic *input;

          input = ak_heap_calloc(daestate->heap,
                                 targets,
                                 sizeof(*input),
                                 false);

          _xml_readAttr(input, input->semanticRaw, _s_dae_semantic);

          ak_url_from_attr(daestate->reader,
                           _s_dae_source,
                           input,
                           &input->source);

          if (!input->semanticRaw || !input->source.url)
            ak_free(input);
          else {
            AkEnum inputSemantic;
            inputSemantic = ak_dae_enumInputSemantic(input->semanticRaw);

            if (inputSemantic < 0)
              inputSemantic = AK_INPUT_SEMANTIC_OTHER;

            input->semantic = inputSemantic;
          }

          if (last_input)
            last_input->next = input;
          else
            targets->input = input;

          last_input = input;
        } else if (_xml_eqElm(_s_dae_extra)) {
          xmlNodePtr nodePtr;
          AkTree   *tree;

          nodePtr = xmlTextReaderExpand(daestate->reader);
          tree = NULL;

          ak_tree_fromXmlNode(daestate->heap,
                              memPtr,
                              nodePtr,
                              &tree,
                              NULL);
          morph->extra = tree;

          _xml_skipElement;

        } else {
          _xml_skipElement;
        }

        /* end element */
        _xml_endElement;
      } while (daestate->nodeRet);

      morph->targets = targets;
    } else if (_xml_eqElm(_s_dae_extra)) {
      xmlNodePtr nodePtr;
      AkTree   *tree;
      
      nodePtr = xmlTextReaderExpand(daestate->reader);
      tree = NULL;
      
      ak_tree_fromXmlNode(daestate->heap,
                          memPtr,
                          nodePtr,
                          &tree,
                          NULL);
      morph->extra = tree;
      
      _xml_skipElement;
    } else {
      _xml_skipElement;
    }
    
    /* end element */
    _xml_endElement;
  } while (daestate->nodeRet);
  
  *dest = morph;
  
  return AK_OK;
}
