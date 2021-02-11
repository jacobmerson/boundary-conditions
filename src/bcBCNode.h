#ifndef BC_BC_NODE_H__
#define BC_BC_NODE_H__
#include "bcINode.h"
#include <unordered_set>

namespace bc {
template <typename GID, typename BC> class BCNode : public INode {
public:
  virtual bool IsBoundaryCondition() const noexcept final { return true; }
  template <typename String,
            std::enable_if_t<std::is_convertible<String, std::string>::value,
                             int> = 0>
  BCNode(String &&name) : INode(std::forward<String>(name)) {}
  template <typename GSet, typename String,
            std::enable_if_t<std::is_convertible<String, std::string>::value,
                             int> = 0>
  BCNode(String &&name, GSet &&g, BC &&bc)
      : INode(std::forward<String>(name)),
        boundary_condition_(std::forward<BC>(bc)),
        geometry_id_(std::forward<GSet>(g)) {}

  virtual void accept(NodeVisitor &v) final { v.visit(*this); }

private:
  BC boundary_condition_;
  // we may actually need to store the id and the dimension
  // need to verify how geometric id is typically reported
  std::unordered_set<GID> geometry_id_;
};
} // namespace bc

#endif
