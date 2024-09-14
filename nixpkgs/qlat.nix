{ fetchPypi
, stdenv
, lib
, config
, buildPythonPackage
, mpi4py
, sympy
, scipy
, jax
, jaxlib
, qlat_utils
, mpi
, git
, fftw
, fftwFloat
, gsl
, cuba
, is-pypi-src ? true
, qlat-name ? ""
, cudaSupport ? config.cudaSupport
, cudaPackages ? {}
}:

let
  orig-stdenv = stdenv;
in

buildPythonPackage rec {

  pname = "qlat${qlat-name}";
  version = if is-pypi-src then version-pypi else version-local;

  pyproject = true;

  src = if is-pypi-src then src-pypi else src-local;

  version-pypi = "0.70";
  src-pypi = fetchPypi {
    pname = "qlat";
    version = version-pypi;
    extension = "tar.gz";
    hash = "sha256-Jm+FcqUt4A5jdlFGHvKBdqNsUa3zU1fNRnWhfWdzDUs=";
  };

  version-local = builtins.readFile ../VERSION + "current";
  src-local = ../qlat;

  enableParallelBuilding = true;

  stdenv = if cudaSupport then cudaPackages.backendStdenv else orig-stdenv;

  build-system = [
    qlat_utils
  ];

  nativeBuildInputs = [
    git
  ]
  ++ lib.optionals cudaSupport (with cudaPackages; [ cuda_nvcc ])
  ;

  propagatedBuildInputs = [
    mpi
    fftw
    fftwFloat
    gsl
    cuba
  ];

  dependencies = [
    qlat_utils
    mpi4py
    sympy
    scipy
    jax
    jaxlib
  ];

  postPatch = ''
    sed -i "s/'-j4'/'-j$NIX_BUILD_CORES'/" pyproject.toml
  '';

  preConfigure = ''
    export OMPI_CXX=c++
    export OMPI_CC=cc
    #
    export
  '';

}