#ifndef _unittesting_h_
#define _unittesting_h_

#ifndef CS_UNITTESTING
#define CS_UNITTESTING
#endif

#ifdef _MSC_FULL_VER

#define TESTCLASS [TestClass]
#define TESTMETHOD [TestMethod]
using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;
#define PUBLIC_REF_CLASS public ref class
#define TESTCONTEXT TestContext^
#define FQTESTCONTEXT Microsoft::VisualStudio::TestTools::UnitTesting::TESTCONTEXT
#define TESTCONTEXTPROP \
		property TestContext^ TestContext\
		{\
			FQTESTCONTEXT get()\
			{\
				return testContextInstance;\
			}\
			System::Void set(FQTESTCONTEXT value)\
			{\
				testContextInstance = value;\
			}\
		};
#define TESTCLASSEXTENDS
#define MAINTESTDECL
#define MAINADDTEST(x)
#define MAINTESTRUN
#define TEST_ADD(x)

//http://msdn.microsoft.com/en-us/library/Microsoft.VisualStudio.TestTools.UnitTesting.Assert_methods.aspx
//http://cpptest.sourceforge.net/cpptest-assert_8h.html
#define TEST_ASSERT(x) Assert::IsTrue(x);
#define TEST_ASSERT_MSG(x,y) Assert::IsTrue(x,gcnew String(y));
//debug messages...
//http://stackoverflow.com/questions/16815804/how-to-get-console-output-in-visual-studio-2012-unit-tests
#else
#include <cpptest.h>
#define TESTCLASS
#define TESTMETHOD
#define PUBLIC_REF_CLASS class
#define TESTCONTEXT void*
#define TESTCONTEXTPROP
#define TESTCLASSEXTENDS : public Test::Suite

#define MAINTESTDECL Test::Suite ts;
#define MAINADDTEST(x) ts.add(std::auto_ptr<Test::Suite>((x)));
#define MAINTESTRUN Test::TextOutput output(Test::TextOutput::Verbose); return !ts.run(output);

#endif

#endif