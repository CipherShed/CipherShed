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

		TESTMETHOD
		void testEndianUnion032()
		{
			test_s32 w1;
			w1.a=0x12345678;

			TEST_ASSERT_MSG(w1.c.a==0x12||w1.c.a==0x34||w1.c.a==0x56||w1.c.a==0x78,"invalid value for a");
			TEST_ASSERT_MSG(w1.c.b==0x12||w1.c.b==0x34||w1.c.b==0x56||w1.c.b==0x78,"invalid value for b");
			TEST_ASSERT_MSG(w1.c.c==0x12||w1.c.c==0x34||w1.c.c==0x56||w1.c.c==0x78,"invalid value for c");
			TEST_ASSERT_MSG(w1.c.d==0x12||w1.c.d==0x34||w1.c.d==0x56||w1.c.d==0x78,"invalid value for d");

			if (w1.c.a==0x12)
			{
				if (w1.c.b==0x34)
				{
					if (w1.c.c==0x56)
					{
						if (w1.c.d==0x78)
						{
							//Big Endian
							TEST_ASSERT_MSG(w1.b.a==0x1234,"16bit union layout unexpected");
						}
						else
						{
							TEST_ASSERT_MSG(0,"Not possible situation");
						}
					}
					else
					{
						TEST_ASSERT_MSG(w1.c.c==0x56,"MIXED ENDIAN unsupported!");
					}
				}
				else
				{
					TEST_ASSERT_MSG(w1.c.b==0x34,"MIXED ENDIAN unsupported!");
				}
			}
			else if (w1.c.a==0x34)
			{
				//MIXED
				TEST_ASSERT_MSG(0,"MIXED ENDIAN unsupported!");
			}
			else if (w1.c.a==0x56)
			{
				//MIXED
				TEST_ASSERT_MSG(0,"MIXED ENDIAN unsupported!");
			}
			else if (w1.c.a==0x78)
			{
				if (w1.c.b==0x56)
				{
					if (w1.c.c==0x34)
					{
						if (w1.c.d==0x12)
						{
							//LITTLE ENDIAN
							TEST_ASSERT_MSG(w1.b.a==0x5678,"16bit union layout unexpected");
						}
						else
						{
							TEST_ASSERT_MSG(0,"Not possible situation");
						}
					}
					else
					{
						TEST_ASSERT_MSG(w1.c.c==0x56,"MIXED ENDIAN unsupported!");
					}
				}
				else
				{
					TEST_ASSERT_MSG(w1.c.b==0x34,"MIXED ENDIAN unsupported!");
				}
			}
			else
			{
				TEST_ASSERT_MSG(0,"Not possible situation");
			}



		};


		TESTMETHOD
		void testEndianUnion016()
		{
			test_s16 w1;
			w1.a=0x1234;

			TEST_ASSERT_MSG(w1.b.a==0x12||w1.b.a==0x34||w1.b.a==0x56||w1.b.a==0x78,"invalid value for a");
			TEST_ASSERT_MSG(w1.b.b==0x12||w1.b.b==0x34||w1.b.b==0x56||w1.b.b==0x78,"invalid value for b");

			if (w1.b.a==0x12)
			{
				//Big Endian
				TEST_ASSERT_MSG(w1.b.b==0x34,"bad 8bit value");
			}
			else if (w1.b.a==0x34)
			{
				//LITTLE ENDIAN
				TEST_ASSERT_MSG(w1.b.b==0x12,"MIXED ENDIAN unsupported!");
			}
			else
			{
				TEST_ASSERT_MSG(0,"Not possible situation");
			}



		};

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

		TESTMETHOD
		void testEndian32bitSwap()
		{
			test_s32 w1;
			w1.c.a=0x12;
			w1.c.b=0x34;
			w1.c.c=0x56;
			w1.c.d=0x78;

			TEST_ASSERT_MSG(w1.c.a==0x12,"t01");
			TEST_ASSERT_MSG(w1.c.b==0x34,"t02");
			TEST_ASSERT_MSG(w1.c.c==0x56,"t03");
			TEST_ASSERT_MSG(w1.c.d==0x78,"t04");

			test_s32 w2;

			w2.a=MirrorBytes32(w1.a);
			
			TEST_ASSERT_MSG(w1.c.a==w2.c.d,"t09");
			TEST_ASSERT_MSG(w1.c.b==w2.c.c,"t10");
			TEST_ASSERT_MSG(w1.c.c==w2.c.b,"t12");
			TEST_ASSERT_MSG(w1.c.d==w2.c.a,"t13");
		};

		TESTMETHOD
		void testEndian64bitSwap()
		{
			test_s64 w1;
			w1.d.a=0x12;
			w1.d.b=0x34;
			w1.d.c=0x56;
			w1.d.d=0x78;
			w1.d.e=0x9a;
			w1.d.f=0xbc;
			w1.d.g=0xde;
			w1.d.h=0xf0;

			TEST_ASSERT_MSG(w1.d.a==0x12,"t01");
			TEST_ASSERT_MSG(w1.d.b==0x34,"t02");
			TEST_ASSERT_MSG(w1.d.c==0x56,"t03");
			TEST_ASSERT_MSG(w1.d.d==0x78,"t04");
			TEST_ASSERT_MSG(w1.d.e==0x9a,"t05");
			TEST_ASSERT_MSG(w1.d.f==0xbc,"t06");
			TEST_ASSERT_MSG(w1.d.g==0xde,"t07");
			TEST_ASSERT_MSG(w1.d.h==0xf0,"t08");

			test_s64 w2;

			w2.a=MirrorBytes64(w1.a);
			
			TEST_ASSERT_MSG(w1.d.a==w2.d.h,"t09");
			TEST_ASSERT_MSG(w1.d.b==w2.d.g,"t10");
			TEST_ASSERT_MSG(w1.d.c==w2.d.f,"t12");
			TEST_ASSERT_MSG(w1.d.d==w2.d.e,"t13");
			TEST_ASSERT_MSG(w1.d.e==w2.d.d,"t14");
			TEST_ASSERT_MSG(w1.d.f==w2.d.c,"t15");
			TEST_ASSERT_MSG(w1.d.g==w2.d.b,"t16");
			TEST_ASSERT_MSG(w1.d.h==w2.d.a,"t17");
		};

		TESTMETHOD
		void testEndian008BitSizeAssumptions()
		{
			uint8 a;			
			TEST_ASSERT_MSG(sizeof(a)==1,"wrong sizeof");

			a=0xff;
			TEST_ASSERT(a==255);

			++a;
			TEST_ASSERT(a!=256);
			TEST_ASSERT(a==(uint8)256);
			TEST_ASSERT(a==0);

			a=1;
			for (int i=1; i<8; ++i)
			{
				a=a<<1;
				TEST_ASSERT_MSG(a!=0,"Columbus was wrong");
			}
			a=a<<1;
			TEST_ASSERT_MSG(a==0,"too many bits in int");

		};

		TESTMETHOD
		void testEndian016BitSizeAssumptions()
		{
			uint16 a;			
			TEST_ASSERT(sizeof(a)==2);

			a=0xffff;
			TEST_ASSERT(a==65535);

			++a;
			TEST_ASSERT(a!=65536);
			TEST_ASSERT(a==(uint16)65536);
			TEST_ASSERT(a==0);

			a=1;
			for (int i=1; i<16; ++i)
			{
				a=a<<1;
				TEST_ASSERT_MSG(a!=0,"Columbus was wrong");
			}
			a=a<<1;
			TEST_ASSERT_MSG(a==0,"too many bits in int");
		};

		TESTMETHOD
		void testEndian032BitSizeAssumptions()
		{
			uint32 a;			
			TEST_ASSERT(sizeof(a)==4);

			a=0xffffffff;
			TEST_ASSERT_MSG(a==4294967295U,"max uint32");

			++a;
			TEST_ASSERT_MSG(a!=4294967296, "max uint32 + 1");
			TEST_ASSERT_MSG(a==(uint32)4294967296, "max uint32 +1 cast to uint32");
			TEST_ASSERT_MSG(a==0,"test overflow");

			a=1;
			for (int i=1; i<32; ++i)
			{
				a=a<<1;
				TEST_ASSERT_MSG(a!=0,"Columbus was wrong");
			}
			a=a<<1;
			TEST_ASSERT_MSG(a==0,"too many bits in int");

		};

		TESTMETHOD
		void testEndian064BitSizeAssumptions()
		{
			uint64 a;			
			TEST_ASSERT(sizeof(a)==8);

			a=0xffffffffffffffff;
			TEST_ASSERT_MSG(a==18446744073709551615U,"max uint64");

			++a;

			//TEST_ASSERT_MSG(a!=18446744073709551616, "max uint64 + 1");
			//TEST_ASSERT_MSG(a==(uint64)18446744073709551616, "max uint64 +1 cast to uint64");
			TEST_ASSERT_MSG(a==0,"test overflow");

			a=1;
			for (int i=1; i<64; ++i)
			{
				a=a<<1;
				TEST_ASSERT_MSG(a!=0,"Columbus was wrong");
			}
			a=a<<1;
			TEST_ASSERT_MSG(a==0,"too many bits in int");

		};

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		EndianTest()
		{
			TEST_ADD(EndianTest::testEndianUnion032);
			TEST_ADD(EndianTest::testEndian008BitSizeAssumptions);
			TEST_ADD(EndianTest::testEndian016BitSizeAssumptions);
			TEST_ADD(EndianTest::testEndian032BitSizeAssumptions);
			TEST_ADD(EndianTest::testEndian064BitSizeAssumptions);
			TEST_ADD(EndianTest::testEndian16bitSwap);
			TEST_ADD(EndianTest::testEndian32bitSwap);
			TEST_ADD(EndianTest::testEndian64bitSwap);
		}
	};
}
