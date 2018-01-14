################################################################
######################### Enviornment ##########################
################################################################
# Shell commands
RM := rm -f
MV := mv -f
MKDIR := mkdir -p



################################################################
########################## Initialize ##########################
################################################################
ifneq ( , $(findstring clean,$(MAKECMDGOALS)))
    TARGET := clean
else ifneq ( , $(findstring print,$(MAKECMDGOALS)))
    TARGET := print
else
    TARGET := build
endif



# Root directories for object/dependency files
OBJDIR := .obj
DEPDIR := .d



ifneq (clean,$(TARGET))
    # C/C++ compiler, linker and assembler
    CC  := gcc
    CXX := g++
    LD  := g++



    # Root directory for source files & additional include/library directories
    SRCDIR := src
    INCDIR := include
    LIBDIR := lib



    # Full paths to source files & libraries
    SRCS := $(shell find $(SRCDIR) -type f -name "*.cpp") \
            $(shell find $(SRCDIR) -type f -name "*.c")

    LIBS :=

    # Full paths to object/dependency files
    OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.cpp.o,$(filter %.cpp,$(SRCS))) \
            $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(filter %.c,$(SRCS)))

    DEPS := $(patsubst $(SRCDIR)/%.cpp,$(DEPDIR)/%.cpp.d,$(filter %.cpp,$(SRCS))) \
            $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.c.d,$(filter %.c,$(SRCS)))



    # Preprocessor/C/C++/Linker flags
    CPPFLAGS := -DNDEBUG
    CFLAGS   := -Wall -Wextra -O3
    CXXFLAGS := -std=c++14
    LDFLAGS  :=

    CPPFLAGS += $(addprefix -I,$(INCDIR))
    CFLAGS   += $(CPPFLAGS)
    CXXFLAGS += $(CFLAGS)
    LDFLAGS  += $(addprefix -L,$(LIBDIR))

    # Link libraries commands
    LDLIBS := $(addprefix -l,$(LIBS))
endif



################################################################
######################## Prepare files #########################
################################################################
# Create the needed directories if they don't exist
ifeq (build,$(TARGET))
    $(shell $(MKDIR) $(OBJDIR) $(DEPDIR) >> /dev/null)
endif



################################################################
######################## Build targets #########################
################################################################
# Build all
all: $(OBJS)



# Build object file
$(OBJDIR)/%.cpp.o : $(DEPDIR)/%.cpp.d
	$(eval FILELOC := $(patsubst $(OBJDIR)/%,%,$(basename $@)))
	$(eval SRCFILE := $(SRCDIR)/$(FILELOC))
	$(eval OBJFILE := $(OBJDIR)/$(FILELOC).o)
	$(eval DEPFILE := $(DEPDIR)/$(FILELOC).d)

	@echo "Compiling $(SRCFILE)" \
	;$(MKDIR) $(dir $(OBJFILE))  \
	;$(CXX) $(CXXFLAGS) -o $(OBJFILE) -c $(SRCFILE)

$(OBJDIR)/%.c.o : $(DEPDIR)/%.c.d
	$(eval FILELOC := $(patsubst $(OBJDIR)/%,%,$(basename $@)))
	$(eval SRCFILE := $(SRCDIR)/$(FILELOC))
	$(eval OBJFILE := $(OBJDIR)/$(FILELOC).o)
	$(eval DEPFILE := $(DEPDIR)/$(FILELOC).d)

	@echo "Compiling $(SRCFILE)" \
	;$(MKDIR) $(dir $(OBJFILE))  \
	;$(CC) $(CFLAGS) -o $(OBJFILE) -c $(SRCFILE)



# Build dependency file
$(DEPDIR)/%.cpp.d : $(SRCDIR)/%.cpp
	$(eval FILELOC := $(patsubst $(DEPDIR)/%,%,$(basename $@)))
	$(eval SRCFILE := $(SRCDIR)/$(FILELOC))
	$(eval OBJFILE := $(OBJDIR)/$(FILELOC).o)
	$(eval DEPFILE := $(DEPDIR)/$(FILELOC).d)

	@$(MKDIR) $(dir $(DEPFILE)) \
	;$(CXX) $(CXXFLAGS) -MM -MP -MT $(OBJFILE) -MF $(DEPFILE)~ $(SRCFILE) \
	;$(MV) $(DEPFILE)~ $(DEPFILE)

$(DEPDIR)/%.c.d : $(SRCDIR)/%.c
	$(eval FILELOC := $(patsubst $(DEPDIR)/%,%,$(basename $@)))
	$(eval SRCFILE := $(SRCDIR)/$(FILELOC))
	$(eval OBJFILE := $(OBJDIR)/$(FILELOC).o)
	$(eval DEPFILE := $(DEPDIR)/$(FILELOC).d)

	@$(MKDIR) $(dir $(DEPFILE)) \
	;$(CC) $(CFLAGS) -MM -MP -MT $(OBJFILE) -MF $(DEPFILE)~ $(SRCFILE) \
	;$(MV) $(DEPFILE)~ $(DEPFILE)



# Print variable
print-%:
	@echo $($*)



# Clean intermediary files only
clean:
	@$(RM) -r $(OBJDIR) $(DEPDIR)



################################################################
############################ Others ############################
################################################################
# Prevent make from deleting the dependency files
.PRECIOUS: %.d

# The next targets are not files
.PHONY: all clean print-%

# Include dependecies
ifeq (build,$(TARGET))
    -include $(DEPS)
endif
