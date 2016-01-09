/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "aio_collada_fx_pass.h"

#include "../../aio_libxml.h"
#include "../../aio_types.h"
#include "../../aio_memory.h"
#include "../../aio_utils.h"
#include "../../aio_tree.h"

#include "../aio_collada_asset.h"
#include "../aio_collada_common.h"
#include "../aio_collada_annotate.h"

#include "aio_collada_fx_states.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <string.h>

int _assetio_hide
aio_load_fx_pass(xmlNode * __restrict xml_node,
                 aio_pass ** __restrict dest) {
  xmlNode  * curr_node;
  xmlAttr  * curr_attr;
  aio_pass * pass;

  curr_node = xml_node;
  pass = aio_malloc(sizeof(*pass));
  memset(pass, '\0', sizeof(*pass));

  curr_attr = curr_node->properties;

  /* parse attributes */
  while (curr_attr) {
    if (curr_attr->type == XML_ATTRIBUTE_NODE) {
      const char * attr_name;
      const char * attr_val;

      attr_name = (const char *)curr_attr->name;
      attr_val = aio_xml_node_content((xmlNode *)curr_attr);

      if (AIO_IS_EQ_CASE(attr_name, "sid")) {
        pass->sid = aio_strdup(attr_val);
        break;
      }
    }

    curr_attr = curr_attr->next;
  }

  /* parse childrens */
  curr_node = xml_node->children;
  while (curr_node) {
    if (curr_node->type == XML_ELEMENT_NODE) {
      const char * node_name;
      node_name = (const char *)curr_node->name;

      if (AIO_IS_EQ_CASE(node_name, "asset")) {

        _AIO_ASSET_LOAD_TO(curr_node,
                           pass->inf);

      } else if (AIO_IS_EQ_CASE(node_name, "annotate")) {
        aio_annotate * annotate;
        aio_annotate * last_annotate;
        int            ret;

        ret = aio_load_collada_annotate(curr_node, &annotate);

        if (ret == 0) {
          last_annotate = pass->annotate;
          if (last_annotate) {
            annotate->prev = last_annotate;
            last_annotate->next = annotate;
          } else {
            pass->annotate = annotate;
          }
        }

      } else if (AIO_IS_EQ_CASE(node_name, "states")) {
        aio_states * states;
        int          ret;

        states = NULL;
        ret = aio_load_fx_state(curr_node, &states);

        if (ret == 0)
          pass->states = states;

      } else if (AIO_IS_EQ_CASE(node_name, "program")) {
      } else if (AIO_IS_EQ_CASE(node_name, "evaluate")) {
      } else if (AIO_IS_EQ_CASE(node_name, "extra")) {

        _AIO_TREE_LOAD_TO(curr_node->children,
                          pass->extra,
                          NULL);
        
      }
    }
    
    curr_node = curr_node->next;
  }

  *dest = pass;

  return 0;
}