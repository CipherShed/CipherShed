#include "../../unittesting.h"

#include <stdio.h>
#include "../../faux/windows/WCHAR.h"
#include "../../faux/windows/BOOL.h"
//#include "../../../Common/Password.h"
//#include <string.h>
//

#include "../../../Common/util/unicode/ConvertUTF.h"

#pragma warning(disable: 4996)
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

namespace CipherShed_Tests_lib
{

	char assertmsg[2048];

	typedef struct ts1
	{
		char a[2048];
		wchar_t w[2048];
		int r;
	} ts1;


	ts1 tests1[]=
	{
		{"",L"",1},
		{"the quick brown fox jumped over the lazy dogs",L"the quick brown fox jumped over the lazy dogs",1},
		{{'A','\0'},{65,0},1},
		{{'A','\0'},{0x05D0u,0},0},
		{{'A','\0'},{0xD834u, 0xDD5Fu,0},0}, //0x01d15f UTF8={0xF0, 0x9D, 0x85, 0x9F, 0} http://www.unicode.org/charts/PDF/Unicode-3.1/U31-1D100.pdf
	};

	typedef struct ts2
	{
		char a[2048];
		wchar_t w[2048];
		int r;
	} ts2;


	ts2 tests2[]=
	{
		{"",L"",conversionOK},
		{"the quick brown fox jumped over the lazy dogs",L"the quick brown fox jumped over the lazy dogs",conversionOK},
		{{'A','\0'},{65,0},conversionOK},
		{{0xD7u, 0x90u, 0},{0x05D0,0},conversionOK},
		{{0xF0u, 0x9Du, 0x85u, 0x9Fu, 0},{0xD834u, 0xDD5Fu,0},conversionOK}, //0x01d15f UTF8= http://www.unicode.org/charts/PDF/Unicode-3.1/U31-1D100.pdf
//		{{0xF0u, 0x9Du, 0x85u, 0x9Fu, 0},{0xD834u}, sourceExhausted},        /* partial character in source, but hit end */
//		{{0},{0}, targetExhausted},        /* insuff. room in target for conversion */
		{{0xF0u, 0x9Du, 0x85u, 0x9Fu, 0},{0xD834u,0}, sourceIllegal},           /* source sequence is illegal/malformed */
	};

	TESTCLASS
	PUBLIC_REF_CLASS UnicodeTest TESTCLASSEXTENDS
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
		void testUnicodeTrue()
		{
			TEST_ASSERT(1==1);
		};

		TESTMETHOD
		void testUnicodeAscii()
		{
			for (int i=0; i<sizeof(tests1)/sizeof(tests1[0]); ++i)
			{
				char* test=tests1[i].a;
				wchar_t* testw=tests1[i].w;
				char buf[2048];
				for (int ii=0; ii<sizeof(buf)/sizeof(buf[0]); ++ii) buf[ii]=0;

				if (tests1[i].r)
				{
					for (int ii=0; i<sizeof(tests1[i].a) && tests1[i].a[ii]; ++ii)
					{
						sprintf(assertmsg,"i:%d, ii:%d, test str <%s> expected:%04x, got %04x", i, ii, tests1[i].a, test[ii], testw[ii]);
						TEST_ASSERT_MSG(test[ii]==testw[ii],assertmsg);
					}
				}
				else
				{
				}

				TEST_ASSERT(1==1);

				ConversionResult cr1;

				wchar_t* bufPtr;
				int bufEnd;
				char* utf8BufPtr;

				bufPtr=testw;
				utf8BufPtr=buf;
				for (bufEnd=0; testw[bufEnd]; ++bufEnd);
				++bufEnd;
				cr1=ConvertUTF16toUTF8((const UTF16**)&bufPtr, (const UTF16*)&testw[bufEnd], (UTF8**)&utf8BufPtr, (UTF8*)&utf8BufPtr[sizeof(buf)], strictConversion);

				TEST_ASSERT(cr1==conversionOK);

				int r=tests1[i].r;
				for (int ii=0; test[ii]; ++ii)
				{
					if (r)
					{
						sprintf(assertmsg,"i:%d, ii:%d, test str <%s> expected:%04x, got %04x", i, ii, test, test[ii], buf[ii]);
						TEST_ASSERT_MSG(test[ii]==buf[ii],assertmsg);
					}
					else
					{
						if (test[ii]!=buf[ii])
						{
							r=1;
							break;
						}
					}
				}
				sprintf(assertmsg,"i:%d, test str <%s> reached end of string without difference", i, test);
				TEST_ASSERT_MSG(r==1,assertmsg);
			}

		};

		TESTMETHOD
		void testUnicodeUTF8()
		{
			for (int i=0; i<sizeof(tests2)/sizeof(tests2[0]); ++i)
			{
				ts2 test2=tests2[i];
				char* test=test2.a;
				wchar_t* testw=test2.w;
				char buf[2048];
				for (int ii=0; ii<sizeof(buf)/sizeof(buf[0]); ++ii) buf[ii]=0;

				ConversionResult cr1;

				wchar_t* bufPtr;
				int bufEnd;
				char* utf8BufPtr;

				bufPtr=testw;
				utf8BufPtr=buf;
				for (bufEnd=0; testw[bufEnd]; ++bufEnd);
				++bufEnd;
				cr1=ConvertUTF16toUTF8((const UTF16**)&bufPtr, (const UTF16*)&testw[bufEnd], (UTF8**)&utf8BufPtr, (UTF8*)&utf8BufPtr[sizeof(buf)], strictConversion);

				sprintf(assertmsg,"i:%d, test str <%s> expected:%04x, got %04x", i, test, test2.r, cr1);
				TEST_ASSERT_MSG(cr1==test2.r,assertmsg);

				int r=test2.r;
				for (int ii=0; test[ii]; ++ii)
				{
					if (r==conversionOK)
					{
						sprintf(assertmsg,"i:%d, ii:%d, test str <%s> expected:%04x, got %04x", i, ii, test, test[ii], buf[ii]);
						TEST_ASSERT_MSG(test[ii]==buf[ii],assertmsg);
					}
					else
					{
						if (test[ii]!=buf[ii])
						{
							r=conversionOK;
							break;
						}
					}
				}
				sprintf(assertmsg,"i:%d, test str <%s> reached end of string without difference", i, test);
				TEST_ASSERT_MSG(r==conversionOK,assertmsg);
			}

		};

		TESTMETHOD
		void testUnicodeUTF8ReplacementCharacter()
		{
			//DBE6 DD9A
			char buf[2048];
			for (int ii=0; ii<sizeof(buf)/sizeof(buf[0]); ++ii) buf[ii]=0;

			wchar_t testw[]={0xFFFD};

			ConversionResult cr1;

			wchar_t* bufPtr;
			char* utf8BufPtr;

			bufPtr=testw;
			
			utf8BufPtr=buf;
			int r=conversionOK;
			cr1=ConvertUTF16toUTF8
			(
				(const UTF16**)&bufPtr, 
				(const UTF16*)&testw[0], 
				(UTF8**)&utf8BufPtr, 
				(UTF8*)&utf8BufPtr[sizeof(buf)], 
				strictConversion
			);

			sprintf(assertmsg,"expected:%04x, got %04x", r, cr1);
			TEST_ASSERT_MSG(cr1==r,assertmsg);

			//sprintf(assertmsg,"i:%d, test str <%s> reached end of string without difference", i, test);
			//TEST_ASSERT_MSG(r==conversionOK,assertmsg);
		};

		TESTMETHOD
		void testUnicodeUTF8LargeNumber()
		{
			//DBE6 DD9A
			char buf[2048];
			for (int ii=0; ii<sizeof(buf)/sizeof(buf[0]); ++ii) buf[ii]=0;

			wchar_t testw[]={0xDBE6u, 0xDD9Au};

			ConversionResult cr1;

			wchar_t* bufPtr;
			char* utf8BufPtr;

			bufPtr=testw;
			
			utf8BufPtr=buf;
			int r=conversionOK;
			cr1=ConvertUTF16toUTF8
			(
				(const UTF16**)&bufPtr, 
				(const UTF16*)&testw[sizeof(testw)/sizeof(testw[0])], 
				(UTF8**)&utf8BufPtr, 
				(UTF8*)&utf8BufPtr[sizeof(buf)], 
				strictConversion
			);

			sprintf(assertmsg,"expected:%04x, got %04x", r, cr1);
			TEST_ASSERT_MSG(cr1==r,assertmsg);

			//sprintf(assertmsg,"i:%d, test str <%s> reached end of string without difference", i, test);
			//TEST_ASSERT_MSG(r==conversionOK,assertmsg);
		};

		TESTMETHOD
		void testUnicodeUTF8TargetExhausted()
		{
			char buf[5];
			for (int ii=0; ii<sizeof(buf)/sizeof(buf[0]); ++ii) buf[ii]=0;

			wchar_t testw[]=L"the quick brown fox";

			ConversionResult cr1;

			wchar_t* bufPtr;
			char* utf8BufPtr;

			bufPtr=testw;
			
			utf8BufPtr=buf;
			int r=targetExhausted;
			cr1=ConvertUTF16toUTF8
			(
				(const UTF16**)&bufPtr, 
				(const UTF16*)&testw[sizeof(testw)/sizeof(testw[0])], 
				(UTF8**)&utf8BufPtr, 
				(UTF8*)&utf8BufPtr[sizeof(buf)], 
				strictConversion
			);

			sprintf(assertmsg,"expected:%04x, got %04x", r, cr1);
			TEST_ASSERT_MSG(cr1==r,assertmsg);

			//sprintf(assertmsg,"i:%d, test str <%s> reached end of string without difference", i, test);
			//TEST_ASSERT_MSG(r==conversionOK,assertmsg);
		};

		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		UnicodeTest()
		{
			TEST_ADD(UnicodeTest::testUnicodeTrue);
			TEST_ADD(UnicodeTest::testUnicodeAscii);
			TEST_ADD(UnicodeTest::testUnicodeUTF8);
			TEST_ADD(UnicodeTest::testUnicodeUTF8ReplacementCharacter);
			TEST_ADD(UnicodeTest::testUnicodeUTF8LargeNumber);
			TEST_ADD(UnicodeTest::testUnicodeUTF8TargetExhausted);
		}
	};
}
