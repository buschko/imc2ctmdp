OSTYPE		:=	$(shell uname -s | tr '[:upper:]' '[:lower:]' | \
sed \
-e 's/cygwin.*/cygwin/' \
-e 's/irix../irix/' \
-e 's/windows.*/windows/' \
-e 's/mingw.*/mingw/')

ifeq ($(OSTYPE),darwin)
	CXX           = /usr/local/bin/g++-4.9
else
ifeq ($(OSTYPE),linux)
	CXX           = /usr/bin/g++
endif
endif
#CXX           = $(CADP)/src/com/cadp_cc
CXXDEBUG      = -g -DNO_EXCEPTION_CATCH -DDEBUG -D_GLIBCXX_CONCEPT_CHECKS
CXXOPTIMIZE   = -O2 -fno-strength-reduce -DNDEBUG
CXXOPTIONS    = -Wall -W -Wpointer-arith -Winline -m32
ifeq ($(OSTYPE),nt_nocygwin)
  CXXOPTIONS += -mno-cygwin -DNT_NOCYGWIN -DGETTIMEOFDAY_IS_ABSENT
endif
#CXXOPT_LINUX  = -march=pentiumpro -malign-double
CXXOPT_SOLARIS= -mhard-float -mv8
CXXOPT_NT_NOCYGWIN =
CXXINCLUDE    = -I$(CADP)/incl
ifeq ($(OSTYPE),nt_nocygwin)
  CXXLINK     = -mno-cygwin -L$(CADP)/bin.win32 -lBCG_IO -lBCG -lm
else
ifeq ($(OSTYPE),darwin)
  CXXLINK     = -L$(CADP)/bin.mac86 -m32 -lBCG_IO -lBCG -lm
else
ifeq ($(OSTYPE),linux)
	CXXLINK     = -L$(CADP)/bin.x64 -lBCG_IO -lBCG -lm
endif
endif
endif
ifeq ($(OSTYPE),nt_nocygwin)
  CADP       ?= /Programme/cadp
else
ifeq ($(OSTYPE),darwin)
  CADP       ?= /lib/cadp
else
ifeq ($(OSTYPE),linux)
	CADP       ?= /lib/cadp
endif
endif
endif

# if you want debug options, uncomment the following line
#DEBUG?=1

ifneq ($(OSTYPE),nt_nocygwin)
# if you want to get the used memory from mallsize(), uncomment the following
  #CXXOPTIONS += -DMALLINFO
endif

CXXFLAGS      = $(CXXOPTIONS) $(CXXINCLUDE)
ifdef DEBUG
  CXXFLAGS   += $(CXXDEBUG)
else
  CXXFLAGS   += $(CXXOPTIMIZE)
endif

ifeq ($(OSTYPE),nt_nocygwin)
  CXXFLAGS   += $(CXXOPT_NT_NOCYGWIN)
else
ifeq ($(OSTYPE),solaris)
  CXXFLAGS   += $(CXXOPT_SOLARIS)
else
  CXXFLAGS   += $(CXXOPT_LINUX)
endif
endif

OBJ_INPUTS    = GraphInputBCG.o GraphInputPrism.o
OBJ_OUTPUTS   = GraphOutputBCG.o GraphOutputCtmdp.o GraphOutputETMCC.o \
                GraphOutputMarked.o GraphOutputLab.o
OBJ_GENERAL   = State.o Transition.o Graph.o Label.o GraphInput.o \
                GraphOutput.o

OBJ_IMC2CTMDP = imc2ctmdp.o $(OBJ_GENERAL) $(OBJ_INPUTS) $(OBJ_OUTPUTS)

###############################################################################

.PHONY: clean all doc imc2mrmc

# default target: imc2ctmdp

imc2ctmdp: $(OBJ_IMC2CTMDP)
	$(CXX) -o $@ $^ $(CXXLINK)

imc2mrmc:
	@make -C $@

all: imc2ctmdp imc2mrmc

.cc.o:
	$(CXX) -c $< $(CXXFLAGS)

clean:
	@make -C doc clean
	@make -C imc2mrmc clean
	rm -f *.o

doc:
	@make -C doc/

depend:
	@echo "Updating dependencies: "
	@sed -e '1,/^### SOURCE FILE DEPENDENCIES GO BELOW/ !D' Makefile >Makefile.new
	@-$(CXX) -MM *.cc $(CXXINCLUDE) | sed 's�$(CADP)�$$(CADP)�' >>Makefile.new
	@mv -f Makefile.new Makefile
	@echo -e "\e[A\e[68C \e[34;01m[\e[32;01m updated \e[34;01m]\e[0m"


### DO NOT DELETE THE NEXT LINE!!!
### SOURCE FILE DEPENDENCIES GO BELOW
Graph.o: Graph.cc Graph.h State.h Transition.h Label.h imc2ctmdp.h \
  GraphInput.h GraphOutput.h
GraphInput.o: GraphInput.cc GraphInput.h
GraphInputBCG.o: GraphInputBCG.cc GraphInputBCG.h GraphInput.h Graph.h \
  $(CADP)/incl/bcg_user.h $(CADP)/incl/bcg_area.h \
  $(CADP)/incl/bcg_standard.h \
  $(CADP)/incl/caesar_system.h \
  $(CADP)/incl/bcg_temporary.h $(CADP)/incl/bcg_types.h \
  $(CADP)/incl/bcg_io_write_bcg.h \
  $(CADP)/incl/bcg_types.h $(CADP)/incl/bcg_edges.h \
  $(CADP)/incl/bcg_states.h $(CADP)/incl/bcg_transition.h \
  $(CADP)/incl/bcg_edge_sort.h \
  $(CADP)/incl/bcg_structure_2.h $(CADP)/incl/bcg_table.h \
  $(CADP)/incl/bcg_list.h $(CADP)/incl/bcg_iterator.h \
  $(CADP)/incl/bcg_edge_table_1.h \
  $(CADP)/incl/bcg_binary_table.h \
  $(CADP)/incl/bcg_edge_table_2.h \
  $(CADP)/incl/bcg_options.h State.h Transition.h Label.h
GraphInputPrism.o: GraphInputPrism.cc GraphInputPrism.h GraphInput.h \
  Graph.h State.h Transition.h Label.h
GraphOutput.o: GraphOutput.cc GraphOutput.h
GraphOutputBCG.o: GraphOutputBCG.cc GraphOutputBCG.h GraphOutput.h \
  Graph.h $(CADP)/incl/bcg_user.h $(CADP)/incl/bcg_area.h \
  $(CADP)/incl/bcg_standard.h \
  $(CADP)/incl/caesar_system.h \
  $(CADP)/incl/bcg_temporary.h $(CADP)/incl/bcg_types.h \
  $(CADP)/incl/bcg_io_write_bcg.h \
  $(CADP)/incl/bcg_types.h $(CADP)/incl/bcg_edges.h \
  $(CADP)/incl/bcg_states.h $(CADP)/incl/bcg_transition.h \
  $(CADP)/incl/bcg_edge_sort.h \
  $(CADP)/incl/bcg_structure_2.h $(CADP)/incl/bcg_table.h \
  $(CADP)/incl/bcg_list.h $(CADP)/incl/bcg_iterator.h \
  $(CADP)/incl/bcg_edge_table_1.h \
  $(CADP)/incl/bcg_binary_table.h \
  $(CADP)/incl/bcg_edge_table_2.h \
  $(CADP)/incl/bcg_options.h State.h Transition.h Label.h
GraphOutputCtmdp.o: GraphOutputCtmdp.cc GraphOutputCtmdp.h GraphOutput.h \
  Graph.h State.h Transition.h Label.h
GraphOutputETMCC.o: GraphOutputETMCC.cc GraphOutputETMCC.h GraphOutput.h \
  Graph.h State.h Transition.h Label.h
GraphOutputLab.o: GraphOutputLab.cc GraphOutputLab.h GraphOutput.h \
  Graph.h State.h Transition.h Label.h
GraphOutputMarked.o: GraphOutputMarked.cc GraphOutputMarked.h \
  GraphOutput.h Graph.h State.h Transition.h Label.h
Label.o: Label.cc Label.h Graph.h
State.o: State.cc State.h Transition.h Label.h Graph.h
Transition.o: Transition.cc Transition.h Label.h
imc2ctmdp.o: imc2ctmdp.cc imc2ctmdp.h Graph.h
