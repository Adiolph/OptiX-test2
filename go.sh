#!/bin/bash -l

sdir=$(pwd)
name=$(basename $sdir)
bdir=$(pwd)/build 
idir=$(pwd)/install 

cat << EOI

   name              : ${name}
   build directory   : ${bdir}
   install directory : ${idir}

EOI



rm -rf $bdir && mkdir -p $bdir 
[ ! -d $bdir ] && exit 1

cd $bdir && pwd 

cuda_prefix=${CUDA_PREFIX:-/usr/local/cuda}
optix_prefix=${OPTIX_PREFIX:-/usr/local/optix}
install_prefix=$idir

cat << EOI

   cuda_prefix    : ${cuda_prefix}
   optix_prefix   : ${optix_prefix}
   install_prefix : ${install_prefix}

EOI



if [ ! -f CMakeCache.txt ]; then 
 
    cmake $sdir \
          -DCMAKE_BUILD_TYPE=Debug \
          -DOptiX_INSTALL_DIR=${optix_prefix} \
          -DCMAKE_INSTALL_PREFIX=${install_prefix}
            
fi 

# cd $bdir && make VERBOSE=1
# ./OptiX-test
