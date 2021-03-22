BOOTLOADER=$(shell pwd)/bootloader
KERNEL=$(shell pwd)/kernel
TOOLCHAIN=$(shell pwd)/toolchain
KERNEL_OUT=$(KERNEL)/linux/arch/arm64/boot
ANDROID=$(shell pwd)/hikey970_android_8_images
ANDROID_IMG=$(shell pwd)/hikey970_android_8_images/image
SERIALNO=54DA9CD5022525E4

all: out out/prm_ptable.img out/sec_xloader.img out/l-loader.bin out/fip.bin out/boot1.img out/boot2.img out/boot3.img out/boot4.img out/cache.img out/system.img out/userdata.img

$(TOOLCHAIN)/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz:
	cd $(TOOLCHAIN) && wget https://releases.linaro.org/components/toolchain/binaries/7.1-2017.08/aarch64-linux-gnu/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz

$(TOOLCHAIN)/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu/.stamp_extracted: $(TOOLCHAIN)/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz
	cd $(TOOLCHAIN) && tar -xvf gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz
	touch $@

$(BOOTLOADER)/edk2/OpenPlatformPkg:
	cd $(BOOTLOADER)/edk2 && ln -sf ../OpenPlatformPkg

$(BOOTLOADER)/.stamp_patches: $(BOOTLOADER)/l-loader.diff
	patch -p1 -d $(BOOTLOADER)/l-loader < $(BOOTLOADER)/l-loader.diff
	touch $@

bootloader: $(BOOTLOADER)/edk2/OpenPlatformPkg $(BOOTLOADER)/.stamp_patches $(TOOLCHAIN)/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu/.stamp_extracted
	rm -f $(BOOTLOADER)/.stamp_build
	cd $(BOOTLOADER) && l-loader/build_uefi.sh hikey970
	touch $(BOOTLOADER)/.stamp_build

$(ANDROID).zip:
	#wget http://hihope.org/hihope/downLoadCenter/testDownFile?filename=20190403/hikey970_android_8_images.rar -O $@
	wget mirror.lemaker.org/hikey970_android_8_images.rar -O $@

$(ANDROID)/.stamp_extracted: $(ANDROID).zip
	unzip $< -d $(ANDROID)
	touch $@

$(ANDROID_IMG)/prm_ptable.img: $(ANDROID)/.stamp_extracted
$(ANDROID_IMG)/sec_xloader.img: $(ANDROID)/.stamp_extracted

out:
	mkdir -p out

out/prm_ptable.img: $(ANDROID_IMG)/prm_ptable.img
	cp $< $@

out/sec_xloader.img: $(ANDROID_IMG)/sec_xloader.img
	cp $< $@

out/l-loader.bin: $(BOOTLOADER)/l-loader/l-loader.bin
	cp $< $@

out/fip.bin: $(BOOTLOADER)/l-loader/fip.bin
	cp $< $@

$(KERNEL)/.stamp_patch:
	cd $(KERNEL)/linux && patch -p1 < ../pmu.diff
	touch $@

build_kernel: $(KERNEL)/.stamp_patch
	ARCH=arm64 CROSS_COMPILE=$(TOOLCHAIN)/aarch64-linux-android-4.9/bin/aarch64-linux-android- make -C $(KERNEL)/linux hikey970_defconfig
	ARCH=arm64 CROSS_COMPILE=$(TOOLCHAIN)/aarch64-linux-android-4.9/bin/aarch64-linux-android- make -C $(KERNEL)/linux -j4
	ARCH=arm64 CROSS_COMPILE=$(TOOLCHAIN)/aarch64-linux-android-4.9/bin/aarch64-linux-android- make -C $(KERNEL)/linux hisilicon/kirin970-hikey970.dtb

$(KERNEL_OUT)/dts/hisilicon/kirin970-hikey970.dtb: build_kernel
	touch $@

$(KERNEL_OUT)/Image: build_kernel
	touch $@

out/kirin970-hikey970.dtb: $(KERNEL_OUT)/dts/hisilicon/kirin970-hikey970.dtb
	cp $< $@

out/Image: $(KERNEL_OUT)/Image
	cp $< $@

out/Image-dtb: out/kirin970-hikey970.dtb out/Image
	cat out/Image out/kirin970-hikey970.dtb > $@

out/ramdisk/ramdisk.orig.cpio.gz: $(ANDROID_IMG)/ramdisk.img
	mkdir -p out/ramdisk
	cp $< $@

out/ramdisk/unpacked/init.rc: out/ramdisk/ramdisk.orig.cpio.gz
	mkdir -p out/ramdisk/unpacked
	cd out/ramdisk/unpacked && gzip -dc ../ramdisk.orig.cpio.gz | cpio -i 

out/ramdisk/.stamp_updated: out/ramdisk/unpacked/init.rc
	cat init.rc.add >> out/ramdisk/unpacked/init.rc
	sed -i -e 's/persist.sys.usb.config=.*/persist.sys.usb.config=mtp,adb/g' out/ramdisk/unpacked/default.prop
	touch $@

out/ramdisk/ramdisk.new.cpio.gz: out/ramdisk/unpacked/init.rc out/ramdisk/.stamp_updated
	cd out/ramdisk/unpacked && find * | sort | cpio -o -H newc | gzip > ../ramdisk.new.cpio.gz

#out/ramdisk.img: $(ANDROID_IMG)/ramdisk.img
out/ramdisk.img: out/ramdisk/ramdisk.new.cpio.gz
	cp $< $@

out/boot1.img: out/Image-dtb out/ramdisk.img
	mkbootimg/mkbootimg.py --kernel out/Image-dtb --ramdisk out/ramdisk.img --cmdline "androidboot.hardware=hikey970 firmware_class.path=/system/etc/firmware loglevel=15 buildvariant=userdebug androidboot.selinux=permissive clk_ignore_unused=true initrd=0xBE19D000,0x16677F earlycon=pl011,0xfff32000,115200 console=ttyAMA6 androidboot.serialno=54DA9CD5022525E4 clk_ignore_unused=true" -o out/boot1.img

out/boot2.img: out/Image-dtb out/ramdisk.img
	mkbootimg/mkbootimg.py --kernel out/Image-dtb --ramdisk out/ramdisk.img --cmdline "androidboot.hardware=hikey970 firmware_class.path=/system/etc/firmware loglevel=15 buildvariant=userdebug androidboot.selinux=permissive clk_ignore_unused=true initrd=0xBE19D000,0x16677F earlycon=pl011,0xfff32000,115200 console=ttyAMA6 androidboot.serialno=54DA9CD5022525E5 clk_ignore_unused=true" -o out/boot2.img

out/boot3.img: out/Image-dtb out/ramdisk.img
	mkbootimg/mkbootimg.py --kernel out/Image-dtb --ramdisk out/ramdisk.img --cmdline "androidboot.hardware=hikey970 firmware_class.path=/system/etc/firmware loglevel=15 buildvariant=userdebug androidboot.selinux=permissive clk_ignore_unused=true initrd=0xBE19D000,0x16677F earlycon=pl011,0xfff32000,115200 console=ttyAMA6 androidboot.serialno=54DA9CD5022525E6 clk_ignore_unused=true" -o out/boot3.img

out/boot4.img: out/Image-dtb out/ramdisk.img
	mkbootimg/mkbootimg.py --kernel out/Image-dtb --ramdisk out/ramdisk.img --cmdline "androidboot.hardware=hikey970 firmware_class.path=/system/etc/firmware loglevel=15 buildvariant=userdebug androidboot.selinux=permissive clk_ignore_unused=true initrd=0xBE19D000,0x16677F earlycon=pl011,0xfff32000,115200 console=ttyAMA6 androidboot.serialno=54DA9CD5022525E7 clk_ignore_unused=true" -o out/boot4.img

out/cache.img: $(ANDROID_IMG)/cache.img
	cp $< $@

out/system.img: $(ANDROID_IMG)/system.img
	cp $< $@

out/userdata.img: $(ANDROID_IMG)/userdata.img
	cp $< $@

flashall1:
	sudo fastboot flash ptable out/prm_ptable.img
	sudo fastboot flash xloader out/sec_xloader.img
	sudo fastboot flash fastboot out/l-loader.bin
	sudo fastboot flash fip out/fip.bin
	sudo fastboot flash boot out/boot1.img
	sudo fastboot flash cache out/cache.img
	sudo fastboot flash system out/system.img
	sudo fastboot flash userdata out/userdata.img

flashall2:
	sudo fastboot flash ptable out/prm_ptable.img
	sudo fastboot flash xloader out/sec_xloader.img
	sudo fastboot flash fastboot out/l-loader.bin
	sudo fastboot flash fip out/fip.bin
	sudo fastboot flash boot out/boot2.img
	sudo fastboot flash cache out/cache.img
	sudo fastboot flash system out/system.img
	sudo fastboot flash userdata out/userdata.img

flashall3:
	sudo fastboot flash ptable out/prm_ptable.img
	sudo fastboot flash xloader out/sec_xloader.img
	sudo fastboot flash fastboot out/l-loader.bin
	sudo fastboot flash fip out/fip.bin
	sudo fastboot flash boot out/boot3.img
	sudo fastboot flash cache out/cache.img
	sudo fastboot flash system out/system.img
	sudo fastboot flash userdata out/userdata.img

flashall4:
	sudo fastboot flash ptable out/prm_ptable.img
	sudo fastboot flash xloader out/sec_xloader.img
	sudo fastboot flash fastboot out/l-loader.bin
	sudo fastboot flash fip out/fip.bin
	sudo fastboot flash boot out/boot4.img
	sudo fastboot flash cache out/cache.img
	sudo fastboot flash system out/system.img
	sudo fastboot flash userdata out/userdata.img

flashbootloader:
	sudo fastboot flash fastboot out/l-loader.bin
	sudo fastboot flash fip out/fip.bin

clean:
	rm -rf out $(BOOTLOADER)/edk2/OpenPlatformPkg
	rm -rf $(TOOLCHAIN)/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu.tar.xz $(TOOLCHAIN)/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu
	rm -rf $(ANDROID).zip $(ANDROID)
	#make -C $(KERNEL)/linux clean

.PHONY: all build_kernel bootloader clean flashall flashbootloader
