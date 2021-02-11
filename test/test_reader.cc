#include <backends/bcBackendYamlCpp.h>
#include <bcModelTraits.h>
#include <fmt/format.h>
#include <fstream>

int main(int, char **) {
  std::ifstream istream(
      "/home/mersoj/code/boundary-conditions/test/syntax-test.yaml");
  auto model_traits = bc::ReadFromStream<bc::YAML_CPP>(istream);
  fmt::print("{}\n", *model_traits);
  return 0;
}
