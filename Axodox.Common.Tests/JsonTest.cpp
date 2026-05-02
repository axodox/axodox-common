#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Json.h"

using namespace Axodox::Json;
using namespace Axodox::Infrastructure;
using namespace std;

named_enum(test_enum, a, b, c);

struct test
{
  static json_object_descriptor<test> json_description;

  virtual ~test() = default;

  vector<test_enum> ve = { test_enum::a };
  float x = 1, y = 2, z = 3;
};

auto test::json_description = describe_json_object<test>("test", "some desc", {
    { &test::x, "x", {.description = "asd" } },
    { &test::y, "y" },
    { &test::z, "z", {.description = "asd", .minimum = 5 } },
    { &test::ve, "ve", {.description = "asd" } }
  });

struct test2 : public test
{
  bool b = true;

  static json_object_descriptor<test2> json_description;
};

auto test2::json_description = describe_json_object<test2, test>("test2", "some other desc", {
  { &test2::b, "b", {.description = "asd" } }
  });

struct test3 : public test2
{
  int c = 5;

  static json_object_descriptor<test3> json_description;
};

auto test3::json_description = describe_json_object<test3, test2>("test3", "deepest desc", {
  { &test3::c, "c", {.description = "asd" } }
  });

namespace Axodox::Common::Tests
{
  TEST_CLASS(ExperimentalTest)
  {
    TEST_METHOD(Test)
    {
      auto result = test3::json_description.to_json();

      auto o = make_value<json_object>();
      test x;
      auto y = &x;
      auto z = &y->ve;
      auto ww = &test::ve;
      OutputDebugStringA(format("main y{} z{}\n", (void*)y, (void*)z).c_str());
      test::json_description.to_json(x, o.get());

      auto text0 = result->to_string();
      auto text1 = stringify_json(test3{});
      auto text2 = stringify_json<value_ptr<test>>(make_value<test2>());
      auto value = try_parse_json<value_ptr<test>>(R"({"$type": "test2", "x": 6, "b": false, "ve": ["a"]})");
    }
  };
}