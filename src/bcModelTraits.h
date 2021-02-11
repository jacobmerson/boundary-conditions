#ifndef BC_MODEL_TRAITS_H__
#define BC_MODEL_TRAITS_H__
#include "bcCategoryNode.h"
#include "bcINode.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace bc {
class ModelTraits {
public:
  template <typename String,
            std::enable_if_t<std::is_convertible<String, std::string>::value,
                             int> = 0>
  ModelTraits(String &&name) : name_(std::forward<String>(name)) {}
  /**
   * Adds a case if it doesn't exist and returns a pointer
   * to it's INode. If the case does exist, the pointer to
   * the case is returned.
   */
  template <typename String,
            typename std::enable_if_t<
                std::is_convertible<String, std::string>::value, int> = 0>
  CategoryNode *AddCase(String &&name) {
    return static_cast<CategoryNode *>(cases_.AddNode(
        std::make_unique<CategoryNode>(std::forward<String>(name))));
  }
  CategoryNode *GetCase(const std::string &name) {
    return static_cast<CategoryNode *>(cases_.FindNode(name));
  }
  /**
   * Removes a case from ModelTraits by it's name
   */
  std::unique_ptr<CategoryNode> RemoveCase(const std::string &name) {
    // return std::move(static_cast<std::unique_ptr<CategoryNode>>();
    return std::move(std::unique_ptr<CategoryNode>(
        static_cast<CategoryNode *>(cases_.RemoveNode(name).release())));
  }
  int NumCases() { return cases_.Size(); }

  friend fmt::formatter<ModelTraits>;

protected:
  NodeSet<> cases_;
  std::string name_;
};

} // namespace bc

template <> struct fmt::formatter<bc::ModelTraits> {
  int level = 0;
  int spaces = 2;
  constexpr auto parse(format_parse_context &ctx) {
    return parse_mt_nodes(ctx, level, spaces);
  };
  template <typename FormatContext>
  auto format(const bc::ModelTraits &mt, FormatContext &ctx) {
    auto out =
        format_to(ctx.out(), "{:>{}}{}\n", "-", level * spaces, mt.name_);
    for (auto &cs : mt.cases_) {
      auto ptr = dynamic_cast<bc::CategoryNode *>(cs.get());
      auto fmt_string = fmt::format("{{:{}.{}t}}", level + 1, spaces);
      if (ptr) {
        out = format_to(out, fmt_string, *ptr);
      } else {
        out = format_to(out, fmt_string, *cs);
      }
    }
    return out;
  }
};
#endif
