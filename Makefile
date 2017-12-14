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



################################################################
######################## Build targets #########################
################################################################
# Build all
all: $(BINS)

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



# Clean intermediary files only
clean: clean-server clean-client clean-shared

clean-server:
	@$(RM) $(SERVERNAME) \
	;$(MAKE) -C $(SERVERDIR) clean

clean-client:
	@$(RM) $(CLIENTNAME) \
	;$(MAKE) -C $(CLIENTDIR) clean

clean-shared:
	@$(MAKE) -C $(SHAREDDIR) clean



# Print variable
print-%:
	@echo $($*)



################################################################
############################ Others ############################
################################################################
# These targets are not files or directories
.PHONY: all clean clean-server clean-client clean-shared print-% $(SHAREDDIR)
