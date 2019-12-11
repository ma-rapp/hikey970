all: gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu/.stamp parsec
	make -C parsec all

gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz:
	wget https://releases.linaro.org/components/toolchain/binaries/7.1-2017.08/aarch64-linux-gnu/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz

gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu/.stamp: gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz
	tar -xf $<
	touch $@

clean:
	rm -rf gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz
	make -C clean all

.PHONY: all clean
