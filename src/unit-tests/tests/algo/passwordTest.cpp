#include "../../unittesting.h"

#include "../../faux/windows/HWND.h"
#include "../../faux/windows/BOOL.h"
#include "../../../Common/Password.h"
#include <string.h>

#include <stdio.h>

#pragma warning(disable: 4996)
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct TS
{
	char* s;
	int r;
} TS;
namespace CipherShed_Tests_Algo
{
	TESTCLASS
	PUBLIC_REF_CLASS PasswordTest TESTCLASSEXTENDS
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
		void testPasswordTrue()
		{
			TEST_ASSERT(1==1);
		};

		TESTMETHOD
		void testPasswordCheckPasswordCharEncodingStruct()
		{
			TS tests[]=
			{
				{"",1},
				{"a",1},
				{"abcdefg",1},
				{"\n",0},
				{"1234567890123456789012345678901234567890123456789012345678901234",1},
				{"12345678901234567890123456789012345678901234567890123456789012345",0},
			};

			int len=sizeof(tests)/sizeof(tests[0]);

			for (int i=0; i<len; ++i)
			{
				Password pwd;
				strcpy((char*)pwd.Text,tests[i].s);
				pwd.Length=strlen(tests[i].s);
				BOOL res=CheckPasswordCharEncoding(NULL, &pwd);
				char msg[2048];
				sprintf(msg,"i:%d test str <%s> expected:%d, got %d",i,tests[i].s, tests[i].r,res);
				TEST_ASSERT_MSG(res==tests[i].r,msg);
			}

		};

		TESTMETHOD
		void testPasswordCheckPasswordCharEncodingHandle()
		{
			TEST_ASSERT(1==1);
		};

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		PasswordTest()
		{
			TEST_ADD(PasswordTest::testPasswordTrue);
			TEST_ADD(PasswordTest::testPasswordCheckPasswordCharEncodingHandle);
			TEST_ADD(PasswordTest::testPasswordCheckPasswordCharEncodingStruct);
		}
	};
}
