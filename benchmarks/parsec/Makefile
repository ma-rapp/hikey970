all: binaries/.stamp

parsec-3.0-core.tar.gz:
	wget http://parsec.cs.princeton.edu/download/3.0/parsec-3.0-core.tar.gz

parsec-3.0-input-sim.tar.gz:
	wget http://parsec.cs.princeton.edu/download/3.0/parsec-3.0-input-sim.tar.gz

parsec-3.0-input-native.tar.gz:
	wget http://parsec.cs.princeton.edu/download/3.0/parsec-3.0-input-native.tar.gz

parsec-3.0/.stamp: parsec-3.0-core.tar.gz parsec-3.0-input-sim.tar.gz parsec-3.0-input-native.tar.gz
	tar -xzf parsec-3.0-core.tar.gz
	tar -xzf parsec-3.0-input-sim.tar.gz
	tar -xzf parsec-3.0-input-native.tar.gz
	touch $@

parsec-3.0/.stamp_patches: parsec-3.0/.stamp
	cd parsec-3.0 && patch -p1 < ../patches/hikey_global.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_gsl.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_libjpeg.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_ssl.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_zlib.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_blackscholes.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_bodytrack.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_canneal.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_dedup.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_facesim.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_ferret.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_fluidanimate.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_freqmine.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_streamcluster.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_swaptions.diff
	cd parsec-3.0 && patch -p1 < ../patches/hikey_x264.diff
	touch $@

parsec-3.0/.stamp_build: parsec-3.0/.stamp_patches
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p blackscholes -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p bodytrack -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p canneal -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p dedup -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p facesim -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p ferret -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p fluidanimate -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p freqmine -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p streamcluster -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p swaptions -c hikey"
	bash -c "cd parsec-3.0 && source env.sh && parsecmgmt -a build -p x264 -c hikey"
	touch $@

binaries/.stamp: parsec-3.0/.stamp_build
	mkdir -p binaries/blackscholes
	cp parsec-3.0/pkgs/apps/blackscholes/inst/amd64-linux.hikey/bin/blackscholes binaries/blackscholes
	tar -xf parsec-3.0/pkgs/apps/blackscholes/inputs/input_simsmall.tar -C binaries/blackscholes
	tar -xf parsec-3.0/pkgs/apps/blackscholes/inputs/input_native.tar -C binaries/blackscholes
	echo "./blackscholes 1 in_4K.txt prices.txt" > binaries/blackscholes/run_simsmall.sh
	echo "./blackscholes 1 in_10M.txt prices.txt" > binaries/blackscholes/run_native.sh
	mkdir -p binaries/bodytrack
	cp parsec-3.0/pkgs/apps/bodytrack/inst/amd64-linux.hikey/bin/bodytrack binaries/bodytrack
	tar -xf parsec-3.0/pkgs/apps/bodytrack/inputs/input_simsmall.tar -C binaries/bodytrack
	tar -xf parsec-3.0/pkgs/apps/bodytrack/inputs/input_native.tar -C binaries/bodytrack
	echo "./bodytrack sequenceB_1 4 1 1000 5 0 1" > binaries/bodytrack/run_simsmall.sh
	echo "./bodytrack sequenceB_261 4 261 4000 5 0 1" > binaries/bodytrack/run_native.sh
	mkdir -p binaries/canneal
	cp parsec-3.0/pkgs/kernels/canneal/inst/amd64-linux.hikey/bin/canneal binaries/canneal
	tar -xf parsec-3.0/pkgs/kernels/canneal/inputs/input_simsmall.tar -C binaries/canneal
	tar -xf parsec-3.0/pkgs/kernels/canneal/inputs/input_native.tar -C binaries/canneal
	echo "./canneal 1 10000 2000 100000.nets 32" > binaries/canneal/run_simsmall.sh
	echo "./canneal 1 15000 2000 2500000.nets 6000" > binaries/canneal/run_native.sh
	mkdir -p binaries/dedup
	cp parsec-3.0/pkgs/kernels/dedup/inst/amd64-linux.hikey/bin/dedup binaries/dedup
	tar -xf parsec-3.0/pkgs/kernels/dedup/inputs/input_simsmall.tar -C binaries/dedup
	tar -xf parsec-3.0/pkgs/kernels/dedup/inputs/input_native.tar -C binaries/dedup
	echo "./dedup -c -p -v -t 1 -i media.dat -o output.dat.ddp" > binaries/dedup/run_simsmall.sh
	echo "./dedup -c -p -v -t 1 -i FC-6-x86_64-disc1.iso -o output.dat.ddp" > binaries/dedup/run_native.sh
	mkdir -p binaries/facesim
	cp parsec-3.0/pkgs/apps/facesim/inst/amd64-linux.hikey/bin/facesim binaries/facesim
	tar -xf parsec-3.0/pkgs/apps/facesim/inputs/input_simsmall.tar -C binaries/facesim
	tar -xf parsec-3.0/pkgs/apps/facesim/inputs/input_native.tar -C binaries/facesim
	echo "./facesim -timing -threads 1" > binaries/facesim/run_simsmall.sh
	echo "./facesim -timing -threads 1 -lastframe 100" > binaries/facesim/run_native.sh
	mkdir -p binaries/ferret
	cp parsec-3.0/pkgs/apps/ferret/inst/amd64-linux.hikey/bin/ferret binaries/ferret
	tar -xf parsec-3.0/pkgs/apps/ferret/inputs/input_simsmall.tar -C binaries/ferret
	tar -xf parsec-3.0/pkgs/apps/ferret/inputs/input_native.tar -C binaries/ferret
	echo "./ferret corel lsh queries 10 20 1 output.txt" > binaries/ferret/run_simsmall.sh
	echo "./ferret corel lsh queries 50 20 1 output.txt" > binaries/ferret/run_native.sh
	mkdir -p binaries/fluidanimate
	cp parsec-3.0/pkgs/apps/fluidanimate/inst/amd64-linux.hikey/bin/fluidanimate binaries/fluidanimate
	tar -xf parsec-3.0/pkgs/apps/fluidanimate/inputs/input_simsmall.tar -C binaries/fluidanimate
	tar -xf parsec-3.0/pkgs/apps/fluidanimate/inputs/input_native.tar -C binaries/fluidanimate
	echo "./fluidanimate 1 5 in_35K.fluid out.fluid" > binaries/fluidanimate/run_simsmall.sh
	echo "./fluidanimate 1 500 in_500K.fluid out.fluid" > binaries/fluidanimate/run_native.sh
	mkdir -p binaries/freqmine
	cp parsec-3.0/pkgs/apps/freqmine/inst/amd64-linux.hikey/bin/freqmine binaries/freqmine
	tar -xf parsec-3.0/pkgs/apps/freqmine/inputs/input_simsmall.tar -C binaries/freqmine
	tar -xf parsec-3.0/pkgs/apps/freqmine/inputs/input_native.tar -C binaries/freqmine
	echo "./freqmine kosarak_250k.dat 220" > binaries/freqmine/run_simsmall.sh
	echo "./freqmine webdocs_250k.dat 11000" > binaries/freqmine/run_native.sh
	#streamcluster fails when executing
	#mkdir -p binaries/streamcluster
	#cp parsec-3.0/pkgs/kernels/streamcluster/inst/amd64-linux.hikey/bin/streamcluster binaries/streamcluster
	#echo "./streamcluster 10 20 32 4096 4096 1000 none output.txt 1" > binaries/streamcluster/run_simsmall.sh
	#echo "./streamcluster 10 20 128 1000000 200000 5000 none output.txt 1" > binaries/streamcluster/run_native.sh
	mkdir -p binaries/swaptions
	cp parsec-3.0/pkgs/apps/swaptions/inst/amd64-linux.hikey/bin/swaptions binaries/swaptions
	echo "./swaptions -ns 16 -sm 10000 -nt 1" > binaries/swaptions/run_simsmall.sh
	echo "./swaptions -ns 128 -sm 1000000 -nt 1" > binaries/swaptions/run_native.sh
	mkdir -p binaries/x264
	cp parsec-3.0/pkgs/apps/x264/inst/amd64-linux.hikey/bin/x264 binaries/x264
	tar -xf parsec-3.0/pkgs/apps/x264/inputs/input_simsmall.tar -C binaries/x264
	tar -xf parsec-3.0/pkgs/apps/x264/inputs/input_native.tar -C binaries/x264
	echo "./x264 --quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads 1 -o eledream.264 eledream_640x360_8.y4m" > binaries/x264/run_simsmall.sh
	echo "./x264  --quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads 1 -o eledream.264 eledream_1920x1080_512.y4m" > binaries/x264/run_native.sh
	touch $@

clean:
	rm -rf parsec-3.0
	rm -rf parsec-3.0-core.tar.gz parsec-3.0-input-sim.tar.gz parsec-3.0-input-native.tar.gz
	rm -rf binaries

.PHONY: all clean
