#include "bcBCNode.h"
#include "bcCategoryNode.h"
#include "bcTypedefs.h"
#include "bcTypes.h"
#include "catch2/catch_test_macros.hpp"
// boundary condition struct
// struct BC {};
using BC = bc::BoolBC;

TEST_CASE("add to category node", "[node]") {
  bc::CategoryNode cn{"solution strategy"};
  REQUIRE(cn.GetName() == "solution strategy");
  bc::INode *node1 = nullptr;
  bc::INode *node2 = nullptr;
  SECTION("Category First") {
    REQUIRE_NOTHROW(node1 = cn.AddCategory("category 2"));
    REQUIRE_NOTHROW(node2 = cn.AddCategory("category 3"));
    REQUIRE(node1 != node2);
    // trying to add two category nodes with the same name throws
    REQUIRE_THROWS(node2 = cn.AddCategory("category 2"));
    // cannot add a bc with the same name as the category
    REQUIRE_THROWS(cn.AddBoundaryCondition("category 2", 1, BC{}));

    REQUIRE_NOTHROW(node1 = cn.AddBoundaryCondition("bc 1", 1, BC{}));
    REQUIRE_NOTHROW(node2 = cn.AddBoundaryCondition("bc 2", 1, BC{}));
    REQUIRE(node1 != node2);
    REQUIRE_THROWS(node2 = cn.AddBoundaryCondition("bc 1", 1, BC{}));
  }
  SECTION("bc first") {
    REQUIRE_NOTHROW(cn.AddBoundaryCondition("bc 1", 1, BC{}));
    REQUIRE_NOTHROW(cn.AddBoundaryCondition("bc 2", 1, BC{}));
    REQUIRE_THROWS(cn.AddBoundaryCondition("bc 2", 1, BC{}));
    REQUIRE_NOTHROW(cn.AddBoundaryCondition("category 2", 1, BC{}));

    REQUIRE_THROWS(cn.AddCategory("category 2"));
    REQUIRE_NOTHROW(cn.AddCategory("category 3"));
  }
}
