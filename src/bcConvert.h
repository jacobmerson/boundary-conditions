#ifndef BC_CONVERT_H__
#define BC_CONVERT_H__
namespace bc {

template <typename T, typename Backend> struct convert;

template <typename BaseClass> struct ConvertFunctions {
  template <typename T> T to() const {
    return convert<BaseClass, T>::decode(static_cast<const BaseClass &>(*this));
  }
  template <typename T> static BaseClass from(const T &other) {
    return convert<BaseClass, T>::encode(other);
  }
};

} // namespace bc

// adding new types to the readers/writers just requires writing a
// convert struct with the appropriate types.
// for examples look at the various backends

#endif
