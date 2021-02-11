#ifndef BC_CATEGORY_NODE_H__
#define BC_CATEGORY_NODE_H__
#include "bcBCNode.h"
#include "bcConvert.h"
#include "bcINode.h"
#include "bcNodeSet.h"
#include "bcTypes.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace bc {

class CategoryNode : public INode, public ConvertFunctions<CategoryNode> {
public:
  template <typename String,
            typename std::enable_if_t<
                std::is_convertible<String, std::string>::value, int> = 0>
  CategoryNode(String &&name) : INode(std::forward<String>(name)) {}
  template <typename String,
            typename std::enable_if_t<
                std::is_convertible<String, std::string>::value, int> = 0>
  CategoryNode *AddCategory(String &&name) {
    return static_cast<CategoryNode *>(nodes_.AddNode(
        std::make_unique<CategoryNode>(std::forward<String>(name))));
  }
  template <typename BC, typename GID = OrdinalType, typename GSet,
            typename String,
            typename std::enable_if_t<
                std::is_convertible<String, std::string>::value, int> = 0>
  BCNode<GID, BC> *AddBoundaryCondition(String &&name, GSet &&geometry,
                                        BC &&boundary_condition) {
    return static_cast<BCNode<GID, BC> *>(
        nodes_.AddNode(std::make_unique<BCNode<GID, BC>>(
            std::forward<String>(name), std::forward<GSet>(geometry),
            std::forward<BC>(boundary_condition))));
  }

  virtual void accept(NodeVisitor &v) final { v.visit(*this); }

  friend fmt::formatter<CategoryNode>;

protected:
  NodeSet<> nodes_;
};
} // namespace bc
template <> struct fmt::formatter<bc::CategoryNode> {
  int level = 0;
  int spaces = 2;
  constexpr auto parse(format_parse_context &ctx) {
    return parse_mt_nodes(ctx, level, spaces);
  };
  template <typename FormatContext>
  auto format(const bc::CategoryNode &cn, FormatContext &ctx) {
    auto out = format_to(ctx.out(), "{:>{}}{}", "-", level * spaces, cn.name_);
    for (auto &nd : cn.nodes_) {
      auto CN_ptr = dynamic_cast<bc::CategoryNode *>(nd.get());
      auto fmt_string = fmt::format("\n{{:{}.{}t}}", level + 1, spaces);
      if (CN_ptr) {
        out = format_to(out, fmt_string, *CN_ptr);
      } else {
        out = format_to(out, fmt_string, *nd);
      }
    }
    return out;
  }
};
#endif
