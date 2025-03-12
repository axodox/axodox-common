#include "common_includes.h"
#include "BitwiseOperations.h"

#ifdef WINRT_Windows_Foundation_H
using namespace winrt;
using namespace winrt::Windows::Foundation;
#endif

namespace Axodox::Infrastructure
{
#ifdef WINRT_Windows_Foundation_H
  bool are_equal(const winrt::Windows::Foundation::IInspectable& a, const winrt::Windows::Foundation::IInspectable& b)
  {
    {
      auto propA = a.try_as<IPropertyValue>();
      auto propB = b.try_as<IPropertyValue>();

      if (propA && propB)
      {
        if (propA.IsNumericScalar() && propB.IsNumericScalar())
        {
          return propA.GetUInt64() == propB.GetUInt64();
        }

        if (propA.Type() == propB.Type())
        {
          switch (propA.Type())
          {
          case PropertyType::UInt8:
            return propA.GetUInt8() == propB.GetUInt8();
          case PropertyType::Int16:
            return propA.GetInt16() == propB.GetInt16();
          case PropertyType::UInt16:
            return propA.GetUInt16() == propB.GetUInt16();
          case PropertyType::Int32:
            return propA.GetInt32() == propB.GetInt32();
          case PropertyType::UInt32:
            return propA.GetUInt32() == propB.GetUInt32();
          case PropertyType::Int64:
            return propA.GetInt64() == propB.GetInt64();
          case PropertyType::UInt64:
            return propA.GetUInt64() == propB.GetUInt64();
          case PropertyType::Boolean:
            return propA.GetBoolean() == propB.GetBoolean();
          case PropertyType::Single:
            return propA.GetSingle() == propB.GetSingle();
          case PropertyType::Double:
            return propA.GetDouble() == propB.GetDouble();
          case PropertyType::String:
            return propA.GetString() == propB.GetString();
          }
        }
      }
    }

    {
      auto stringA = a.try_as<IReference<hstring>>();
      auto stringB = b.try_as<IReference<hstring>>();
      if (stringA && stringB) return stringA.Value() == stringB.Value();
    }

    return a == b;
  }
#endif
}