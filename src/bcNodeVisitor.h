#ifndef BC_NODE_VISITOR_H__
#define BC_NODE_VISITOR_H__
#include "bcTypedefs.h"
#include "bcTypes.h"

namespace bc {
struct CategoryNode;
template <typename, typename> struct BCNode;

struct NodeVisitor {
  virtual void visit(CategoryNode &) = 0;
  virtual void visit(BCNode<OrdinalType, BoolBC> &) = 0;
  virtual void visit(BCNode<OrdinalType, MatrixBC> &) = 0;
  virtual void visit(BCNode<OrdinalType, ScalarBC> &) = 0;
  virtual void visit(BCNode<OrdinalType, StringBC> &) = 0;
  virtual void visit(BCNode<OrdinalType, VectorBC> &) = 0;
  virtual ~NodeVisitor() = default;
};

} // namespace bc

#endif
