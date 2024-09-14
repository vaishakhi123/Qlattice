{ stdenv
, config
, lib
, fetchurl
, fetchFromGitHub
, mpi
, c-lime
, zlib
, openssl
, gsl
, hdf5-cpp
, gmp
, mpfr
, fftw
, fftwFloat
, git
, autoconf
, automake
, which
, openmp ? null
, cudaSupport ? config.cudaSupport
, cudaPackages ? {}
, NVCC_ARCH ? "sm_86"
}:

let

  grid-stdenv = if cudaSupport then cudaPackages.backendStdenv else stdenv;

in grid-stdenv.mkDerivation rec {

  pname = "Grid-lehner";
  version = "9f89486df5e65c873308df23240a3b826c257d76";

  src = fetchFromGitHub {
    owner = "lehner";
    repo = "Grid";
    rev = version;
    hash = "sha256-2qdp/HcLEB/lbQ5ipm/3FGW5gofkLfcJj6d4HYtP0fs=";
  };

  enableParallelBuilding = true;

  nativeBuildInputs = [
    mpi
    git
    autoconf
    automake
    which
  ]
  ++ lib.optionals cudaSupport (with cudaPackages; [ cuda_nvcc ])
  ;

  propagatedBuildInputs = [
    mpi
    c-lime
    zlib
    fftw
    fftwFloat
    gsl
    openssl
    hdf5-cpp
    gmp
    mpfr
  ]
  ++ lib.optional grid-stdenv.cc.isClang openmp
  ++ lib.optionals cudaSupport (with cudaPackages; [
    cuda_cccl
    cuda_cudart
	cuda_profiler_api
  ])
  ;

  preConfigure = let
    eigen-file-name = "eigen-3.3.7.tar.bz2";
    eigen-src = fetchurl {
      url = "https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.bz2";
      hash = "sha256-aFrfFL2OnAFbeAl8HcIvLwE0N1bxlqzcdqZ44a41LhE=";
    };
    cpu_cxx = "c++";
    gpu_cxx = "nvcc";
    cpu_cflags = "-fPIC -w -Wno-psabi";
    gpu_cflags = "-Xcompiler -fPIC -ccbin mpic++ -arch=${NVCC_ARCH} -w";
    cpu_ldflags = "";
    gpu_ldflags = "-Xcompiler -fopenmp -ccbin mpic++";
    cxx = if cudaSupport then gpu_cxx else cpu_cxx;
    cflags = if cudaSupport then gpu_cflags else cpu_cflags;
    ldflags = if cudaSupport then gpu_ldflags else cpu_ldflags;
	cpu_extra = ''
	'';
	gpu_extra = ''
      which nvcc
      nvcc --version
	'';
	extra = if cudaSupport then gpu_extra else cpu_extra;
  in ''
    echo "-- deploying Eigen source..."
    cp -pv '${eigen-src}' '${eigen-file-name}'
    bash ./scripts/update_eigen.sh '${eigen-file-name}'
    rm '${eigen-file-name}'
    patch Eigen/unsupported/Eigen/CXX11/Tensor scripts/eigen-3.3.5.Tensor.patch
    #
    echo '-- generating Make.inc files...'
    bash ./scripts/filelist
    echo '-- generating configure script...'
    autoreconf -fvi
    #
    echo '-- set FLAGS ...'
    export CXX=${cxx}
    export CFLAGS="${cflags}"
    export CXXFLAGS="${cflags}"
    export LDFLAGS="${ldflags}"
    echo CXX="$CXX"
    echo CFLAGS="$CFLAGS"
    echo CXXFLAGS="$CXXFLAGS"
    echo LDFLAGS="$LDFLAGS"
    #
	export OMPI_CXX=c++
	export OMPI_CC=cc
    #
    which mpic++
    mpic++ --version
	#
    which c++
    c++ --version
	#
	echo
	echo 'grid-stdenv=${grid-stdenv.cc}'
	echo
  '' + extra;

  configureFlags = let
    cpu_flags = [
      "--enable-simd=AVX2"
      "--enable-alloc-align=4k"
      "--enable-comms=mpi-auto"
      "--enable-gparity=no"
      # "--with-gmp"
      # "--with-mpfr"
      # "--with-lime"
      # "--with-openssl"
      # "--with-fftw"
      # "--with-hdf5"
    ];
    gpu_flags = [
      "--enable-simd=GPU"
      "--enable-gen-simd-width=32"
      "--enable-alloc-align=4k"
      "--enable-comms=mpi-auto"
      "--enable-gparity=no"
      "--disable-fermion-reps"
      "--enable-unified=no"
      "--enable-accelerator=cuda"
      "--enable-accelerator-cshift"
      # "--with-gmp"
      # "--with-mpfr"
      # "--with-lime"
      # "--with-openssl"
      # "--with-fftw"
      # "--with-hdf5"
      ];
      flags = if cudaSupport then gpu_flags else cpu_flags;
      in flags;

    }