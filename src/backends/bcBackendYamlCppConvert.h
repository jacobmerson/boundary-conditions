#ifndef BC_BACKEND_YAML_CPP_CONVERT_H__
#define BC_BACKEND_YAML_CPP_CONVERT_H__
#include "bcBackendYamlCpp.h"
#include "bcConvert.h"
#include "bcTypes.h"

// conversions from yaml to/from the BC datatypes
namespace YAML {
template <> struct convert<bc::BoolBC> {
  static Node encode(const bc::BoolBC &rhs) {
    return convert<bool>::encode(rhs);
  }
  static bool decode(const Node &node, bc::BoolBC &rhs) {
    return convert<bool>::decode(node, rhs);
  }
};
template <> struct convert<bc::ScalarBC> {
  static Node encode(const bc::ScalarBC &rhs) {
    return convert<double>::encode(rhs);
  }
  static bool decode(const Node &node, bc::ScalarBC &rhs) {
    return convert<double>::decode(node, rhs);
  }
};
template <> struct convert<bc::StringBC> {
  static Node encode(const bc::StringBC &rhs) {
    return convert<std::string>::encode(rhs);
  }
  static bool decode(const Node &node, bc::StringBC &rhs) {
    return convert<std::string>::decode(node, rhs);
  }
};
} // namespace YAML

namespace bc {}

#endif
