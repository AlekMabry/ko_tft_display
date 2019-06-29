obj-m += tft_display.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	-sudo rmmod tft_display
	# Clear kernel log
	sudo dmesg -C
	# Insert module
	sudo insmod tft_display.ko
	# Displayu kernel log
	dmesg
