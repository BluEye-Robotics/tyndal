#include <tyndall/reflect/reflect.h>
#include <string>
#include <typeinfo>

int main()
{

  {
    struct S
    {
      float floating_point;
      int integer;
      unsigned char unsigned_char = 5;
    };

    constexpr S s
    {
      .floating_point = 3.14f,
      .integer = 42,
      .unsigned_char = 255,
    };

    constexpr auto floating_point = reflect(s).get<0>();
    static_assert(floating_point == s.floating_point);

    constexpr auto integer = reflect(s).get<1>();
    static_assert(integer == s.integer);

    constexpr auto unsigned_char = reflect(s).get<2>();
    static_assert(unsigned_char == s.unsigned_char);

    constexpr size_t size = reflect(s).size();
    static_assert(size == 3);
  }

  {
    struct A{int a; int b;};

    struct S
    {
      float floating_point;
      int integer;
      std::string std_string;
      int another_integer;
      unsigned int unsigned_integer;
      unsigned char unsigned_char;
      char signed_char;
      A a;
    };

    S s;
    constexpr size_t size = reflect(s).size();
    static_assert(size == 8);

    static_assert(reflect(s).get_format() == "fiSijhcii"_strval);
  }

}
