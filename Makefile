OUT := ta-service

CFLAGS := -Wall -g
LDFLAGS := -lqrtr

SRCS := main.c qmi_ta227.c qmi_ta228.c qmi_svc229.c ta.c
OBJS := $(SRCS:.c=.o)

$(OUT): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.c: %.qmi
	qmic -k < $<

install: $(OUT)
	install -D -m 755 $< $(DESTDIR)$(prefix)/bin/$<

clean:
	rm -f $(OUT) $(OBJS)
