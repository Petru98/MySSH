# MySSH
## Prerequisites
### Libraries
- Crypto++ (tested version 5.6.4)

### Others
- Linux (tested on Debian 9)
- GCC with c++14 support (tested version 6.3.0)
- Doxygen
- GNU Make



## Build
To build everything (the server, the client and the documentation) run `make` or `make all`. To delete all files created by the previous commands (including the binary files) you can use `make clean`.

Here is a list of more commands you can use to build/clean parts of the project separately:
- documentation: `make doc` / `make clean-doc`
- server: `make server` / `make clean-server`
- client: `make client` / `make clean-client`
- binary files (server an client): `make release` / `make clean-release`



## Usage
### Server
Usage: `./myssh-server [OPTIONS]`

*OPTIONS* can be any combination of the following (or none of them):
- `-p PORT` or `--port PORT` (Listen to *PORT* for connections; **1100** by default)
- `-d FILE` or `--database FILE` (Use *FILE* as the database; **db.xml** by default)

The database represents an XML file. If the database file you specified (or the default one, in case you didn't specify any file) doesn't exist it will be created if needed. Also, the server will create a directory called **home** where the user's directories will exist.

After running the command above (and no errors have occurred) you will see a right arrow (`>`). This means that the server can now accept connections from clients and you can run the following commands:
- `exit` - close server
- `info` - get information about the server (you can use this to get the port)
- `adduser NAME PASSWORD` - add a new user account with the name *NAME* and password *PASSWORD* to the database
- `rmuser NAME` - Removes the user with the name NAME from database

In order for clients to log in, they need an account which you must create using **adduser**. Once a user logs in, they will be able to run commands in their home directory (**home/NAME** where *NAME* is the user's name).

### Client
Usage: `./myssh-client [IP] [PORT]`

*IP* represents the server's IP address (127.0.0.1 by default) and *PORT* represents the port the server is listening to (1100 by default).

Once the initialization is done and the client connects to the server, you will be prompted for your user name and password. In order to use an account it must exist in the server's database. After logging in successfully, you will be able to run non-interactive commands in a special folder on the server which represents your home directory.

You can link multiple commands with the following:
- `COMMAND1 ; COMMAND2 [...]` - execute commands in order, from left to right



## Bibliography
- https://profs.info.uaic.ro/~computernetworks/files/NetEx/S12/ServerConcThread/servTcpConcTh2.c
- https://profs.info.uaic.ro/~computernetworks/files/NetEx/S9/servTcpCSel.c
