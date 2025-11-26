#CCI Debugger
Application to perform i2c read / write using the cci interface

##Usage

ccidbgr [-dev=<cciDev>] [-master=<cciMaster>]

##Options

-dev=<cciDev>        CCI Device ID [default: 0]. 10

-master=<cciMaster>  CCI Master on CCI Device [default: 0].

-infile=filename [-outfile=filename]
  -infile  - file containing register sequences (read/write)
  -outfile - optional file where results are written (default is console)

##Debugging Instructions
1. Ensure ais_server is running

2. Follow the menu options if it is invoked in interactive mode
 a. First set slave address [8bit format], address type [# of bytes], and data type [# of bytes]
 through cci_update
 b. Use read/write/write_sync_array commands

3. Create input file when it is invoked in file mode using '-infile=' option.
 3.1 Input file format
   a. up: slaveaddr [8bit format] addrtype [# of bytes] datatype [# of bytes]
   b. wr: addr [addr format from addrtype] data [data format from datatype] delay [# of usec]
   c. rd: addr [addr format from addrtype]
   d. Example:
    up: slaveaddr 0xc4 addrtype 2 datatype 1
    wr: addr 0x0 data 0x82 delay 0
    up: slaveaddr 0x82 addrtype 2 datatype 1
    rd: addr 0x10
 3.2 Provide output file name to redirect output to file. Output goes to console if
     output file is not specified.

