GPP   		= g++ -g -O0 -Wall -Wextra -std=gnu++14
MAKEDEPCPP  = g++ -MM -std=gnu++14
GRIND 		= valgrind --leak-check=full --show-reachable=yes
DEPFILE     = makefile.dep
GMAKE       = ${MAKE} --no-print-directory

MODULES     = auxlib stringset
CPPHEADER   = ${MODULES:=.h}
CPPSOURCE   = ${MODULES:=.cpp} main.cpp
ASGN		= asg1
DELIV		= ${CPPSOURCE} ${CPPHEADER}

all : oc

test : oc
	oc test.oc

oc : main.o stringset.o auxlib.o
	${GPP} main.o stringset.o auxlib.o -o oc

%.o : %.cpp
	${GPP} -c $<

clean :
	-rm stringset.o main.o auxlib.o

spotless : clean
	- rm oc 

test.out : oc
	${GRIND} oc * * * >test.out 2>&1

lis : test.out
	mkpspdf Listing.ps stringset.h stringset.cpp main.cpp \
	        Makefile test.out

deps : ${CPPSOURCE} ${CPPHEADER} ${GENFILES}
	@ echo "# ${DEPFILE} created `LC_TIME=C date`" >${DEPFILE}
	${MAKEDEPCPP} ${CPPSOURCE} >>${DEPFILE}

${DEPFILE} :
	@ touch ${DEPFILE}
	${GMAKE} dep

submit : 
	submit cmps104a-wm.s16 ${ASGN} ${DELIV} Makefile README

