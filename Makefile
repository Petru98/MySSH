################################################################
######################### Enviornment ##########################
################################################################
# Shell commands
CP := cp -f

# The names of the final binary files
export SERVERNAME := myssh-server
export CLIENTNAME := myssh-client

# Paths to target root directories
export SERVERDIR := server
export CLIENTDIR := client
export SHAREDDIR := shared

DOCDIR := doc



################################################################
######################## Build targets #########################
################################################################
# Build targets
all: server client $(DOCDIR)
server: server-release
client: client-release

release: server-release client-release
debug: server-debug client-debug

server-release: $(SERVERNAME)
client-release: $(CLIENTNAME)
server-debug: $(SERVERNAME)-d
client-debug: $(CLIENTNAME)-d



# Build binary files
$(SERVERNAME): $(SERVERDIR)/$(SERVERNAME)
	@$(CP) $(SERVERDIR)/$(SERVERNAME) $(SERVERNAME)

$(CLIENTNAME): $(CLIENTDIR)/$(CLIENTNAME)
	@$(CP) $(CLIENTDIR)/$(CLIENTNAME) $(CLIENTNAME)

$(SERVERNAME)-d: $(SERVERDIR)/$(SERVERNAME)-d
	@$(CP) $(SERVERDIR)/$(SERVERNAME)-d $(SERVERNAME)-d

$(CLIENTNAME)-d: $(CLIENTDIR)/$(CLIENTNAME)-d
	@$(CP) $(CLIENTDIR)/$(CLIENTNAME)-d $(CLIENTNAME)-d



# Build server/client/shared
$(SERVERDIR)/$(SERVERNAME): $(SHAREDDIR)
	@$(MAKE) -C $(SERVERDIR) release

$(CLIENTDIR)/$(CLIENTNAME): $(SHAREDDIR)
	@$(MAKE) -C $(CLIENTDIR) release

$(SERVERDIR)/$(SERVERNAME)-d: $(SHAREDDIR)
	@$(MAKE) -C $(SERVERDIR) debug

$(CLIENTDIR)/$(CLIENTNAME)-d: $(SHAREDDIR)
	@$(MAKE) -C $(CLIENTDIR) debug

$(SHAREDDIR):
	@$(MAKE) -C $(SHAREDDIR)



# Build documentation
$(DOCDIR): Doxyfile \
     $(wildcard $(SERVERDIR)/include/*.hpp) \
     $(wildcard $(CLIENTDIR)/include/*.hpp) \
     $(wildcard $(SHAREDDIR)/include/*.hpp) \
     $(wildcard $(SERVERDIR)/include/*.cpp) \
     $(wildcard $(CLIENTDIR)/include/*.cpp) \
     $(wildcard $(SHAREDDIR)/include/*.cpp)
	@$(RM) -r $(DOCDIR) \
	;doxygen Doxyfile



# Clean intermediary files only
clean: clean-server clean-client clean-shared clean-doc

clean-release: clean-server-release clean-client-release
clean-debug:clean-server-debug clean-client-debug



clean-server:
	@$(RM) $(SERVERNAME) $(SERVERNAME)-d \
	;$(MAKE) -C $(SERVERDIR) clean

clean-client:
	@$(RM) $(CLIENTNAME) $(CLIENTNAME)-d \
	;$(MAKE) -C $(CLIENTDIR) clean



clean-server-release:
	@$(RM) $(SERVERNAME) \
	;$(MAKE) -C $(SERVERDIR) clean-release

clean-client-release:
	@$(RM) $(CLIENTNAME) \
	;$(MAKE) -C $(CLIENTDIR) clean-release



clean-server-debug:
	@$(RM) $(SERVERNAME)-d \
	;$(MAKE) -C $(SERVERDIR) clean-debug

clean-client-debug:
	@$(RM) $(CLIENTNAME)-d \
	;$(MAKE) -C $(CLIENTDIR) clean-debug



clean-shared:
	@$(MAKE) -C $(SHAREDDIR) clean

clean-doc:
	@$(RM) -r $(DOCDIR)



# Print variable
print-%:
	@echo $($*)



# Create zip file
zip:
	@zip -r myssh Makefile Doxyfile README.md doc \
		client/Makefile client/include client/src \
		server/Makefile server/include server/src \
		shared/Makefile shared/include shared/src



################################################################
############################ Others ############################
################################################################
# These targets are not files or directories
.PHONY: all release debug
.PHONY: server client server-release client-release server-debug client-debug
.PHONY: clean clean-release clean-debug clean-server clean-client clean-shared clean-doc
.PHONY: clean-server-release clean-client-release clean-server-debug clean-client-debug
.PHONY: print-% $(SHAREDDIR)
