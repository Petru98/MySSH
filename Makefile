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



################################################################
######################## Build targets #########################
################################################################
# Build all
all: $(BINS)

# Build binary files
$(SERVERNAME): $(SERVERDIR)
	@$(CP) $(SERVERDIR)/$(SERVERNAME) $(SERVERNAME)

$(CLIENTNAME): $(CLIENTDIR)
	@$(CP) $(CLIENTDIR)/$(CLIENTNAME) $(CLIENTNAME)

# Bulid server/client
$(SERVERDIR):
	@$(MAKE) -C $(SERVERDIR)

$(CLIENTDIR):
	@$(MAKE) -C $(CLIENTDIR)



# Clean intermediary files only
clean: clean-server clean-client

clean-server:
	@$(MAKE) -C $(SERVERDIR) clean

clean-client:
	@$(MAKE) -C $(CLIENTDIR) clean



# Print variable
print-%:
	@echo $($*)



################################################################
############################ Others ############################
################################################################
# These targets are not files or directories
.PHONY: all clean clean-server clean-client print-%
