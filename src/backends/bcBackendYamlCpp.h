#ifndef BC_BACKEND_YAML_CPP_H__
#define BC_BACKEND_YAML_CPP_H__
#include "bcModelTraits.h"
#include "bcModelTraitsIO.h"

namespace bc {
struct YAML_CPP {};

template <>
void WriteToStream<YAML_CPP>(const ModelTraits *model_traits,
                             /*
                             template <>
                             void WriteToStream<YAML_CPP>(const ModelTraits
                             *model_traits, std::ostream &stream) {}
                             */
                             std::ostream &stream);
template <>
std::unique_ptr<ModelTraits> ReadFromStream<YAML_CPP>(std::istream &stream);
} // namespace bc

#endif
