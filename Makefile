# This should work on Linux.  Modify as needed for other platforms.
BOINC_DIR=../boinc-mge
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SCHED_DIR = $(BOINC_DIR)/sched
BOINC_TOOLS_DIR = $(BOINC_DIR)/tools
BOINC_DB_DIR = $(BOINC_DIR)/db

CXXFLAGS += -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -I$(BOINC_SCHED_DIR) \
    -I$(BOINC_TOOLS_DIR) \
    -I$(BOINC_DB_DIR) \
    -I/usr/include/mysql \
    -L.

PROGS = cannyedge_app cannyedge_work_generator cannyedge_assimilator

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROGS) *.o *.a contrib/*.o libstdc++.a

install: cannyedge_app

cannyedge_app: cannyedge_app.o libcontrib.a libstdc++.a $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o cannyedge_app cannyedge_app.o -lcontrib libstdc++.a -lpthread \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a

cannyedge_work_generator: cannyedge_work_generator.o libcontrib.a libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_LIB_DIR)/libboinc_crypt.a $(BOINC_SCHED_DIR)/libsched.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o cannyedge_work_generator cannyedge_work_generator.o $(BOINC_SCHED_DIR)/libsched.a $(BOINC_LIB_DIR)/libboinc.a \
	$(BOINC_LIB_DIR)/libboinc_crypt.a -lcontrib -lpthread -lmariadbclient -lcrypto -lssl

cannyedge_assimilator: $(BOINC_SCHED_DIR)/assimilator.o cannyedge_assimilator.o $(BOINC_SCHED_DIR)/validate_util.o libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_LIB_DIR)/libboinc_crypt.a $(BOINC_SCHED_DIR)/libsched.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o cannyedge_assimilator $(BOINC_SCHED_DIR)/assimilator.o cannyedge_assimilator.o $(BOINC_SCHED_DIR)/validate_util.o $(BOINC_SCHED_DIR)/libsched.a $(BOINC_LIB_DIR)/libboinc.a \
	$(BOINC_LIB_DIR)/libboinc_crypt.a -lpthread -lmariadbclient -lcrypto -lssl

libcontrib.a: contrib/bmp.o contrib/CannyEdgeDetector.o
	ar rcs libcontrib.a contrib/bmp.o contrib/CannyEdgeDetector.o

libs: libcontrib.a
