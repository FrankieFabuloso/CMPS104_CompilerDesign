# Reworking of this makefile was made with help of private tutor.

MKFILE    = Makefile
DEPSFILE  = ${MKFILE}.deps
NOINCLUDE = submit testsubmit clean spotless
NEEDINCL  = ${filter ${NOINCLUDE}, ${MAKECMDGOALS}}
VALGRIND  = valgrind --leak-check=full --show-reachable=yes

#
# Definitions of list of files:
#
MODULES    = icmake typechecker astree lyutils auxlib stringset
HSOURCES   = ${MODULES:=.h}
CSOURCES   = ${MODULES:=.cpp} main.cpp 
LSOURCES  = scanner.l
YSOURCES  = parser.y
ETCSRC    = README ${MKFILE} ${DEPSFILE}
ETCSUB    = README ${MKFILE}
CLGEN     = yylex.cpp
HYGEN     = yyparse.h
CYGEN     = yyparse.cpp
CGENS     = ${CLGEN} ${CYGEN}
ALLGENS   = ${HYGEN} ${CGENS}
EXECBIN   = oc
ALLCSRC   = ${CSOURCES} ${CGENS}
OBJECTS   = ${ALLCSRC:.cpp=.o}
LREPORT   = yylex.output
YREPORT   = yyparse.output
REPORTS   = ${LREPORT} ${YREPORT}
ALLSRC    = ${ETCSRC} ${YSOURCES} ${LSOURCES} ${CSOURCES} ${HSOURCES}
SUBSRC    = ${ETCSUB} ${LSOURCES} ${YSOURCES} ${CSOURCES} ${HSOURCES}

#
# Definitions of the compiler and compilation options:
#
GCC       = g++ -g -O0 -Wall -Wextra -std=gnu++14 -fdiagnostics-color
MKDEPS    = g++ -MM -std=gnu++14

#
# Definitions for submission of given assignment
#
CLASS       = cmps104a-wm.s16
PROJECT     = asg5

all: ${EXECBIN}

${EXECBIN}: ${CLGEN} ${CYGE{} ${OBJECTS}
	${GCC} -o ${EXECBIN} ${OBJECTS}

#
# Build an object file form a C source file.
#
%.o: %.cpp
	${GCC} -c $<

#
# Build the scanner.
#
${CLGEN}: ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

#
# Build the parser.
#
${CYGEN} ${HYGEN}: ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

#
# Clean and spotless. Spotless removes the executable.
#
clean:
	- rm ${OBJECTS} ${ALLGENS} ${REPORTS} ${DEPSFILE}

spotless: clean
	- rm ${EXECBIN}
	- rm -r ${TESTDIR}
	- rm *.str
	- rm *.tok
	- rm *.ast
	- rm *.sym
	- rm *.err
	- rm *.out
	- rm *.oc

#
# Build the dependencies file using the C preprocessor
#
deps: ${ALLCSRC}
	@ echo "# ${DEPSFILE} created `date` by ${MAKE}" >${DEPSFILE}
	${MKDEPS} ${ALLCSRC} >>${DEPSFILE}

${DEPSFILE}:
	@ touch ${DEPSFILE}
	${MAKE} --no-print-directory deps


#
# Real Submission (will fail if not on unix.ucsc.edu)
#
submit: ${SUBSRC} spotless
	submit ${CLASS} ${PROJECT} ${SUBSRC}

#
# Everything
#
again :
	gmake --no-print-directory spotless deps all
	
ifeq "${NEEDINCL}" ""
include ${DEPSFILE}
endif

