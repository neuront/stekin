ifeq ($(MODE), inspect)
	OPT_FLAGS=
	DYN_LINK=-rdynamic
else
	OPT_FLAGS=-O3
	DYN_LINK=
endif

CC=g++ -c -std=c++0x
INCLUDE=-I.
RESOLVE_DEP=g++ -MM $(INCLUDE)
LINK=g++ $(DYN_LINK)
AR=ar rcs

CFLAGS=-Wall -Wextra -Wold-style-cast -Werror $(OPT_FLAGS)
MKTMP := $(shell mktemp)

LIB_DIR=libs
EXTERN_LIBS=-lgmp -lgmpxx -ltrace -lbacktracpp
TEST_EXTERN_LIBS=-lgtest -lgtest_main -lpthread
LIBS=-L$(LIB_DIR) -lstkn $(EXTERN_LIBS)
TEST_LIBS=-L$(LIB_DIR) -lstkntest -lstkn $(TEST_EXTERN_LIBS) $(EXTERN_LIBS)

SAMPLEDIR=samples
ERRSAMPLEDIR=$(SAMPLEDIR)/errors
CHEKC_MEMONRY=valgrind --log-file=tmp.log.memcheck --leak-check=full

COMPILE=$(CC) $(CFLAGS) $(INCLUDE)
COMPILE_GENERATED=$(CC) $(INCLUDE)

%.d:$(WORKDIR)/%.cpp
	echo -n "$(WORKDIR)/" > $(MKTMP)
	$(RESOLVE_DEP) $< >> $(MKTMP)
	echo "	$(COMPILE) $< -o $(WORKDIR)/$*.o" >> $(MKTMP)
	make -f $(MKTMP)

%.dt:$(TESTDIR)/%.cpp
	echo -n "$(TESTDIR)/" > $(MKTMP)
	$(RESOLVE_DEP) $< >> $(MKTMP)
	echo "	$(COMPILE) $< -o $(TESTDIR)/$*.o" >> $(MKTMP)
	make -f $(MKTMP)
