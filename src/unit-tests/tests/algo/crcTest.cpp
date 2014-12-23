
#include "../../unittesting.h"

//#pragma warning( push )
//#pragma warning( disable : 4959 4956 )
#undef TC_WINDOWS_DRIVER
#define BOOL int
#include "../../../Common/Crc.h"
//#pragma warning( pop )

namespace crc
{
	TESTCLASS
	PUBLIC_REF_CLASS CrcTest TESTCLASSEXTENDS
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
		void testCRC()
		{
			//http://www.ross.net/crc/download/crc_v3.txt
			//http://reveng.sourceforge.net/crc-catalogue/

			TEST_ASSERT(1==1) //crc32_selftests()
			TEST_ASSERT(crc32_selftests()!=FALSE)
		};


		TESTMETHOD
		void testCRCsmall()
		{
				TEST_ASSERT(crc32_selfTestSmall()!=FALSE)
		}


		TESTMETHOD
		void testCRCbig()
		{
				TEST_ASSERT(crc32_selfTestLarge()!=FALSE);
		}

		TESTMETHOD
		void testCRCcatalog()
		{
				//http://reveng.sourceforge.net/crc-catalogue/17plus.htm
				unsigned char testData[]={'1','2','3','4','5','6','7','8','9'};
				TEST_ASSERT(GetCrc32 (testData, sizeof (testData)) == 0xcbf43926UL);
		}

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		CrcTest()
		{
			TEST_ADD(CrcTest::testCRC);
			TEST_ADD(CrcTest::testCRCsmall);
			TEST_ADD(CrcTest::testCRCbig);
			TEST_ADD(CrcTest::testCRCcatalog);
		}
	};
}
