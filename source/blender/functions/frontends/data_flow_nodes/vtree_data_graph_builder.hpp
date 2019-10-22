#pragma once

#include "FN_core.hpp"
#include "BKE_virtual_node_tree_cxx.h"
#include "BLI_string_map.h"

#include "vtree_data_graph.hpp"
#include "mappings.hpp"

struct ID;
struct PointerRNA;

namespace FN {
namespace DataFlowNodes {

using BKE::VirtualNode;
using BKE::VirtualNodeTree;
using BKE::VirtualSocket;

class VTreeDataGraphBuilder {
 private:
  VirtualNodeTree &m_vtree;
  Vector<BuilderSocket *> m_socket_map;
  Vector<Type *> m_type_by_vsocket;
  std::unique_ptr<TypeMappings> &m_type_mappings;
  Vector<BuilderNode *> m_placeholder_nodes;
  DataGraphBuilder m_graph_builder;

 public:
  VTreeDataGraphBuilder(VirtualNodeTree &vtree);

  std::unique_ptr<VTreeDataGraph> build();

  Vector<BuilderSocket *> &socket_map()
  {
    return m_socket_map;
  }

  BuilderNode *insert_function(Function &fn);
  BuilderNode *insert_matching_function(Function &fn, const VirtualNode &vnode);
  BuilderNode *insert_function(Function &fn, const VirtualNode &vnode);
  BuilderNode *insert_placeholder(const VirtualNode &vnode);

  template<typename T> void add_resource(std::unique_ptr<T> resource, const char *name)
  {
    m_graph_builder.add_resource(std::move(resource), name);
  }

  void insert_link(BuilderOutputSocket *from, BuilderInputSocket *to);
  void insert_links(ArrayRef<BuilderOutputSocket *> from, ArrayRef<BuilderInputSocket *> to);

  void map_input_socket(BuilderInputSocket *socket, const VirtualSocket &vsocket);
  void map_output_socket(BuilderOutputSocket *socket, const VirtualSocket &vsocket);
  void map_sockets(BuilderNode *node, const VirtualNode &vnode);
  void map_data_sockets(BuilderNode *node, const VirtualNode &vnode);

  BuilderInputSocket *lookup_input_socket(const VirtualSocket &vsocket);
  BuilderOutputSocket *lookup_output_socket(const VirtualSocket &vsocket);
  bool is_input_unlinked(const VirtualSocket &vsocket);
  bool verify_data_sockets_mapped(const VirtualNode &vnode) const;

  Type *type_by_name(StringRef data_type) const;
  VirtualNodeTree &vtree() const;
  bool is_data_socket(const VirtualSocket &vsocket) const;
  Type *query_socket_type(const VirtualSocket &vsocket) const;
  Type *query_type_property(const VirtualNode &vnode, StringRefNull prop_name) const;
  bool has_data_socket(const VirtualNode &vnode) const;
  Type *type_from_rna(PointerRNA &rna, StringRefNull prop_name) const;
  ArrayRef<BuilderNode *> placeholder_nodes();

  std::string to_dot();
  void to_dot__clipboard();

 private:
  void initialize_type_by_vsocket_map();
  bool check_if_sockets_are_mapped(const VirtualNode &vnode,
                                   ArrayRef<const VirtualSocket *> vsockets) const;
};

}  // namespace DataFlowNodes
}  // namespace FN
