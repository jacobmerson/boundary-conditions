#include "exprtk.hpp"
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <string>

using ScalarType = double;

template <typename T = ScalarType> class ExprtkFunction {
public:
  using symbol_table_t = exprtk::symbol_table<ScalarType>;
  using expression_t = exprtk::expression<ScalarType>;
  using parser_t = exprtk::parser<ScalarType>;
  ExprtkFunction(std::string eqn) {
    symbol_table_t symbol_table;
    symbol_table.add_variable("x", x_);
    symbol_table.add_variable("y", y_);
    symbol_table.add_variable("z", z_);
    symbol_table.add_variable("t", t_);
    symbol_table.add_constants();
    expression_.register_symbol_table(symbol_table);

    parser_t parser;
    if (!parser.compile(eqn, expression_)) {
      throw std::runtime_error("expression couldn't be compiled");
    }
  }

  ScalarType operator()(ScalarType x, ScalarType y, ScalarType z,
                        ScalarType t) {
    x_ = x;
    y_ = y;
    z_ = z;
    t_ = t;
    return expression_.value();
  }

private:
  // symbol_table_t symbol_table_{};
  expression_t expression_{};
  ScalarType x_{};
  ScalarType y_{};
  ScalarType z_{};
  ScalarType t_{};
};

auto myfunc(
    std::function<ScalarType(ScalarType, ScalarType, ScalarType, ScalarType)>
        f) {
  return f(1, 1, 1, 2);
}

int main(int, char **argv) {
  ExprtkFunction<> f("x^2+y");
  auto v = f(std::atof(argv[1]), std::atof(argv[2]), std::atof(argv[3]),
             std::atof(argv[4]));
  fmt::print("{}, {}\n", v, myfunc(f));

  /*
  //std::string expression_string = "z := x-(3*y)";
  std::string expression_string = "x-(3*y)";
  ScalarType x{123.456};
  ScalarType y{98.98};
  ScalarType z{0.0};


  expression_t expression;
  expression.register_symbol_table(symbol_table);

  parser_t parser;

  if(!parser.compile(expression_string, expression))
  {
    fmt::print("Compilation Error...\n");
    return 1;
  };

  auto result = expression.value();

  fmt::print("{}\n",result);
  fmt::print("{} {} {}\n",x,y,z);
  */
}
