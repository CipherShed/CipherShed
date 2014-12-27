#include "../../unittesting.h"

#include "../../../Common/Endian.h"

typedef unsigned __int8 uint8;

typedef union 
{
	uint64 a;
	struct
	{
		uint32 a;
		uint32 b;
	} b;
	struct
	{
		uint16 a;
		uint16 b;
		uint16 c;
		uint16 d;
	} c;
	struct
	{
		uint8 a;
		uint8 b;
		uint8 c;
		uint8 d;
		uint8 e;
		uint8 f;
		uint8 g;
		uint8 h;
	} d;
} test_s64;

typedef union 
{
	uint32 a;
	struct
	{
		uint16 a;
		uint16 b;
	} b;
	struct
	{
		uint8 a;
		uint8 b;
		uint8 c;
		uint8 d;
	} c;
} test_s32;

typedef union 
{
	uint16 a;
	struct
	{
		uint8 a;
		uint8 b;
	} b;
} test_s16;

namespace CipherShed_Tests_Algo
{
	TESTCLASS
	PUBLIC_REF_CLASS EndianTest TESTCLASSEXTENDS
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
		void testEndian16bitSwap()
		{
			test_s16 w1;
			w1.b.a=0x12;
			w1.b.b=0x34;
			test_s16 w2;
			w2.a=MirrorBytes16(w1.a);
			
			TEST_ASSERT(w1.b.a!=w1.b.b);
			
			TEST_ASSERT(w1.b.a==w2.b.b);
			
			TEST_ASSERT(w1.b.b==w2.b.a);
		};

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		EndianTest()
		{
			TEST_ADD(EndianTest::testEndian16bitSwap);
		}
	};
}
