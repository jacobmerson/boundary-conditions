#ifndef BC_NODE_SET_H__
#define BC_NODE_SET_H__
#include "bcINode.h"
#include <algorithm>
#include <fmt/format.h>
#include <memory>
#include <type_traits>

namespace bc {

/*
 * This is here so that we can use the template, template parameters
 * with the defaults for vector and unordered map. This workaround
 * is not needed in c++17 and up.
 */
template <typename T> using BC_VEC_WORKAROUND = std::vector<T>;
/*
 * The node set collection is templated because depending on the number of nodes
 * within each level, performance may be significantly better by using a vector,
 * or some other container like unordered_set. A generic implimentation is
 * provided, and an unordered set implimentation is provided, but if you want to
 * use some other type of container that doesn't play nice with the std
 * algorithms, it's reccomended that you specialize the add, find, and remove
 * functions
 */
template <template <typename> class Cont = BC_VEC_WORKAROUND> class NodeSet {
public:
  /*
   * Add a new node to the set.
   */
  using Container = Cont<std::unique_ptr<INode>>;
  // the default implementation works for containers with push_back
  INode *AddNode(std::unique_ptr<INode> node) {
    if (!FindNode(node->GetName())) {
      nodes_.push_back(std::move(node));
      return nodes_.back().get();
    }
    throw std::runtime_error(fmt::format(
        "Node cannot be added because node with name {} already exists.\n",
        node->GetName()));
  }
  /*
   * A node with the given name and returns the (unique_ptr) pointer.
   * Note that this removes the whole tree attached to the node as well.
   * I.e., it does not relink any nodes that the named node contains
   */
  std::unique_ptr<INode> RemoveNode(const std::string &name) {
    auto it = FindNodeIt(name);
    // return a nullptr if the node doesn't exist;
    if (it == std::end(nodes_)) {
      return std::unique_ptr<INode>(nullptr);
    }
    // grab the pointer that we will erase from the container
    auto ptr = std::move(*it);
    nodes_.erase(it);
    return ptr;
  };
  /*
   * Get a non-owning pointer to the node with the given name
   */
  INode *FindNode(const std::string &name) {
    auto it = FindNodeIt(name);
    if (it == std::end(nodes_))
      return nullptr;
    return it->get();
  };
  int Size() { return nodes_.size(); }
  // iterators
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;
  iterator begin() noexcept { return nodes_.begin(); }
  const_iterator begin() const noexcept { return nodes_.begin(); }
  iterator end() noexcept { return nodes_.end(); }
  const_iterator end() const noexcept { return nodes_.end(); }

private:
  auto FindNodeIt(const std::string &name) {
    return std::find_if(std::begin(nodes_), std::end(nodes_),
                        [&name](auto &nd) { return nd->GetName() == name; });
  }
  Container nodes_;
};

} // namespace bc

#endif
