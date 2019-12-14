cd ~/testtools/river && cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Debug && make && sudo make install && cd ~/testtools/river/development && river.tracer -p libfmi.so < input.txt
