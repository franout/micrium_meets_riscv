RISCV_ISA ?= "rv32i_zicsr"
RISCV_ABI ?= "ilp32"
OPTIMIZATION_FLAGS  ?= "-O0"

program ?= $(program)
CC        ?= ggcc
LD 		  ?= gcc
SRCDIR    := ./Software
HEADERDIR := ./Software
BUILDDIR  := ./build
TARGET    := $(program).elf
SOURCES   := $(shell find $(SRCDIR) -type f -name *.c* )
ASSEMBLY  := $(shell find $(SRCDIR) -type f -name *.s -or -name *.S)
HEDEARS   := $(shell find $(HEADERDIR) -type f -name *.h*)
OBJECTS   := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(addsuffix .o,$(basename $(SOURCES))))
DEPS      := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(addsuffix .d,$(basename $(SOURCES))))


CFLAGS :=  $(OPTIMIZATION_FLAGS) -mcmodel=medlow -march=$(RISCV_ISA) -mabi=$(RISCV_ABI)  -static -lc -Wall -Wextra   -L$$CC_INSTALLATION_PATH/lib   --entry=_start --static   -fno-exceptions -fPIC
LDFLAG :=  $(OPTIMIZATION_FLAGS)    -Wl,-Map=$(BUILDDIR)/$(program).map,--cref,--no-warn-mismatch -lc -T ./Software/generic/RISCV-platform/linker.ld -Xlinker --defsym=__stack_size=0xA00 -Wl,--entry=_start --static -fno-exceptions -fPIC 
CXXFLAGS := $(OPTIMIZATION_FLAGS) -Wall -Wextra 
COBJFLAGS := $(CFLAGS) -c


## verbsely add includes 
INC       := -I$(SRCDIR)/generic/CSP/Comet/ -I$(SRCDIR)/uC-CPU/ -I$(SRCDIR)/uC-CPU/RISC-V/GCC/ -I$(SRCDIR)/generic/generic/riscv32/micrium_meets_bernina/application/ -I$(SRCDIR)/generic/generic/riscv32/micrium_meets_bernina/Cfg/ -I$(SRCDIR)/generic/generic/riscv32/micrium_meets_bernina/ -I$(SRCDIR)/uC-Clk/Cfg/ -I$(SRCDIR)/uC-Clk/Source/ -I$(SRCDIR)/uC-CPU/Cfg/  -I$(SRCDIR)/uC-LIB/Cfg/ -I$(SRCDIR)/uC-LIB/ -I$(SRCDIR)/uC-OS3/Cfg/ -I$(SRCDIR)/uC-OS3/Ports/RISC-V/RV32/GCC/ -I$(SRCDIR)/uC-OS3/Source/ -I./Software/generic/RISCV-platform/


all: debug
debug: CFLAGS += -g
debug: $(TARGET)
release: $(TARGET)
release: CFLAGS += -O3

GREEN=`tput setaf 2`
RESET=`tput sgr0`

define print_green
	@echo "$(GREEN)$(1)$(RESET)"
endef

all: $(TARGET)

clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET)

$(TARGET):  $(BUILDDIR) $(OBJECTS) $(HEDEARS)
	$(call print_green,"Linking object files...")
	@ echo $(CC) $(LDFLAG) $(CFLAGS) $(INC)  $(OBJECTS) $(ASSEMBLY)  -o $(TARGET)
	$(CC) $(LDFLAG) $(CFLAGS) $(INC)  $(OBJECTS) $(ASSEMBLY)  -o $(TARGET) 
	$(call print_green,"$(TARGET) has been created!")

$(BUILDDIR) :
	mkdir $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c* 
	@mkdir -p $(dir $@)
	$(CC)  $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC)  $(CFLAGS) $(INC) -M $< -MT $@ > $(@:.o=.td)
	@cp $(@:.o=.td) $(@:.o=.d); 
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.td) >> $(@:.o=.d); 
	@rm -f $(@:.o=.td)


-include $(DEPS)

.PHONY: clean all