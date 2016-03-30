CC = /opt/csw/gcc4/bin/gcc
# added -fno-builtin 2008-02-28 to get rid of warning messages
CFLAGS = -Wall -pedantic -g -fno-builtin
# LIBS = 
# INCLUDE =

APPS = temp_date.exe gas_date.exe gas_update_gas.exe temp_update_temps.exe temp_r.exe temp_paulsen_r.exe 

all: $(APPS)

clean:
	rm -f *.o *.bak $(APPS) core

depend:
	makedepend -- $(CFLAGS) -- $(INCLUDE) -- *.c

gas_output.exe: gas_output.o julian.o
	$(CC) $(CFLAGS) $(INCLUDE) gas_output.o julian.o -o gas_output.exe

temp_output.exe: temp_output.o julian.o
	$(CC) $(CFLAGS) $(INCLUDE) temp_output.o julian.o -o temp_output.exe

temp_date.exe: temp_date.o julian.o
	$(CC) $(CFLAGS) $(INCLUDE) temp_date.o julian.o -o temp_date.exe

gas_date.exe: gas_date.o julian.o
	$(CC) $(CFLAGS) $(INCLUDE) gas_date.o julian.o -o gas_date.exe

gas_update_gas.exe: gas_update_gas.o julian.o
	$(CC) $(CFLAGS) $(INCLUDE) gas_update_gas.o julian.o -o gas_update_gas.exe

temp_rtoutput.exe: temp_rtoutput.o
	$(CC) $(CFLAGS) $(INCLUDE) temp_rtoutput.o -o temp_rtoutput.exe

temp_update_temps.exe: temp_update_temps.o
	$(CC) $(CFLAGS) $(INCLUDE) temp_update_temps.o -o temp_update_temps.exe

temp_r.exe: temp_r.o nrutil.o  temp_r_read_dat.o
	$(CC) $(CFLAGS) $(INCLUDE) temp_r.o nrutil.o  temp_r_read_dat.o -o temp_r.exe

temp_paulsen_r.exe: temp_paulsen_r.o nrutil.o  temp_r_read_dat.o
	$(CC) $(CFLAGS) $(INCLUDE) temp_paulsen_r.o nrutil.o  temp_r_read_dat.o -o temp_paulsen_r.exe

# DO NOT DELETE THIS LINE -- make depend depends on it.

gas_update_gas.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
gas_update_gas.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
gas_update_gas.o: /usr/include/errno.h /usr/include/sys/errno.h
gas_update_gas.o: /usr/include/string.h nr.h nrutil.h
nrutil.o: /usr/include/malloc.h /usr/include/stdio.h
nrutil.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
temp_date.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
temp_date.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
temp_date.o: /usr/include/string.h nr.h nrutil.h
temp_r.o: /usr/include/math.h /usr/include/stdio.h
temp_r.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
temp_r.o: nrutil.h /usr/include/pwd.h /usr/include/sys/types.h
temp_r.o: /usr/include/sys/isa_defs.h /usr/include/sys/machtypes.h
temp_r.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
temp_r.o: /usr/include/sys/time.h /usr/include/time.h /usr/include/errno.h
temp_r.o: /usr/include/sys/errno.h /usr/include/stdlib.h
temp_r_read_dat.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
temp_r_read_dat.o: /usr/include/sys/va_list.h /usr/include/string.h
temp_update_temps.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
temp_update_temps.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
temp_update_temps.o: /usr/include/errno.h /usr/include/sys/errno.h
