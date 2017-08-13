#include "unittesting.h"

namespace unittesting
{
	TESTCLASS
	PUBLIC_REF_CLASS UnitTestingFramework TESTCLASSEXTENDS
	{
	private:
		TESTCONTEXT testContextInstance;

	public: 
		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
		TESTCONTEXTPROP

		#pragma region Additional test attributes
		//
		//You can use the following additional attributes as you write your tests:
		//
		//Use ClassInitialize to run code before running the first test in the class
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
		//
		//Use TestInitialize to run code before running each test
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//Use TestCleanup to run code after each test has run
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
		#pragma endregion 

		/**
		The each test method needs this decoration for the VS unit test execution.
		*/
		TESTMETHOD
		void TestFramework()
		{
			//http://blogs.msdn.com/b/jsocha/archive/2010/11/19/writing-unit-tests-in-visual-studio-for-native-c.aspx
			//Assert::AreEqual<int>(1,2);
			TEST_ASSERT(1==1)
			//
			// TODO: Add test logic	here
			//
		};

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		UnitTestingFramework()
		{
			TEST_ADD(UnitTestingFramework::TestFramework);
		}
	};
}

#ifndef _MSC_FULL_VER
#include "tests/algo/crcTest.cpp"
#include "tests/algo/endianTest.cpp"
#include "tests/algo/passwordTest.cpp"
#include "tests/lib/unicodeTest.cpp"
#include "tests/lib/stringUtilTest.cpp"
#endif

#pragma warning( push )
#pragma warning( disable : 4956 )
int main(int argc, char *argv[], char *envp[])
{
	MAINTESTDECL
	MAINADDTEST(new unittesting::UnitTestingFramework);
	MAINADDTEST(new CipherShed_Tests_Algo::PasswordTest);
	MAINADDTEST(new crc::CrcTest);
	MAINADDTEST(new CipherShed_Tests_Algo::EndianTest);
	MAINADDTEST(new CipherShed_Tests_lib::UnicodeTest);
	MAINADDTEST(new CipherShed_Tests_lib::StringUtilTest);
	MAINTESTRUN

}
#pragma warning( pop )

