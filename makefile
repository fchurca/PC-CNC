cnc: main.cpp cnc.cpp ParallelPort.cpp
	c++ $? -o $@

cnc.db: main.cpp cnc.cpp ParallelPort.cpp
	c++ -g $? -o $@
clean:
	rm -f cnc cnc.db *.o *~

test: cnc
	./cnc

debug: cnc.db
	./cnc.db

.PHONY: clean debug
