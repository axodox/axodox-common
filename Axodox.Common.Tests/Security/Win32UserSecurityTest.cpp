#include "common_includes.h"
#include "Include/Axodox.Security.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Security;
using namespace std;

namespace Axodox::Common::Tests
{
  TEST_CLASS(Win32UserSecurityTests)
  {
  public:
    TEST_METHOD(TestAccessIsLimitedToCurrentUser)
    {
      //The current user's SID
      winrt::handle token;
      winrt::check_bool(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, token.put()));
      DWORD size = 0;
      GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);
      vector<uint8_t> tokenUser(size);
      winrt::check_bool(GetTokenInformation(token.get(), TokenUser, tokenUser.data(), size, &size));
      auto userSid = reinterpret_cast<TOKEN_USER*>(tokenUser.data())->User.Sid;

      //The access list has a single entry, allowing the current user, and nothing is inherited
      win32_user_security security;
      auto descriptor = security.attributes()->lpSecurityDescriptor;

      BOOL isPresent = FALSE, isDefaulted = FALSE;
      PACL dacl = nullptr;
      winrt::check_bool(GetSecurityDescriptorDacl(descriptor, &isPresent, &dacl, &isDefaulted));
      Assert::IsTrue(isPresent && dacl, L"there must be an access list");
      Assert::AreEqual(WORD(1), dacl->AceCount);

      ACCESS_ALLOWED_ACE* ace = nullptr;
      winrt::check_bool(GetAce(dacl, 0, reinterpret_cast<void**>(&ace)));
      Assert::AreEqual(BYTE(ACCESS_ALLOWED_ACE_TYPE), ace->Header.AceType);
      Assert::IsTrue(EqualSid(&ace->SidStart, userSid) != FALSE, L"the entry must be for the current user");

      SECURITY_DESCRIPTOR_CONTROL control = 0;
      DWORD revision = 0;
      winrt::check_bool(GetSecurityDescriptorControl(descriptor, &control, &revision));
      Assert::IsTrue((control & SE_DACL_PROTECTED) != 0, L"the access list must not inherit entries");
    }
  };
}
