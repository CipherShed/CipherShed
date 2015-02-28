#include "../../unittesting.h"

#include "../../faux/windows/HWND.h"
#include "../../faux/windows/BOOL.h"
#include "../../faux/windows/EnableWindow.h"
#include "../../../Common/Password.h"
#include <string.h>

#include <stdio.h>

#include "../../../Common/util/unicode/strcmpw.h"

#pragma warning(disable: 4996)
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifndef IDYES
#define IDYES               6
#endif
#ifndef IDNO
#define IDNO                7
#endif

typedef struct TS
{
	char* s;
	int r;
} TS;
namespace CipherShed_Tests_Algo
{
	typedef struct TS_PasswordCheckPasswordLength
	{
		char* s;
		int b;
		int r;
	} TS_PasswordCheckPasswordLength;

	static TS_PasswordCheckPasswordLength testDataPasswordCheckPasswordLength[]=
	{
		{"",                                                                      IDNO,0}, //len=0
		{"a",                                                                     IDNO,0}, //len=1
		{"ab",                                                                    IDNO,0}, //len=2
		{"abc",                                                                   IDNO,0}, //len=3
		{"abcd",                                                                  IDNO,0}, //len=4
		{"abcde",                                                                 IDNO,0}, //len=5
		{"abcdef",                                                                IDNO,0}, //len=6
		{"abcdefg",                                                               IDNO,0}, //len=7
		{"abcdefgh",                                                              IDNO,0}, //len=8
		{"abcdefghi",                                                             IDNO,0}, //len=9
		{"abcdefghig",                                                            IDNO,0}, //len=10
		{"abcdefghigk",                                                           IDNO,0}, //len=11
		{"abcdefghigkl",                                                          IDNO,0}, //len=12
		{"abcdefghigklm",                                                         IDNO,0}, //len=13
		{"abcdefghigklmn",                                                        IDNO,0}, //len=14
		{"abcdefghigklmno",                                                       IDNO,0}, //len=15
		{"abcdefghigklmnop",                                                      IDNO,0}, //len=16
		{"abcdefghigklmnopq",                                                     IDNO,0}, //len=17
		{"abcdefghigklmnopqr",                                                    IDNO,0}, //len=18
		{"abcdefghigklmnopqrs",                                                   IDNO,0}, //len=19
		{"abcdefghigklmnopqrst",                                                  IDNO,1}, //len=20
		{"abcdefghigklmnopqrstu",                                                 IDNO,1}, //len=21
		{"abcdefghigklmnopqrstuv",                                                IDNO,1}, //len=22
		{"abcdefghigklmnopqrstuvw",                                               IDNO,1}, //len=23
		{"abcdefghigklmnopqrstuvwx",                                              IDNO,1}, //len=24
		{"abcdefghigklmnopqrstuvwxy",                                             IDNO,1}, //len=25
		{"abcdefghigklmnopqrstuvwxyz",                                            IDNO,1}, //len=26
		{"abcdefghigklmnopqrstuvwxyzA",                                           IDNO,1}, //len=27
		{"abcdefghigklmnopqrstuvwxyzAB",                                          IDNO,1}, //len=28
		{"abcdefghigklmnopqrstuvwxyzABC",                                         IDNO,1}, //len=29
		{"abcdefghigklmnopqrstuvwxyzABCD",                                        IDNO,1}, //len=30
		{"abcdefghigklmnopqrstuvwxyzABCDE",                                       IDNO,1}, //len=31
		{"abcdefghigklmnopqrstuvwxyzABCDEF",                                      IDNO,1}, //len=32
		{"abcdefghigklmnopqrstuvwxyzABCDEFG",                                     IDNO,1}, //len=33
		{"abcdefghigklmnopqrstuvwxyzABCDEFGH",                                    IDNO,1}, //len=34
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHI",                                   IDNO,1}, //len=35
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJ",                                  IDNO,1}, //len=36
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJK",                                 IDNO,1}, //len=37
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKL",                                IDNO,1}, //len=38
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLM",                               IDNO,1}, //len=39
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMN",                              IDNO,1}, //len=40
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNO",                             IDNO,1}, //len=41
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOP",                            IDNO,1}, //len=42
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQ",                           IDNO,1}, //len=43
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQR",                          IDNO,1}, //len=44
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRS",                         IDNO,1}, //len=45
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRST",                        IDNO,1}, //len=46
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU",                       IDNO,1}, //len=47
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV",                      IDNO,1}, //len=48
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW",                     IDNO,1}, //len=49
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWX",                    IDNO,1}, //len=50
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY",                   IDNO,1}, //len=51
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",                  IDNO,1}, //len=52
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1",                 IDNO,1}, //len=53
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12",                IDNO,1}, //len=54
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123",               IDNO,1}, //len=55
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234",              IDNO,1}, //len=56
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345",             IDNO,1}, //len=57
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456",            IDNO,1}, //len=58
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567",           IDNO,1}, //len=59
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678",          IDNO,1}, //len=60
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789",         IDNO,1}, //len=61
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890",        IDNO,1}, //len=62
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!",       IDNO,1}, //len=63
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@",      IDNO,1}, //len=64
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#",     IDNO,1}, //len=65
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$",    IDNO,1}, //len=66
		{"",                                                                      IDYES,1}, //len=0
		{"a",                                                                     IDYES,1}, //len=1
		{"ab",                                                                    IDYES,1}, //len=2
		{"abc",                                                                   IDYES,1}, //len=3
		{"abcd",                                                                  IDYES,1}, //len=4
		{"abcde",                                                                 IDYES,1}, //len=5
		{"abcdef",                                                                IDYES,1}, //len=6
		{"abcdefg",                                                               IDYES,1}, //len=7
		{"abcdefgh",                                                              IDYES,1}, //len=8
		{"abcdefghi",                                                             IDYES,1}, //len=9
		{"abcdefghig",                                                            IDYES,1}, //len=10
		{"abcdefghigk",                                                           IDYES,1}, //len=11
		{"abcdefghigkl",                                                          IDYES,1}, //len=12
		{"abcdefghigklm",                                                         IDYES,1}, //len=13
		{"abcdefghigklmn",                                                        IDYES,1}, //len=14
		{"abcdefghigklmno",                                                       IDYES,1}, //len=15
		{"abcdefghigklmnop",                                                      IDYES,1}, //len=16
		{"abcdefghigklmnopq",                                                     IDYES,1}, //len=17
		{"abcdefghigklmnopqr",                                                    IDYES,1}, //len=18
		{"abcdefghigklmnopqrs",                                                   IDYES,1}, //len=19
		{"abcdefghigklmnopqrst",                                                  IDYES,1}, //len=20
		{"abcdefghigklmnopqrstu",                                                 IDYES,1}, //len=21
		{"abcdefghigklmnopqrstuv",                                                IDYES,1}, //len=22
		{"abcdefghigklmnopqrstuvw",                                               IDYES,1}, //len=23
		{"abcdefghigklmnopqrstuvwx",                                              IDYES,1}, //len=24
		{"abcdefghigklmnopqrstuvwxy",                                             IDYES,1}, //len=25
		{"abcdefghigklmnopqrstuvwxyz",                                            IDYES,1}, //len=26
		{"abcdefghigklmnopqrstuvwxyzA",                                           IDYES,1}, //len=27
		{"abcdefghigklmnopqrstuvwxyzAB",                                          IDYES,1}, //len=28
		{"abcdefghigklmnopqrstuvwxyzABC",                                         IDYES,1}, //len=29
		{"abcdefghigklmnopqrstuvwxyzABCD",                                        IDYES,1}, //len=30
		{"abcdefghigklmnopqrstuvwxyzABCDE",                                       IDYES,1}, //len=31
		{"abcdefghigklmnopqrstuvwxyzABCDEF",                                      IDYES,1}, //len=32
		{"abcdefghigklmnopqrstuvwxyzABCDEFG",                                     IDYES,1}, //len=33
		{"abcdefghigklmnopqrstuvwxyzABCDEFGH",                                    IDYES,1}, //len=34
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHI",                                   IDYES,1}, //len=35
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJ",                                  IDYES,1}, //len=36
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJK",                                 IDYES,1}, //len=37
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKL",                                IDYES,1}, //len=38
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLM",                               IDYES,1}, //len=39
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMN",                              IDYES,1}, //len=40
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNO",                             IDYES,1}, //len=41
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOP",                            IDYES,1}, //len=42
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQ",                           IDYES,1}, //len=43
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQR",                          IDYES,1}, //len=44
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRS",                         IDYES,1}, //len=45
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRST",                        IDYES,1}, //len=46
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU",                       IDYES,1}, //len=47
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV",                      IDYES,1}, //len=48
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW",                     IDYES,1}, //len=49
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWX",                    IDYES,1}, //len=50
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY",                   IDYES,1}, //len=51
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",                  IDYES,1}, //len=52
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1",                 IDYES,1}, //len=53
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12",                IDYES,1}, //len=54
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123",               IDYES,1}, //len=55
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234",              IDYES,1}, //len=56
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345",             IDYES,1}, //len=57
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456",            IDYES,1}, //len=58
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567",           IDYES,1}, //len=59
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678",          IDYES,1}, //len=60
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789",         IDYES,1}, //len=61
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890",        IDYES,1}, //len=62
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!",       IDYES,1}, //len=63
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@",      IDYES,1}, //len=64
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#",     IDYES,1}, //len=65
		{"abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$",    IDYES,1}, //len=66
	};

	char assertmsg[2048];
	static TS tests[]=
	{
		{"",1},
		{"a",1},
		{"abcdefg",1},
		{"\n",0},
		{"1234567890123456789012345678901234567890123456789012345678901234",1},
		{"12345678901234567890123456789012345678901234567890123456789012345",0},
		{"\xA5",0},
		{"\x08",0},
	};

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
		void testPasswordCheckPasswordCharEncodingInvalid()
		{
			TEST_ASSERT(!CheckPasswordCharEncoding(NULL,NULL));
			Password pwd;
			fauxWindowText hnd;
			TEST_ASSERT(!CheckPasswordCharEncoding(&hnd,&pwd));
		}

		static int strlenA(char* str)
		{
			int l=0;
			while (*str++) ++l;
			return l;
		}

		static int strlenW(WCHAR* str)
		{
			int l=0;
			while (*str++) ++l;
			return l;
		}

		static int copyAtoW(char* a, WCHAR* w, int limit)
		{
			while(1)
			{
				*w=*a;

				if (!*w) 
				{
					return 0;
				}
				else if (--limit)
				{					
					++w;
					++a;
				}
				else
				{
					*w=0;
					return strlenA(a);
				}
			}
		}

		TESTMETHOD
		void testPasswordCheckPasswordCharEncodingHandle()
		{
			int len=sizeof(tests)/sizeof(tests[0]);

			for (int i=0; i<len; ++i)
			{
				fauxWindowText fwt;
				//WCHAR hnd[2048];

				int e=copyAtoW(tests[i].s,fwt.buf.w,sizeof(fwt.buf.w)/sizeof(*fwt.buf.w));
				TEST_ASSERT(e==0);

				int ii=strlenW(fwt.buf.w);
				int len=GetWindowTextLengthW(&fwt);
				sprintf(assertmsg,"i:%d test str <%s> len:%d, ii:%d",i,tests[i].s, len,ii);
				TEST_ASSERT_MSG(len==ii,assertmsg);

				BOOL res=CheckPasswordCharEncoding(&fwt, NULL);
				sprintf(assertmsg,"i:%d test str <%s> expected:%d, got %d",i,tests[i].s, tests[i].r,res);
				TEST_ASSERT_MSG(res==tests[i].r,assertmsg);
			}
		};


		TESTMETHOD
		void testPasswordCheckPasswordLengthInvalid()
		{
			fauxMessageBox hwnd1;
			fauxWindowText hwnd2;
			TEST_ASSERT(!CheckPasswordLength (NULL,&hwnd2));
			TEST_ASSERT(!CheckPasswordLength (&hwnd1,NULL));
		}

		TESTMETHOD
		void testPasswordCheckPasswordLength()
		{
			int len=sizeof(testDataPasswordCheckPasswordLength)/sizeof(testDataPasswordCheckPasswordLength[0]);

			for (int i=0; i<len; ++i)
			{
				fauxWindowText passwordWidget;
				fauxMessageBox messageBoxWidget;
				messageBoxWidget.retval=testDataPasswordCheckPasswordLength[i].b;

				int e=copyAtoW(testDataPasswordCheckPasswordLength[i].s,(WCHAR*)passwordWidget.buf.w,sizeof(passwordWidget.buf.w)/sizeof(*passwordWidget.buf.w));
				TEST_ASSERT(e==0);
				int ii=strlenW(passwordWidget.buf.w);
				
				int len=GetWindowTextLengthW(&passwordWidget);
				sprintf(assertmsg,"i:%d test str <%s> len:%d, ii:%d",i,testDataPasswordCheckPasswordLength[i].s, len,ii);
				TEST_ASSERT_MSG(len==ii,assertmsg);

				BOOL res=CheckPasswordLength (&messageBoxWidget,&passwordWidget);
				sprintf(assertmsg,"i:%d test str <%s> test button: 0x%02x expected:%d, got %d",i,testDataPasswordCheckPasswordLength[i].s,testDataPasswordCheckPasswordLength[i].b, testDataPasswordCheckPasswordLength[i].r,res);
				TEST_ASSERT_MSG(res==testDataPasswordCheckPasswordLength[i].r,assertmsg);
			}
		};

		WCHAR* strcpyw(WCHAR* d, WCHAR* s)
		{
			WCHAR* r=d;
			while (*s) *d++=*s++;
			*d=0;
			return r;
		}

		int strcmpwa(WCHAR* a, char* b)
		{
			if ((void*)a==(void*)b) return 0;
			if (a==NULL) return -1;
			if (b==NULL) return 1;
			while(1)
			{
				if (*a==*b)
				{
					if (*a==0)
					{
						return 0;
					}
					else
					{
						++a;
						++b;
					}
				}
				else if (*a>*b)
				{
					return 1;
				}
				else
				{
					return -1;
				}
			}
		}


		TESTMETHOD
		void testPasswordVerifyPasswordAndUpdate()
		{
			fauxEnablableWidget few;
			few.enabled=false;
			HWND hwndDlg=NULL;
			HWND hButton=&few;
			fauxWindowText fwtPassword;
			strcpyw(fwtPassword.buf.w,(WCHAR*)L"password");
			HWND hPassword=&fwtPassword;
			fauxWindowText fwtVerify;
			strcpyw(fwtVerify.buf.w,(WCHAR*)L"password");
			HWND hVerify=&fwtVerify;
			char buf1[2048];
			unsigned char *szPassword=(unsigned char *)buf1;
			char buf2[2048];
			char *szVerify=buf2;
			BOOL keyFilesEnabled=false;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==TRUE);
			TEST_ASSERT(0==strcmpwa(fwtPassword.buf.w,buf1));
			TEST_ASSERT(0==strcmpwa(fwtVerify.buf.w,buf2));
		}

		TESTMETHOD
		void testPasswordVerifyPasswordAndUpdateMTString()
		{
			fauxEnablableWidget few;
			few.enabled=false;
			HWND hwndDlg=NULL;
			HWND hButton=&few;
			fauxWindowText fwtPassword;
			strcpyw(fwtPassword.buf.w,(WCHAR*)L"");
			HWND hPassword=&fwtPassword;
			fauxWindowText fwtVerify;
			strcpyw(fwtVerify.buf.w,(WCHAR*)L"");
			HWND hVerify=&fwtVerify;
			char buf1[2048];
			unsigned char *szPassword=(unsigned char *)buf1;
			char buf2[2048];
			char *szVerify=buf2;
			BOOL keyFilesEnabled=false;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==false);
			TEST_ASSERT(0==strcmpwa(fwtPassword.buf.w,buf1));
			TEST_ASSERT(0==strcmpwa(fwtVerify.buf.w,buf2));
		}


		TESTMETHOD
		void testPasswordVerifyPasswordAndUpdateMissmatch()
		{
			fauxEnablableWidget few;
			few.enabled=false;
			HWND hwndDlg=NULL;
			HWND hButton=&few;
			fauxWindowText fwtPassword;
			strcpyw(fwtPassword.buf.w,(WCHAR*)L"xyzzy");
			HWND hPassword=&fwtPassword;
			fauxWindowText fwtVerify;
			strcpyw(fwtVerify.buf.w,(WCHAR*)L"password");
			HWND hVerify=&fwtVerify;
			char buf1[2048];
			unsigned char *szPassword=(unsigned char *)buf1;
			char buf2[2048];
			char *szVerify=buf2;
			BOOL keyFilesEnabled=false;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==false);
			TEST_ASSERT(0==strcmpwa(fwtPassword.buf.w,buf1));
			TEST_ASSERT(0==strcmpwa(fwtVerify.buf.w,buf2));
		}



		TESTMETHOD
		void testPasswordstrcmpw()
		{
			WCHAR a[64]={0x20,0x2020,0x00};
			WCHAR b[64]={0x20,0x2021,0x00};
			WCHAR c[64]={0x20,0x2021,0x00};

			TEST_ASSERT_MSG(strcmpw(a,a)==0,"a!=a ?!?!");

			TEST_ASSERT_MSG(strcmpw(NULL,a)==-1,"NULL,x not -1 ?!?!");

			TEST_ASSERT_MSG(strcmpw(a,NULL)==1,"x,NULL not +1 ?!?!");

			TEST_ASSERT_MSG(strcmpw(a,b)==-1,"a<b not -1 ?!?!");

			TEST_ASSERT_MSG(strcmpw(b,a)==1,"b>a not +1 ?!?!");

			TEST_ASSERT_MSG(strcmpw(b,c)==0,"b=c not 0 ?!?!");
		}



		TESTMETHOD
		void testPasswordChangePwdMtPasswords()
		{
			char *lpszVolume;

			Password mt={0,{0x00},{0,0,0}};
			Password notmt={1,{0x20,0x00},{0,0,0}};

			Password *oldPassword;
			Password *newPassword;
			int pkcs5;
			HWND hwndDlg;
			int res;

			TEST_ASSERT_MSG(ChangePwd (lpszVolume, &mt, newPassword, pkcs5, hwndDlg)==-1,"mt old password should return -1");

			TEST_ASSERT_MSG(ChangePwd (lpszVolume, &notmt, &mt, pkcs5, hwndDlg)==-1,"mt new password should return -1");
		}


		TESTMETHOD
		void testPasswordVerifyPasswordAndUpdateKeyFile()
		{
			fauxEnablableWidget few;
			few.enabled=false;
			HWND hwndDlg=NULL;
			HWND hButton=&few;
			fauxWindowText fwtPassword;
			strcpyw(fwtPassword.buf.w,(WCHAR*)L"");
			HWND hPassword=&fwtPassword;
			fauxWindowText fwtVerify;
			strcpyw(fwtVerify.buf.w,(WCHAR*)L"");
			HWND hVerify=&fwtVerify;
			char buf1[2048];
			unsigned char *szPassword=(unsigned char *)buf1;
			char buf2[2048];
			char *szVerify=buf2;
			BOOL keyFilesEnabled=true;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==TRUE);
			TEST_ASSERT(0==strcmpwa(fwtPassword.buf.w,buf1));
			TEST_ASSERT(0==strcmpwa(fwtVerify.buf.w,buf2));

			keyFilesEnabled=false;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==FALSE);
		}


		/**
				Run through the VerifyPasswordAndUpdate2 method, not returning the passwords.
		*/
		TESTMETHOD
		void testPasswordVerifyPasswordAndUpdateNullReturnBuffers()
		{
			fauxEnablableWidget few;
			few.enabled=false;
			HWND hwndDlg=NULL;
			HWND hButton=&few;
			fauxWindowText fwtPassword;
			strcpyw(fwtPassword.buf.w,(WCHAR*)L"");
			HWND hPassword=&fwtPassword;
			fauxWindowText fwtVerify;
			strcpyw(fwtVerify.buf.w,(WCHAR*)L"");
			HWND hVerify=&fwtVerify;
			char buf1[2048];
			unsigned char *szPassword=(unsigned char *)buf1;
			char buf2[2048];
			char *szVerify=buf2;
			BOOL keyFilesEnabled=true;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, NULL, 0, NULL, 0, keyFilesEnabled);
			TEST_ASSERT(few.enabled==TRUE);
		}

		TESTMETHOD
		void testPasswordVerifyPasswordAndUpdateReturnBuffersTooSmall()
		{
			fauxEnablableWidget few;
			few.enabled=false;
			HWND hwndDlg=NULL;
			HWND hButton=&few;
			fauxWindowText fwtPassword;
			strcpyw(fwtPassword.buf.w,(WCHAR*)L"password");
			HWND hPassword=&fwtPassword;
			fauxWindowText fwtVerify;
			strcpyw(fwtVerify.buf.w,(WCHAR*)L"password");
			HWND hVerify=&fwtVerify;
			char buf1[2048];
			unsigned char *szPassword=(unsigned char *)buf1;
			char buf2[2048];
			char *szVerify=buf2;
			BOOL keyFilesEnabled=false;
			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, 2, keyFilesEnabled);
			TEST_ASSERT(few.enabled==FALSE);

			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, 2, szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==FALSE);

			VerifyPasswordAndUpdate2(hwndDlg, hButton, hPassword, hVerify, szPassword, sizeof(buf1), szVerify, sizeof(buf2), keyFilesEnabled);
			TEST_ASSERT(few.enabled==TRUE);
		}

		TESTMETHOD
		void testPasswordStrlenwNULL()
		{
			int res=strlenw(NULL);

			TEST_ASSERT_MSG(res==0,"strlenw(NULL) should be zero");
		}



		/**
		The constructor needs the add each test method for the non-VS unit test execution.
		*/
		PasswordTest()
		{
			TEST_ADD(PasswordTest::testPasswordTrue);

			TEST_ADD(PasswordTest::testPasswordCheckPasswordCharEncodingHandle);
			TEST_ADD(PasswordTest::testPasswordCheckPasswordCharEncodingStruct);
			TEST_ADD(PasswordTest::testPasswordCheckPasswordCharEncodingInvalid);

			TEST_ADD(PasswordTest::testPasswordCheckPasswordLengthInvalid);
			TEST_ADD(PasswordTest::testPasswordCheckPasswordLength);

			TEST_ADD(PasswordTest::testPasswordVerifyPasswordAndUpdate);
			TEST_ADD(PasswordTest::testPasswordVerifyPasswordAndUpdateMTString);
			TEST_ADD(PasswordTest::testPasswordVerifyPasswordAndUpdateMissmatch);

			TEST_ADD(PasswordTest::testPasswordVerifyPasswordAndUpdateKeyFile);
			TEST_ADD(PasswordTest::testPasswordVerifyPasswordAndUpdateNullReturnBuffers);
			TEST_ADD(PasswordTest::testPasswordVerifyPasswordAndUpdateReturnBuffersTooSmall);

			TEST_ADD(PasswordTest::testPasswordstrcmpw);

			TEST_ADD(PasswordTest::testPasswordChangePwdMtPasswords);

			TEST_ADD(PasswordTest::testPasswordStrlenwNULL);
		}
	};
}
