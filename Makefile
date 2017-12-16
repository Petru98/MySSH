################################################################
######################### Enviornment ##########################
################################################################
# Shell commands
CP := cp -f

# The names of the final binary files
export SERVERNAME := myssh-server
export CLIENTNAME := myssh-client

BINS := $(SERVERNAME) $(CLIENTNAME)

# Paths to target root directories
export SERVERDIR := server
export CLIENTDIR := client
export SHAREDDIR := shared

DOCDIR := doc



################################################################
######################## Build targets #########################
################################################################
# Build all
all: $(BINS) $(DOCDIR)

# Build binary files
$(SERVERNAME): $(SERVERDIR)/$(SERVERNAME)
	@$(CP) $(SERVERDIR)/$(SERVERNAME) $(SERVERNAME)

$(CLIENTNAME): $(CLIENTDIR)/$(CLIENTNAME)
	@$(CP) $(CLIENTDIR)/$(CLIENTNAME) $(CLIENTNAME)

# Build server/client/shared
$(SERVERDIR)/$(SERVERNAME): $(SHAREDDIR)
	@$(MAKE) -C $(SERVERDIR)

$(CLIENTDIR)/$(CLIENTNAME): $(SHAREDDIR)
	@$(MAKE) -C $(CLIENTDIR)

$(SHAREDDIR):
	@$(MAKE) -C $(SHAREDDIR)

# Build documentation
$(DOCDIR): Doxyfile \
     $(wildcard $(SERVERDIR)/include/*.hpp) $(wildcard $(SERVERDIR)/include/*.h) \
     $(wildcard $(CLIENTDIR)/include/*.hpp) $(wildcard $(CLIENTDIR)/include/*.h) \
     $(wildcard $(SHAREDDIR)/include/*.hpp) $(wildcard $(SHAREDDIR)/include/*.h)
	@$(RM) -r $(DOCDIR) \
	;doxygen Doxyfile



# Clean intermediary files only
clean: clean-server clean-client clean-shared clean-doc

clean-server:
	@$(RM) $(SERVERNAME) \
	;$(MAKE) -C $(SERVERDIR) clean

clean-client:
	@$(RM) $(CLIENTNAME) \
	;$(MAKE) -C $(CLIENTDIR) clean

clean-shared:
	@$(MAKE) -C $(SHAREDDIR) clean

clean-doc:
	@$(RM) -r $(DOCDIR)



# Print variable
print-%:
	@echo $($*)



################################################################
############################ Others ############################
################################################################
# These targets are not files or directories
.PHONY: all clean clean-server clean-client clean-shared clean-doc print-% $(SHAREDDIR)
