#include "bcBackendYamlCpp.h"
#include "bcBackendYamlCppConvert.h"
#include "bcTypes.h"
#include <fmt/ostream.h>
#include <yaml-cpp/yaml.h>

namespace bc {

void ParseBoundaryConditions(const YAML::Node &yaml_node,
                             CategoryNode *parent_node) {
  for (auto &&bc : yaml_node) {
    auto name = bc["name"].as<std::string>();
    auto type = bc["type"].as<std::string>();
    auto geometry_vec = bc["geometry"].as<std::vector<int>>();
    std::unordered_set<int> geometry(std::begin(geometry_vec),
                                     std::end(geometry_vec));
    if (type == "scalar") {
      auto value = bc["value"].as<ScalarBC>();
      parent_node->AddBoundaryCondition(std::move(name), std::move(geometry),
                                        std::move(value));
    } else if (type == "bool") {
      auto value = bc["value"].as<BoolBC>();
      parent_node->AddBoundaryCondition(std::move(name), std::move(geometry),
                                        std::move(value));
    } else if (type == "string") {
      auto value = bc["value"].as<StringBC>();
      parent_node->AddBoundaryCondition(std::move(name), std::move(geometry),
                                        std::move(value));
    } else if (type == "vector") {
      // auto value = bc["value"].as<VectorBC>();
      fmt::print("matrix not converting yet skipping\n");
    } else if (type == "matrix") {
      // auto value = bc["value"].as<MatrixBC>();
      fmt::print("matrix not converting yet skipping\n");
    } else {
      throw std::runtime_error(
          fmt::format("Boundary condition type {} not implimented.", type));
    }
  }
}

void ParseCaseHelper(const YAML::Node &yaml_node, CategoryNode *parent_node) {
  for (auto &&c : yaml_node) {
    auto key = c.first.as<std::string>();
    // if(c.first.as<std::string>() == "boundary conditions")
    if (key == "boundary conditions") {
      ParseBoundaryConditions(c.second, parent_node);
    } else {
      ParseCaseHelper(c.second, parent_node->AddCategory(key));
    }
  }
}

void ParseCase(const YAML::Node &yaml_case, ModelTraits *model_traits) {
  auto model_case = model_traits->AddCase(yaml_case["name"].as<std::string>());
  auto &case_traits = yaml_case["model traits"];
  auto &geometry_type = case_traits["geometry_type"];
  if (geometry_type && geometry_type.as<std::string>() != "int") {
    throw std::runtime_error("only integer bc types are currently supported");
  }
  ParseCaseHelper(case_traits, model_case);
}

std::unique_ptr<ModelTraits> LoadFromYamlNode(const YAML::Node &base_node) {
  auto model_traits =
      std::make_unique<ModelTraits>(base_node["name"].as<std::string>());
  for (auto &cs : base_node["cases"]) {
    ParseCase(cs, model_traits.get());
  }
  return model_traits;
}

template <>
std::unique_ptr<ModelTraits> ReadFromStream<YAML_CPP>(std::istream &stream) {
  auto node = YAML::Load(stream);
  return LoadFromYamlNode(node["model traits"]);
}

} // namespace bc
