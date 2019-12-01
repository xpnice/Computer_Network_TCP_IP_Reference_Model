CC=gcc
#库文件与库查找路径
DLIBS=-lcommon
LDFLAGS=-L.
#指定运行时的库文件路径
RPATH=-Wl,-rpath=.
#头文件查找路径
HEAD=-I../

SRCS=\
		sender2_network.c sender2_datalink.c sender2_physical.c \
		receiver2_network.c  receiver2_physical.c receiver2_datalink.c \

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


sender2_network.o:sender2_network.c 
	$(CC) $(HEAD) -o $@ -c $<
sender2_network:sender2_network.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

sender2_datalink.o:sender2_datalink.c 
	$(CC) $(HEAD) -o $@ -c $<
sender2_datalink:sender2_datalink.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

sender2_physical.o:sender2_physical.c 
	$(CC) $(HEAD) -o $@ -c $<
sender2_physical:sender2_physical.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver2_network.o:receiver2_network.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver2_network:receiver2_network.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver2_datalink.o:receiver2_datalink.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver2_datalink:receiver2_datalink.o
	$(CC) -o $@ $^  $(LDFLAGS) $(DLIBS) $(RPATH) 

receiver2_physical.o:receiver2_physical.c 
	$(CC) $(HEAD) -o $@ -c $<
receiver2_physical:receiver2_physical.o
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



