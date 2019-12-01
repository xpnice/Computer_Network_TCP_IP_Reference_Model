CC=gcc
#库文件与库查找路径
DLIBS=-lcommon
LDFLAGS=-L.
#指定运行时的库文件路径
RPATH=-Wl,-rpath=.
#头文件查找路径
HEAD=-I../

SRCS=\
		sender3_network.c sender3_datalink.c sender3_physical.c \
		receiver3_network.c  receiver3_physical.c receiver3_datalink.c \
		test.c

TARGET=$(patsubst %.c, %, $(SRCS))
OBJS=$(SRCS:.c=.o) 


all:libcommon.so $(TARGET) 


# %:%.o
# 	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

# %.o:%.c
# 	$(CC) $(HEAD) -o $@ -c $<

test.o:test.c 
	$(CC) $(HEAD) -o $@ -c $<
test:test.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 


sender3_network.o:sender3_network.c 
	$(CC) $(HEAD) -o $@ -c $<
sender3_network:sender3_network.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

sender3_datalink.o:sender3_datalink.c 
	$(CC) $(HEAD) -o $@ -c $<
sender3_datalink:sender3_datalink.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

sender3_physical.o:sender3_physical.c 
	$(CC) $(HEAD) -o $@ -c $<
sender3_physical:sender3_physical.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver3_network.o:receiver3_network.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver3_network:receiver3_network.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver3_datalink.o:receiver3_datalink.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver3_datalink:receiver3_datalink.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver3_physical.o:receiver3_physical.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver3_physical:receiver3_physical.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

#共享库----------------------------------------------
common.o:../common.c
	$(CC) -o $@  -fPIC -c $<

#编译生成共享库
libcommon.so:common.o 
	$(CC) -shared -o $@ $^
#-----------------------------------------------------

.PHONY:clean all
clean:
	rm -rf $(OBJS) $(TARGET)  s_file.txt d_file.txt RDL_RPL.share SDL_SPL.share
	rm -rf libcommon.so common.o



