################################################################
######################### Enviornment ##########################
################################################################
# Shell commands
RM := rm -f
MV := mv -f
MKDIR := mkdir -p

# The name of the final binary file
ifdef CLIENTNAME
    NAME := $(CLIENTNAME)
else
    NAME := $(shell $(MAKE) -s -C .. print-CLIENTNAME)
endif



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

ifneq ( , $(findstring debug,$(MAKECMDGOALS)))
    TARGETMODE := debug
else
    TARGETMODE := release
endif



# Root directories for object/dependency files
OBJDIR := .obj
DEPDIR := .d



ifneq (clean,$(TARGET))
    # C/C++ compiler, linker and assembler
    CC  := gcc
    CXX := g++
    LD  := g++



    # The directory with all the shared source/object files
    ifdef SHAREDDIR
        SHAREDDIR := ../$(SHAREDDIR)
    else
        SHAREDDIR := ../$(shell $(MAKE) -s -C .. print-SHAREDDIR)
    endif

    SHAREDINCDIR := $(SHAREDDIR)/include
    SHAREDOBJDIR := $(SHAREDDIR)/.obj
    SHAREDOBJS   := PgpSocket Socket IpAddress CommandLine FileDescriptor Logging error Lock Mutex tinyxml2
    SHAREDOBJS   := $(shell find $(SHAREDOBJDIR) -type f -name "*.o" | grep $(addprefix -e ,$(SHAREDOBJS)))



    # Root directory for source files & additional include/library directories
    SRCDIR := src
    INCDIR := include $(SHAREDINCDIR)
    LIBDIR := lib

    # Target directories for object/dependency files
	ifeq (build,$(TARGET))
        OBJDIR := $(OBJDIR)/$(TARGETMODE)
        DEPDIR := $(DEPDIR)/$(TARGETMODE)
    endif



    # Full paths to source files & libraries
    SRCS := $(shell find $(SRCDIR) -type f -name "*.cpp") \
            $(shell find $(SRCDIR) -type f -name "*.c")

    LIBS := crypto++ pthread

    # Full paths to object/dependency files
    OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.cpp.o,$(filter %.cpp,$(SRCS))) \
            $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(filter %.c,$(SRCS)))

    DEPS := $(patsubst $(SRCDIR)/%.cpp,$(DEPDIR)/%.cpp.d,$(filter %.cpp,$(SRCS))) \
            $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.c.d,$(filter %.c,$(SRCS)))



    # Preprocessor/C/C++/Linker flags
    CPPFLAGS :=
    CFLAGS   := -Wall -Wextra
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
all: release

release: $(NAME)
debug: $(NAME)-d



# Build binary files
$(NAME): CPPFLAGS += -DNDEBUG
$(NAME): CFLAGS += -DNDEBUG -O3
$(NAME): CXXFLAGS += -DNDEBUG -O3

$(NAME): $(OBJS)
	@echo "Linking"     \
	;$(MKDIR) $(dir $@) \
	;$(LD) $(LDFLAGS) -o $@ $^ $(SHAREDOBJS) $(LDLIBS)

$(NAME)-d: CFLAGS += -ggdb
$(NAME)-d: CXXFLAGS += -ggdb

$(NAME)-d: $(OBJS)
	@echo "Linking"     \
	;$(MKDIR) $(dir $@) \
	;$(LD) $(LDFLAGS) -o $@ $^ $(SHAREDOBJS) $(LDLIBS)



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
	@$(RM) -r $(OBJDIR) $(DEPDIR) $(NAME) $(NAME)-d
clean-release:
	@$(RM) -r $(OBJDIR)/release $(DEPDIR)/release $(NAME)
clean-debug:
	@$(RM) -r $(OBJDIR)/debug $(DEPDIR)/debug $(NAME)-d



################################################################
############################ Others ############################
################################################################
# Prevent make from deleting the dependency files
.PRECIOUS: %.d

# The next targets are not files
.PHONY: all release debug clean clean-release clean-debug print-%

# Include dependecies
ifeq (build,$(TARGET))
    -include $(DEPS)
endif
