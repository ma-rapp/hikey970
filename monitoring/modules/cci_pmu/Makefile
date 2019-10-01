obj-m += cci_pmu.o

all:
	ARCH=arm64 CROSS_COMPILE=/path/to/aarch64-linux-android-4.9/bin/aarch64-linux-android- make -C /path/to/kernel/linux M=$(CURDIR) modules

clean:
	ARCH=arm64 CROSS_COMPILE=/path/to/aarch64-linux-android-4.9/bin/aarch64-linux-android- make -C /path/to/kernel/linux M=$(CURDIR) clean
