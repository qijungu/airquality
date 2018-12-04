all:
	cd opcreader; make
	cd no2reader; make

clean:
	cd opcreader; make clean
	cd no2reader; make clean
	cd vehstatus; rm -rf *.pyc
	cd dronedata; rm -rf *.pyc
	cd dronesite; rm -rf *.pyc

