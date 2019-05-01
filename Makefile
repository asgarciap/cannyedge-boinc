# This should work on Linux.  Modify as needed for other platforms.
BOINC_DIR=../boinc-mge
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SCHED_DIR = $(BOINC_DIR)/sched

CXXFLAGS += -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -L.

PROGS = cannyedge_app cannyedge_work_generator \

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROGS) *.o contrib/*.o libstdc++.a

install: cannyedge_app

# specify library paths explicitly (rather than -l)
# because otherwise you might get a version in /usr/lib etc.

cannyedge_app: cannyedge_app.o contrib/bmp.o contrib/CannyEdgeDetector.o libstdc++.a $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o cannyedge_app cannyedge_app.o libstdc++.a -pthread \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a

cannyedge_work_generator: cannyedge_work_generator contrib/bmp.o contrib/CannyEdgeDetector.o libstdc++.a $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(BOINC_SCHED_DIR)/libsched.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_LIB_DIR)/libboinc_crypt.a -pthread -mariadbclient -crypto -ssl
