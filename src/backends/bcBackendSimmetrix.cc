#include "bcBackendSimmetrix.h"
#include "bcBackendSimmetrixConvert.h"
#include "bcExprtkFunction.h"
#include "bcGeometrySet.h"
#include "bcSimWrapper.h"
#include <SimAttribute.h>
#include <SimError.h>
#include <SimModel.h>
#include <SimUtil.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <fmt/format.h>
#include <type_traits>

namespace bc {
enum class SimEquationType { Space, Time, SpaceTime, None };

template <typename T, int dim>
static void AddExpression(pAttInfo bc, CategoryNode *parent_node,
                          std::string &&name, DimGeometrySet<> &&geometry_set,
                          SimEquationType eqn_type) {
  switch (eqn_type) {
  case SimEquationType::Space:
    parent_node->AddBoundaryCondition(
        std::move(name), std::move(geometry_set),
        FunctionBC<T, 3, dim>::template from<SIMMETRIX>(bc));
    break;
  case SimEquationType::Time:
    parent_node->AddBoundaryCondition(
        std::move(name), std::move(geometry_set),
        FunctionBC<T, 1, dim>::template from<SIMMETRIX>(bc));
    break;
  case SimEquationType::SpaceTime:
    parent_node->AddBoundaryCondition(
        std::move(name), std::move(geometry_set),
        FunctionBC<T, 4, dim>::template from<SIMMETRIX>(bc));
    break;
  case SimEquationType::None:
    parent_node->AddBoundaryCondition(
        std::move(name), std::move(geometry_set),
        GenericBC<T, dim>::template from<SIMMETRIX>(bc));
    break;
  }
}

static SimEquationType GetEquationType(const std::string &expression) {
  using std::find;
  bool has_spatial = false;
  bool has_temporal = false;
  auto it_end = end(expression);
  auto it = find(begin(expression), it_end, '$');
  while (it != it_end) {
    auto var = *++it;
    if (var == 'x' || var == 'y' || var == 'z') {
      has_spatial = true;
    } else if (var == 't') {
      has_temporal = true;
    } else {
      throw std::runtime_error(fmt::format(
          "Unexpected variable ({}) in Simmetrix equation. Expected x,y,z,t.",
          var));
    }
    it = find(it, it_end, '$');
  }
  if (has_spatial && has_temporal) {
    return SimEquationType::SpaceTime;
  }
  if (has_spatial) {
    return SimEquationType::Space;
  }
  if (has_temporal) {
    return SimEquationType::Time;
  }
  return SimEquationType::None;
}

static SimEquationType
GetEquationType(const std::vector<std::string> &expressions) {
  bool has_spatial = false;
  bool has_temporal = false;
  for (const auto &eqn : expressions) {
    auto equation_type = GetEquationType(eqn);
    if (equation_type == SimEquationType::Space) {
      has_spatial = true;
    } else if (equation_type == SimEquationType::Time) {
      has_temporal = true;
    }
    if (equation_type == SimEquationType::SpaceTime ||
        (has_spatial && has_temporal)) {
      return SimEquationType::SpaceTime;
    }
  }
  // if has space && time already returned in loop
  if (has_spatial) {
    return SimEquationType::Space;
  }
  if (has_temporal) {
    return SimEquationType::Time;
  }
  return SimEquationType::None;
}

static DimGeometrySet<> GetGeomFromModelAssoc(pModelAssoc ma) {
  if (ma == nullptr) {
    return DimGeometrySet<>{};
  }
  SimList<pModelItem> model_items = AMA_modelItems(ma);
  std::vector<DimGeometry> geom;
  geom.reserve(model_items.Size());
  for (auto &model_item : model_items) {
    auto item_type = ModelItem_type(model_item);
    // if this static assert fails, Simmetrix has changed their Gtype enums, and
    // any input fies generated based on this function will be broken
    static_assert(Gvertex == 0 && Gedge == 1 && Gface == 2 && Gregion == 3 &&
                      Gmodel == 9,
                  "gType enums must convert to expected dimensions.");
    if (ModelItem_isGEntity(model_item) != 0) {
      geom.emplace_back(item_type, GEN_tag(static_cast<pGEntity>(model_item)));
    } else {
      fmt::print("Adding model items that are not geometric entities is not "
                 "supported\n");
    }
  }
  // return std::make_unique<DimGeometrySet<>>(geom.begin(), geom.end());
  return {geom.begin(), geom.end()};
}

static void AddBoundaryCondition(CategoryNode *parent_node, pAttInfo sim_node,
                                 pModelAssoc ma) {
  auto rep_type = AttNode_repType(static_cast<pANode>(sim_node));
  SimString name = AttNode_infoType(static_cast<pANode>(sim_node));
  auto geom = GetGeomFromModelAssoc(ma);
  if (rep_type == Att_int) {
    if (AttInfoInt_isExpression(static_cast<pAttInfoInt>(sim_node)) != 0) {
      SimString e = AttInfoInt_expression(static_cast<pAttInfoInt>(sim_node));
      auto exp = std::string(e);
      auto equation_type = GetEquationType(exp);
      AddExpression<OrdinalType, 0>(sim_node, parent_node, std::string(name),
                                    std::move(geom), equation_type);
    } else {
      auto value = IntBC::from<SIMMETRIX>(sim_node);
      parent_node->AddBoundaryCondition(std::string(name), std::move(geom),
                                        std::move(value));
    }
  } else if (rep_type == Att_string) {
    if (AttInfoString_isExpression(static_cast<pAttInfoString>(sim_node)) !=
        0) {
      throw std::runtime_error("String Expressions Not Currently Supported");
    }
    auto value = StringBC::from<SIMMETRIX>(sim_node);
    parent_node->AddBoundaryCondition(std::string(name), std::move(geom),
                                      std::move(value));
  } else if (rep_type == Att_double) {
    if (AttInfoDouble_isExpression(static_cast<pAttInfoDouble>(sim_node)) !=
        0) {
      SimString e =
          AttInfoDouble_expression(static_cast<pAttInfoDouble>(sim_node));
      auto exp = std::string(e);
      auto equation_type = GetEquationType(exp);
      AddExpression<ScalarType, 0>(sim_node, parent_node, std::string(name),
                                   std::move(geom), equation_type);
    } else {
      auto value = ScalarBC::from<SIMMETRIX>(sim_node);
      parent_node->AddBoundaryCondition(std::string(name), std::move(geom),
                                        std::move(value));
    }
  } else if (rep_type == Att_tensor0) {
    if (AttInfoTensor0_isExpression(static_cast<pAttInfoTensor0>(sim_node)) !=
        0) {
      SimString e =
          AttInfoTensor0_expression(static_cast<pAttInfoTensor0>(sim_node));
      auto exp = std::string(e);
      auto equation_type = GetEquationType(exp);
      AddExpression<ScalarType, 0>(sim_node, parent_node, std::string(name),
                                   std::move(geom), equation_type);
    } else {
      auto value = ScalarBC::from<SIMMETRIX>(sim_node);
      parent_node->AddBoundaryCondition(std::string(name), std::move(geom),
                                        std::move(value));
    }
  } else if (rep_type == Att_tensor1) {
    auto *tensor_att = static_cast<pAttInfoTensor1>(sim_node);
    if (AttInfoTensor1_hasComponentExpressions(tensor_att) != 0) {
      // put the equations into a vector
      auto size = AttInfoTensor1_dimension(tensor_att);
      std::vector<std::string> equations;
      equations.reserve(size);
      for (int i = 0; i < size; ++i) {
        if (AttInfoTensor1_isExpression(tensor_att, i) != 0) {
          SimString eqn = AttInfoTensor1_expression(tensor_att, i);
          equations.emplace_back(eqn);
        } else {
          equations.push_back(
              std::to_string(AttInfoTensor1_value(tensor_att, i)));
        }
      }
      auto equation_type = GetEquationType(equations);
      AddExpression<ScalarType, 1>(sim_node, parent_node, std::string(name),
                                   std::move(geom), equation_type);
    } else {
      auto value = VectorBC::from<SIMMETRIX>(sim_node);
      parent_node->AddBoundaryCondition(std::string(name), std::move(geom),
                                        std::move(value));
    }
  } else if (rep_type == Att_tensor2) {
    auto *tensor_att = static_cast<pAttInfoTensor2>(sim_node);
    SimEquationType equation_type;
    bool is_expression = [&]() {
      bool isExpression = false;
      int dim = AttInfoTensor2_dimension(tensor_att);
      std::vector<std::string> equations;
      for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
          if (AttInfoTensor2_isExpression(tensor_att, i, j) != 0) {
            equations.emplace_back(
                SimString(AttInfoTensor2_expression(tensor_att, i, j)));
            isExpression = true;
          }
        }
      }
      equation_type = GetEquationType(equations);
      return isExpression;
    }();
    if (is_expression) {
      AddExpression<ScalarType, 2>(sim_node, parent_node, std::string(name),
                                   std::move(geom), equation_type);
    }
    auto value = MatrixBC::from<SIMMETRIX>(sim_node);
    parent_node->AddBoundaryCondition(std::string(name), std::move(geom),
                                      std::move(value));
  } else {
    fmt::print("Cannot add a boundary condition with the AttRepType of {}. "
               "Skipping it.\n",
               rep_type);
  }
}

/*
 * Returns true if the simmetrix AttRepType should
 * be used as a BC node in model traits
 */
static bool AttRepTypeIsBC(AttRepType rep_type) {
  return (rep_type == Att_int || rep_type == Att_string ||
          rep_type == Att_double || rep_type == Att_tensor0 ||
          rep_type == Att_tensor1 || rep_type == Att_tensor2);
}
/*
 * Returns true if the simmetrix AttRepType should
 * be used as a category node in model traits
 */
static bool AttRepTypeIsCategory(AttRepType rep_type) {
  return (rep_type == Att_void || rep_type == Att_case);
}

// the simmetrix attribute node should be used as a const node
// but since the simmetrix api doesn't take const AttNode*, we
// cannot explicitly use that definition here
static void ParseNode(CategoryNode *parent_node, pANode sim_node, pACase cs,
                      pModelAssoc model_association) {
  auto rep_type = AttNode_repType(sim_node);
  SimString name = AttNode_infoType(sim_node);
  if (rep_type == Att_case) {
    // get the model from the parent case and set it on the new case node
    // if the model is not set on the case, then various functions such as
    // AMA_modelItems exhibit undefined behaviors
    auto *model = AttCase_loadedModel(cs);
    cs = static_cast<pACase>(sim_node);
    AttCase_setModel(cs, model);
  }
  SimList<pModelAssoc> model_associations =
      AttCase_findModelAssociations(cs, sim_node);
  if (model_associations.Size() > 1) {
    throw std::runtime_error(
        "Only expect simmetrix nodes to have a single model association");
  }
  if (model_associations.Size() == 1) {
    model_association = model_associations[0];
  }
  if (AttRepTypeIsBC(rep_type)) {
    AddBoundaryCondition(parent_node, static_cast<pAttInfo>(sim_node),
                         model_association);
  } else if (AttRepTypeIsCategory(rep_type)) {
    parent_node = parent_node->AddCategory(std::string(name));
  } else {
    throw std::runtime_error("Unsupported AttRepType");
  }
  // process children
  SimList<pANode> children = AttNode_children(sim_node);
  for (auto *child : children) {
    ParseNode(parent_node, child, cs, model_association);
  }
}

// TODO, ReadFromFile should also take an instance of backend,
// and should allow the user to choose a "root node" for the attributes
template <>
std::unique_ptr<ModelTraits>
ReadFromFile<SIMMETRIX>(const std::string &filename) {
  try {
    SimmodelerLog lg("reader.log");
    SimmodelerStartup s;
    SimmodelerLicenses l(s);
    SimmodelerParasolid ps;

    SimProgress progress;

    SimGModel model = GM_load(filename.c_str(), nullptr, progress);
    // since the attribute manager comes from the model it will
    // be deleted when the model gets deleted. For this reason,
    // we do not use an RAII wrapper for this instance of the Att manager
    auto *manager = GM_attManager(model);

    SimList<pANode> cases = AMAN_cases(manager);
    SimString last_image_class;
    std::unique_ptr<ModelTraits> mt;
    // take the cs by value since it is a pointer type
    // auto cs = (pACase)cases[3];
    for (auto *cs : cases) {
      auto num_parents = AttNode_NumberOfParents(cs);
      SimString info_type = AttNode_infoType(cs);
      //// root cases have no parents
      if (num_parents == 0 && std::strcmp(info_type, "Meshing") != 0) {
        SimString image_class = AttNode_imageClass(cs);
        // construct the model traits object on the first root node
        if (last_image_class == nullptr) {
          mt = std::make_unique<ModelTraits>(std::string(image_class));
        }
        // assume that all of the cases are part of the same image class
        else if (std::strcmp(image_class, last_image_class) != 0) {
          throw std::runtime_error(
              "all root cases must share the same image class");
        }
        SimString name = AttNode_name(cs);
        auto reptype = AttNode_repType(cs);
        assert(reptype == Att_case);
        auto *mt_case = mt->AddCase(std::string(name));
        last_image_class = std::move(image_class);
        SimList<pANode> children = AttNode_children(cs);
        AttCase_setModel(static_cast<pACase>(cs), model);
        SimList<pModelAssoc> model_associations =
            AttCase_findModelAssociations(static_cast<pACase>(cs), cs);
        if (model_associations.Size() > 1) {
          throw std::runtime_error(
              "Only expect simmetrix nodes to have a single model association");
        }
        pModelAssoc model_association =
            model_associations.Size() == 0 ? nullptr : model_associations[0];
        for (auto *child : children) {
          ParseNode(mt_case, child, static_cast<pACase>(cs), model_association);
        }
      }
    }
    return mt;
  } catch (pSimError err) {
    fmt::print("SimModSuite error caught:\n");
    fmt::print("\tError code: {}\n", SimError_code(err));
    fmt::print("\tError string: {}\n", SimError_toString(err));
    SimError_delete(err);
  } catch (std::exception &e) {
    fmt::print("Exception: {}\n", e.what());
  } catch (...) {
    fmt::print("unhandled exception\n");
  }

  return {};
}
} // namespace bc
