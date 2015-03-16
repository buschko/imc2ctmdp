#CXX           = /usr/bin/g++
CXX	      = /usr/local/bin/g++-4.8
CADP          = /Users/guckd/lib/cadp
CXXDEBUG      = -g -DNO_EXCEPTION_CATCH -DDEBUG
CXXOPTIMIZE   = -O2 -fno-strength-reduce -DNDEBUG
CXXOPTIONS    = -Wall -W -Wpointer-arith -Winline
#CXXOPT_LINUX  = -march=pentiumpro -malign-double
CXXOPT_SOLARIS= -mhard-float -mv8
CXXINCLUDE    = -I$(CADP)/incl
CXXLINK       = -L$(CADP)/bin.mac86 -lBCG_IO -lBCG -lm
#CXXFLAGS      = $(CXXDEBUG) $(CXXOPTIONS) $(CXXOPT_LINUX) $(CXXINCLUDE)
#CXXFLAGS      = $(CXXDEBUG) $(CXXOPTIONS) $(CXXOPT_SOLARIS) $(CXXINCLUDE)
CXXFLAGS      = $(CXXOPTIMIZE) $(CXXOPTIONS) $(CXXOPT_LINUX) $(CXXINCLUDE)
#CXXFLAGS      = $(CXXOPTIMIZE) $(CXXOPTIONS) $(CXXOPT_SOLARIS) $(CXXINCLUDE)

OBJ_INPUTS    = GraphInputBCG.o GraphInputPrism.o
OBJ_GENERAL   = State.o Transition.o Graph.o Label.o GraphInput.o \
                GraphOutput.o

OBJ_PATH      = ..
OBJ_IMC2MRMC  = $(foreach obj,$(OBJ_GENERAL) $(OBJ_INPUTS),$(OBJ_PATH)/$(obj))
MY_OBJ        = imc2mrmc.o

LIB           = imc2mrmc.a

#LINKED_LIBS   = $(CADP)/bin.mac86/libBCG.a $(CADP)/bin.mac86/libBCG_IO.a

###############################################################################

.PHONY: clean all

# default target: all
all: $(LIB)

$(LIB): $(OBJ_IMC2MRMC) $(MY_OBJ) $(LINKED_LIBS)
	@rm -f $@
	@ar rv $@ $(OBJ_IMC2MRMC) $(MY_OBJ)
	@for lib in $(LINKED_LIBS); do \
	   ar x $$lib && \
		 ar t $$lib | xargs ar rv $@ && \
		 ar t $$lib | xargs rm -f ; \
	 done

$(LINKED_LIBS):
	@echo "Error! File not found: $@"
	@false

$(OBJ_PATH)/%.o: $(OBJ_PATH)/%.cc $(OBJ_PATH)/%.h
	@make -C $(OBJ_PATH) $(@:$(OBJ_PATH)/%=%)

.c.o:
	gcc -O2 -c -o $@ $<

.cc.o:
	$(CXX) -c $< $(CXXFLAGS)

clean:
	rm -f $(LIB) $(MY_OBJ)
