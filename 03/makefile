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


sender1_network.o:sender1_network.c 
	$(CC) $(HEAD) -o $@ -c $<
sender1_network:sender1_network.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

sender1_datalink.o:sender1_datalink.c 
	$(CC) $(HEAD) -o $@ -c $<
sender1_datalink:sender1_datalink.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

sender1_physical.o:sender1_physical.c 
	$(CC) $(HEAD) -o $@ -c $<
sender1_physical:sender1_physical.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver1_network.o:receiver1_network.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver1_network:receiver1_network.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver1_datalink.o:receiver1_datalink.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver1_datalink:receiver1_datalink.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver1_physical.o:receiver1_physical.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver1_physical:receiver1_physical.o
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



