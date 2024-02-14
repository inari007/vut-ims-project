CC = -std=c++17 g++ -O2 -g
CFLAGS = -lsimlib -lm

all:
	$(CC) ims.cpp $(CFLAGS) -o ims

clean:
	$(RM) ims

run:
	@$(MAKE) && ./ims $(ARGS)



