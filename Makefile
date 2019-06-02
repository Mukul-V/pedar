all: pedar

pedar:
	./build.sh

clean:
	./clean.sh

# Builder will call this to install the application before running.
install:
	echo "Installing ..."
	sudo debuild -b -uc -us
	sudo gdebi -n ../pedar_1.0.0-1_amd64.deb

# Builder uses this target to run your application.
run: pedar
	./pedar test1.q
