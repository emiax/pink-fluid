#Pink Fluid
In order to clone, compile and run this:

    git clone --recursive https://github.com/emiax/pink-fluid.git
    cd pink-fluid/build
    cmake ..
    make -j4
    ./pink-fluid


If you want to add tests, add these under test directory, these should have the suffix .cpp. The tests are run with Gtest, and for information on how to write these tests, check the documentation of Gtest.

If you want to run these tests, use the following commands

    cd build
    cmake ..
    make -j4
    ./pink-fluid_test
    
This project __requires__ cmake, git and svn

To change the projects name (and the corresponding execuatables), change the ````set(PROJECT_NAME_STR Opengl-Bootstrap)```` line.


##Based on LiTHehack/opengl-bootstrap

Macs and Linux is currently supported (Mac users need to change the OpengGL version in the Opengl-Bootstrap file). 
Windows support is currently very iffy. Pull Requests are welcome.
