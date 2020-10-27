# An test project to investigate how to build an OptiX project

I get an Parse error while calling `rtProgramCreateFromPTXString`. The whole output of `go.sh` in my Linux computer is:
```
lab110@lab110-MS-7B89:~/optix-test$ ./go.sh 

   name              : optix-test
   build directory   : /home/lab110/optix-test/build
   install directory : /home/lab110/optix-test/install

/home/lab110/optix-test/build

   cuda_prefix    : /usr/local/cuda
   optix_prefix   : /usr/local/optix
   install_prefix : /home/lab110/optix-test/install

-- The C compiler identification is GNU 7.5.0
-- The CXX compiler identification is GNU 7.5.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
CMAKE_MODULE_PATH: /home/lab110/optix-test/CMake/usr/local/optix/SDK/CMake
-- OptiX_INCLUDE_DIR :  
-- Looking for pthread.h
-- Looking for pthread.h - found
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed
-- Looking for pthread_create in pthreads
-- Looking for pthread_create in pthreads - not found
-- Looking for pthread_create in pthread
-- Looking for pthread_create in pthread - found
-- Found Threads: TRUE  
-- Found CUDA: /usr/local/cuda (found suitable version "10.1", minimum required is "10.0") 
--  COMPUTE_CAPABILITY=30 
/usr/local/cuda/bin/nvcc --machine=64;--ptx;-gencode=arch=compute_30,code=sm_30;--use_fast_math;--relocatable-device-code=true;--generate-line-info;-Wno-deprecated-gpu-targets;-I/usr/local/optix/include;-I/home/lab110/optix-test/cuda;-I/usr/local/cuda/include cuda/point_source.cu -o /home/lab110/optix-test/build/ptx/point_source.ptx
/usr/local/cuda/bin/nvcc --machine=64;--ptx;-gencode=arch=compute_30,code=sm_30;--use_fast_math;--relocatable-device-code=true;--generate-line-info;-Wno-deprecated-gpu-targets;-I/usr/local/optix/include;-I/home/lab110/optix-test/cuda;-I/usr/local/cuda/include cuda/simple_dom.cu -o /home/lab110/optix-test/build/ptx/simple_dom.ptx
/usr/local/cuda/bin/nvcc --machine=64;--ptx;-gencode=arch=compute_30,code=sm_30;--use_fast_math;--relocatable-device-code=true;--generate-line-info;-Wno-deprecated-gpu-targets;-I/usr/local/optix/include;-I/home/lab110/optix-test/cuda;-I/usr/local/cuda/include cuda/sphere.cu -o /home/lab110/optix-test/build/ptx/sphere.ptx
PTX_SOURCES: /home/lab110/optix-test/build/ptx/point_source.ptx/home/lab110/optix-test/build/ptx/simple_dom.ptx/home/lab110/optix-test/build/ptx/sphere.ptx
-- Configuring done
-- Generating done
-- Build files have been written to: /home/lab110/optix-test/build
/usr/bin/cmake -S/home/lab110/optix-test -B/home/lab110/optix-test/build --check-build-system CMakeFiles/Makefile.cmake 0
/usr/bin/cmake -E cmake_progress_start /home/lab110/optix-test/build/CMakeFiles /home/lab110/optix-test/build/CMakeFiles/progress.marks
make -f CMakeFiles/Makefile2 all
make[1]: Entering directory '/home/lab110/optix-test/build'
make -f CMakeFiles/OptiX-test.dir/build.make CMakeFiles/OptiX-test.dir/depend
make[2]: Entering directory '/home/lab110/optix-test/build'
[ 14%] Generating ptx/sphere.ptx
cd /home/lab110/optix-test && /usr/local/cuda/bin/nvcc --machine=64 --ptx -gencode=arch=compute_30,code=sm_30 --use_fast_math --relocatable-device-code=true --generate-line-info -Wno-deprecated-gpu-targets -I/usr/local/optix/include -I/home/lab110/optix-test/cuda -I/usr/local/cuda/include cuda/sphere.cu -o /home/lab110/optix-test/build/ptx/sphere.ptx
[ 28%] Generating ptx/point_source.ptx
cd /home/lab110/optix-test && /usr/local/cuda/bin/nvcc --machine=64 --ptx -gencode=arch=compute_30,code=sm_30 --use_fast_math --relocatable-device-code=true --generate-line-info -Wno-deprecated-gpu-targets -I/usr/local/optix/include -I/home/lab110/optix-test/cuda -I/usr/local/cuda/include cuda/point_source.cu -o /home/lab110/optix-test/build/ptx/point_source.ptx
[ 42%] Generating ptx/simple_dom.ptx
cd /home/lab110/optix-test && /usr/local/cuda/bin/nvcc --machine=64 --ptx -gencode=arch=compute_30,code=sm_30 --use_fast_math --relocatable-device-code=true --generate-line-info -Wno-deprecated-gpu-targets -I/usr/local/optix/include -I/home/lab110/optix-test/cuda -I/usr/local/cuda/include cuda/simple_dom.cu -o /home/lab110/optix-test/build/ptx/simple_dom.ptx
cd /home/lab110/optix-test/build && /usr/bin/cmake -E cmake_depends "Unix Makefiles" /home/lab110/optix-test /home/lab110/optix-test /home/lab110/optix-test/build /home/lab110/optix-test/build /home/lab110/optix-test/build/CMakeFiles/OptiX-test.dir/DependInfo.cmake --color=
Dependee "/home/lab110/optix-test/build/CMakeFiles/OptiX-test.dir/DependInfo.cmake" is newer than depender "/home/lab110/optix-test/build/CMakeFiles/OptiX-test.dir/depend.internal".
Dependee "/home/lab110/optix-test/build/CMakeFiles/CMakeDirectoryInformation.cmake" is newer than depender "/home/lab110/optix-test/build/CMakeFiles/OptiX-test.dir/depend.internal".
Scanning dependencies of target OptiX-test
make[2]: Leaving directory '/home/lab110/optix-test/build'
make -f CMakeFiles/OptiX-test.dir/build.make CMakeFiles/OptiX-test.dir/build
make[2]: Entering directory '/home/lab110/optix-test/build'
[ 57%] Building CXX object CMakeFiles/OptiX-test.dir/src/error_check.cc.o
/usr/bin/c++   -I/home/lab110/optix-test/include -I/usr/local/optix/include -I/home/lab110/optix-test/build -isystem /usr/local/cuda/include  -g   -std=gnu++14 -o CMakeFiles/OptiX-test.dir/src/error_check.cc.o -c /home/lab110/optix-test/src/error_check.cc
[ 71%] Building CXX object CMakeFiles/OptiX-test.dir/src/main.cc.o
/usr/bin/c++   -I/home/lab110/optix-test/include -I/usr/local/optix/include -I/home/lab110/optix-test/build -isystem /usr/local/cuda/include  -g   -std=gnu++14 -o CMakeFiles/OptiX-test.dir/src/main.cc.o -c /home/lab110/optix-test/src/main.cc
[ 85%] Building CXX object CMakeFiles/OptiX-test.dir/src/read_PTX.cc.o
/usr/bin/c++   -I/home/lab110/optix-test/include -I/usr/local/optix/include -I/home/lab110/optix-test/build -isystem /usr/local/cuda/include  -g   -std=gnu++14 -o CMakeFiles/OptiX-test.dir/src/read_PTX.cc.o -c /home/lab110/optix-test/src/read_PTX.cc
[100%] Linking CXX executable OptiX-test
/usr/bin/cmake -E cmake_link_script CMakeFiles/OptiX-test.dir/link.txt --verbose=1
/usr/bin/c++  -g   CMakeFiles/OptiX-test.dir/src/error_check.cc.o CMakeFiles/OptiX-test.dir/src/main.cc.o CMakeFiles/OptiX-test.dir/src/read_PTX.cc.o  -o OptiX-test  -Wl,-rpath,/usr/local/optix/lib64:/usr/local/cuda/lib64 /usr/local/optix/lib64/liboptix.so /usr/local/optix/lib64/liboptixu.so /usr/local/optix/lib64/liboptix_prime.so /usr/local/cuda/lib64/libcudart_static.a /usr/local/cuda/lib64/libcurand.so 
make[2]: Leaving directory '/home/lab110/optix-test/build'
[100%] Built target OptiX-test
make[1]: Leaving directory '/home/lab110/optix-test/build'
/usr/bin/cmake -E cmake_progress_start /home/lab110/optix-test/build/CMakeFiles 0
Create ptx source: /home/lab110/optix-test/build/ptx/point_source.ptx
Create ptx source :/home/lab110/optix-test/build/ptx/point_source.ptx
OptiX Error: 'Parse error
(/home/lab110/optix-test/src/main.cc:117)'
```