ODIR = ../../tmp/unittests
CC = g++
CommonSDIR = ../Common

CFLAGS = -DCS_UNITTESTING -fprofile-arcs -ftest-coverage -I . -I ../../var/opt/cpptest-code/cpptest/src/


.PHONY: clean gcov coverage.xml run unittests

coverage.xml: ../../doc/devdocs/generated/reports/coverage.xml

../../doc/devdocs/generated/reports/coverage.xml: gcov
	mkdir -p ../../doc/devdocs/generated/reports/
	gcovr --xml -b -k -r ../../ -o ../../doc/devdocs/generated/reports/coverage.xml -v -g -s -e '^src/unit-tests/' -e '^var/'

run: unittests
	../../tmp/unittests/unittesting

gcov: $(ODIR)/unittesting run
	gcov -b -c ../../tmp/unittests/*.gcda -o ../../tmp/unittests/
	mv -v *.gcov ../../tmp/unittests/

unittests: $(ODIR)/unittesting

$(ODIR)/unittesting: $(ODIR)/unittesting.o $(ODIR)/Crc.o $(ODIR)/Endian.o $(ODIR)/Password.o $(ODIR)/strcpys.o $(ODIR)/ConvertUTF.o $(ODIR)/strcmpw.o $(ODIR)/CloseHandle.o $(ODIR)/CreateFile.o $(ODIR)/DeviceIoControl.o $(ODIR)/EnableWindow.o $(ODIR)/FlushFileBuffers.o $(ODIR)/GetFileSize.o $(ODIR)/GetFileTime.o $(ODIR)/GetLastError.o $(ODIR)/GetWindowText.o $(ODIR)/GetWindowTextLength.o $(ODIR)/SetFilePointer.o $(ODIR)/SetFileTime.o $(ODIR)/SetLastError.o $(ODIR)/MessageBox.o $(ODIR)/wip.o $(ODIR)/source.o $(ODIR)/suite.o $(ODIR)/textoutput.o $(ODIR)/time.o $(ODIR)/utils.o $(ODIR)/collectoroutput.o $(ODIR)/compileroutput.o $(ODIR)/htmloutput.o $(ODIR)/missing.o 
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -o $@ $(ODIR)/unittesting.o $(ODIR)/Crc.o $(ODIR)/Endian.o $(ODIR)/Password.o $(ODIR)/strcpys.o $(ODIR)/ConvertUTF.o $(ODIR)/strcmpw.o $(ODIR)/CloseHandle.o $(ODIR)/CreateFile.o $(ODIR)/DeviceIoControl.o $(ODIR)/EnableWindow.o $(ODIR)/FlushFileBuffers.o $(ODIR)/GetFileSize.o $(ODIR)/GetFileTime.o $(ODIR)/GetLastError.o $(ODIR)/GetWindowText.o $(ODIR)/GetWindowTextLength.o $(ODIR)/SetFilePointer.o $(ODIR)/SetFileTime.o $(ODIR)/SetLastError.o $(ODIR)/MessageBox.o $(ODIR)/wip.o $(ODIR)/source.o $(ODIR)/suite.o $(ODIR)/textoutput.o $(ODIR)/time.o $(ODIR)/utils.o $(ODIR)/collectoroutput.o $(ODIR)/compileroutput.o $(ODIR)/htmloutput.o $(ODIR)/missing.o 

$(ODIR)/unittesting.o: unittesting.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/Crc.o: ../Common/Crc.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/Endian.o: ../Common/Endian.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/Password.o: ../Common/Password.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/strcpys.o: ../Common/strcpys.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/ConvertUTF.o: ../Common/util/unicode/ConvertUTF.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/strcmpw.o: ../Common/util/unicode/strcmpw.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/CloseHandle.o: faux/windows/CloseHandle.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/CreateFile.o: faux/windows/CreateFile.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/DeviceIoControl.o: faux/windows/DeviceIoControl.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/EnableWindow.o: faux/windows/EnableWindow.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/FlushFileBuffers.o: faux/windows/FlushFileBuffers.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/GetFileSize.o: faux/windows/GetFileSize.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/GetFileTime.o: faux/windows/GetFileTime.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/GetLastError.o: faux/windows/GetLastError.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/GetWindowText.o: faux/windows/GetWindowText.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/GetWindowTextLength.o: faux/windows/GetWindowTextLength.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/SetFilePointer.o: faux/windows/SetFilePointer.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/SetFileTime.o: faux/windows/SetFileTime.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/SetLastError.o: faux/windows/SetLastError.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/MessageBox.o: faux/windows/MessageBox.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/wip.o: faux/ciphershed/wip.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/source.o: ../../var/opt/cpptest-code/cpptest/src/source.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/suite.o: ../../var/opt/cpptest-code/cpptest/src/suite.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/textoutput.o: ../../var/opt/cpptest-code/cpptest/src/textoutput.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/time.o: ../../var/opt/cpptest-code/cpptest/src/time.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/utils.o: ../../var/opt/cpptest-code/cpptest/src/utils.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/collectoroutput.o: ../../var/opt/cpptest-code/cpptest/src/collectoroutput.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/compileroutput.o: ../../var/opt/cpptest-code/cpptest/src/compileroutput.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/htmloutput.o: ../../var/opt/cpptest-code/cpptest/src/htmloutput.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/missing.o: ../../var/opt/cpptest-code/cpptest/src/missing.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)




clean:
	rm -rf $(ODIR)/
