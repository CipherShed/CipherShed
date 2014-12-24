#include "../../unittesting.h"

//#undef TC_WINDOWS_DRIVER
//#define BOOL int
//#include "../../../Common/Volumes.h"

namespace CipherShed_Tests_IO
{
	TESTCLASS
	PUBLIC_REF_CLASS HeaderTest TESTCLASSEXTENDS
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
		void testTrue()
		{
			TEST_ASSERT(1==1);
		};

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		HeaderTest()
		{
			TEST_ADD(HeaderTest::testTrue);
		}
	};
}
