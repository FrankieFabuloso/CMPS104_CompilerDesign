MKFILE    = Makefile
DEPSFILE  = ${MKFILE}.deps
NOINCLUDE = clean spotless
NEEDINCL  = ${filter ${NOINCLUDE}, ${MAKECMDGOALS}}
VALGRIND  = valgrind --leak-check=full --show-reachable=yes

# Definitions of list of files:
HSOURCES  = astree.h  lyutils.h  auxlib.h  stringset.h
CPPSOURCE  = astree.cpp lyutils.cpp auxlib.cpp stringset.cpp main.cpp
LSOURCES  = scanner.l
YSOURCES  = parser.y
ETCSRC    = README ${MKFILE} ${DEPSFILE}
CLGEN     = yylex.cc
HYGEN     = yyparse.h
CYGEN     = yyparse.cc
CGENS     = ${CLGEN} ${CYGEN}
ALLGENS   = ${HYGEN} ${CGENS}
EXECBIN   = oc
ALLCSRC   = ${CPPSOURCE} ${CGENS}
OBJECTS   = ${ALLCSRC:.cc=.o}
LREPORT   = yylex.output
YREPORT   = yyparse.output
IREPORT   = ident.output
REPORTS   = ${LREPORT} ${YREPORT} ${IREPORT}
ALLSRC    = ${ETCSRC} ${YSOURCES} ${LSOURCES} ${HSOURCES} ${CPPSOURCE}
TESTINS   = ${wildcard test?.in}
LISTSRC   = ${ALLSRC} ${HYGEN}
DELIV	  = ${CPPSOURCE} ${HSOURCES} ${DEPSFILE}

# Definitions of the compiler and compilation options:
GCC       = g++ -g -O0 -Wall -Wextra -std=gnu++14
MKDEPS    = g++ -MM -std=gnu++14

all : ${EXECBIN}

# Build the executable image from the object files.
${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}
	ident ${OBJECTS} ${EXECBIN} >${IREPORT}

# Build an object file form a C source file.
%.o : %.cc
	${GCC} -c $<

# Build the scanner.
${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

# Build the parser.
${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

# Make a listing from all of the sources
lis : ${LISTSRC} tests
	mkpspdf List.source.ps ${LISTSRC}
	mkpspdf List.output.ps ${REPORTS} \
		${foreach test, ${TESTINS:.in=}, \
		${patsubst %, ${test}.%, in out err}}

# Clean and spotless remove generated files.
clean :
	rm *.o *.str *.tok yylex.cc *.out *.err *.oc *.ast \
	*.output *.lexyacctrace oc yyparse.*

spotless : clean
	- rm ${EXECBIN}

# Build the dependencies file using the C preprocessor
deps : ${ALLCSRC}
	@ echo "# ${DEPSFILE} created `date` by ${MAKE}" >${DEPSFILE}
	${MKDEPS} ${ALLCSRC} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${MAKE} --no-print-directory deps

%.out %.err : %.in ${EXECBIN}
	( ${VALGRIND} ${EXECBIN} -ly -@@ $< \
	;  echo EXIT STATUS $$? 1>&2 \
	) 1>$*.out 2>$*.err

submit : 
	submit cmps104a-wm.s16 asg3 ${ALLSRC}
	
ifeq "${NEEDINCL}" ""
include ${DEPSFILE}
endif

