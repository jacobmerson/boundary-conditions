#ifndef BC_INTERFACE_NODE_H__
#define BC_INTERFACE_NODE_H__
#include "bcNodeVisitor.h"
#include "fmt/format.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <type_traits>

namespace bc {

class INode {
public:
  template <typename String,
            std::enable_if_t<std::is_convertible<String, std::string>::value,
                             int> = 0>
  INode(String &&name) : name_(std::forward<String>(name)) {}
  virtual bool IsBoundaryCondition() const noexcept { return false; }
  const std::string &GetName() const noexcept { return name_; }
  // copying is disabled because this is a base class and should
  // be passed around with pointer semantics, not by value
  INode(const INode &) = delete;
  INode &operator=(const INode &) = delete;
  // default all the operations since we need a virtual destructor
  INode(INode &&) noexcept = default;
  INode &operator=(INode &&) = default;
  virtual ~INode() = default;

  virtual void accept(NodeVisitor &v) = 0;

protected:
  std::string name_;
};

} // namespace bc

constexpr auto parse_mt_nodes(fmt::format_parse_context &ctx, int &level,
                              int &spaces) {
  auto it = ctx.begin(), end = ctx.end();
  if (it == end) {
    return it;
  }
  char *int_end = nullptr;
  level = std::strtol(it, &int_end, 10);
  std::advance(it, int_end - it);
  if (*it == '.') {
    spaces = std::strtol(++it, &int_end, 10);
    std::advance(it, int_end - it);
  }
  if (*it != 't') {
    throw fmt::format_error("must use t format specifier with tab levels");
  }
  ++it;
  if (it != end && *it != '}') {
    throw fmt::format_error("invalid format");
  }
  return it;
}
template <typename T>
struct fmt::formatter<
    T, std::enable_if_t<std::is_base_of<bc::INode, T>::value, char>> {
  int level = 0;
  int spaces = 2;
  constexpr auto parse(format_parse_context &ctx) {
    return parse_mt_nodes(ctx, level, spaces);
  };
  template <typename FormatContext>
  auto format(const T &node, FormatContext &ctx) {
    auto marker = node.IsBoundaryCondition() ? "+" : "-";
    return format_to(ctx.out(), "{:>{}}{}", marker, level * spaces,
                     node.GetName());
  }
};
#endif
