#!/bin/bash

. scripts/conf.sh

mkdir -p "$distfiles"

cd "$distfiles"

dget() {
    name="$1"
    url="$2"
    if [ -f "$name" ] ; then
        echo "$name is downloaded"
    else
        wget --no-check-certificate -O "$name" -c "$url"
    fi
}

aget() {
    url="$1"
    name="${1##*/}"
    dget "$name" "$url"
}

dget "c-lime.tar.gz" "https://github.com/usqcd-software/c-lime/tarball/master"

dget "ninja-1.11.1.tar.gz" "https://github.com/ninja-build/ninja/archive/refs/tags/v1.11.1.tar.gz"

dget "lapack-3.10.1.tar.gz" "https://github.com/Reference-LAPACK/lapack/archive/refs/tags/v3.10.1.tar.gz"

aget "https://tukaani.org/xz/xz-5.2.7.tar.gz"

aget "https://ftp.gnu.org/gnu/tar/tar-1.34.tar.gz"

aget "http://www.fftw.org/fftw-3.3.10.tar.gz"

aget "https://feynarts.de/cuba/Cuba-4.2.2.tar.gz"

aget "https://gnu.askapache.com/gsl/gsl-2.7.1.tar.gz"

aget "https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.bz2"

aget "https://gmplib.org/download/gmp/gmp-6.2.1.tar.bz2"

aget "https://ftp.gnu.org/gnu/mpfr/mpfr-4.1.1.tar.xz"

aget "https://ftp.gnu.org/gnu/mpc/mpc-1.2.1.tar.gz"

aget "http://ftp.gnu.org/gnu/autoconf/autoconf-2.71.tar.gz"

aget "http://ftp.gnu.org/gnu/automake/automake-1.16.5.tar.gz"

aget "https://github.com/Kitware/CMake/releases/download/v3.24.3/cmake-3.24.3.tar.gz"

aget "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.7/src/hdf5-1.10.7.tar.bz2"

aget "http://mirrors.concertpass.com/gcc/releases/gcc-12.2.0/gcc-12.2.0.tar.xz"

aget "https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.4.tar.bz2"

aget "https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.2/llvm-project-15.0.2.src.tar.xz"

aget "https://www.cpan.org/src/5.0/perl-5.34.0.tar.gz"

aget "https://www.openssl.org/source/openssl-3.0.7.tar.gz"

aget "https://www.python.org/ftp/python/3.11.0/Python-3.11.0.tar.xz"

aget "https://github.com/libffi/libffi/releases/download/v3.4.4/libffi-3.4.4.tar.gz"

aget "https://github.com/skvadrik/re2c/releases/download/2.2/re2c-2.2.tar.xz"

aget "https://zlib.net/zlib-1.2.13.tar.gz"

aget "https://gigenet.dl.sourceforge.net/project/gnuplot/gnuplot/5.4.5/gnuplot-5.4.5.tar.gz"

aget "https://cytranet.dl.sourceforge.net/project/tclap/tclap-1.2.5.tar.gz"

aget "https://github.com/xianyi/OpenBLAS/releases/download/v0.3.21/OpenBLAS-0.3.21.tar.gz"

aget "https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz"

(
mkdir -p python-packages
cd python-packages
dget "ninja-1.0.tar.gz" "https://github.com/jinluchang/Ninja-dummy/archive/refs/tags/1.0.tar.gz"
aget "https://files.pythonhosted.org/packages/a3/50/c4d2727b99052780aad92c7297465af5fe6eec2dbae490aa9763273ffdc1/pip-22.3.1.tar.gz"
aget "https://files.pythonhosted.org/packages/b5/5b/e7b84f1107a9cd07df1436d474bb85b4d157f691ecc0c502124bb7f9275b/meson-0.64.1.tar.gz"
aget "https://files.pythonhosted.org/packages/67/cc/ac8b62a4457d6e323116a509269dae3e7948c70b0adcc0d4910cf2f528da/meson_python-0.11.0.tar.gz"
aget "https://files.pythonhosted.org/packages/4c/76/1e41fbb365ad20b6efab2e61b0f4751518444c953b390f9b2d36cf97eea0/Cython-0.29.32.tar.gz"
aget "https://files.pythonhosted.org/packages/42/38/775b43da55fa7473015eddc9a819571517d9a271a9f8134f68fb9be2f212/numpy-1.23.5.tar.gz"
aget "https://files.pythonhosted.org/packages/0a/2e/44795c6398e24e45fa0bb61c3e98de1cfea567b1b51efd3751e2f7ff9720/scipy-1.9.3.tar.gz"
aget "https://files.pythonhosted.org/packages/5a/36/4667b08bc45131fe655a27b1a112c1730f3244343c53a338f44d730bd6ba/sympy-1.11.1.tar.gz"
aget "https://files.pythonhosted.org/packages/99/e0/ed0e81de05cfa4ecbcbceec6603d175387d8bc7a6332cbfd155d09958ccf/pythran-0.12.0.tar.gz"
aget "https://files.pythonhosted.org/packages/ca/6a/6b6003a757a97c4a992b611b122fa7b5efeb0b498f09123829a30e79e8c1/scikit-build-0.16.3.tar.gz"
aget "https://files.pythonhosted.org/packages/52/fa/931038182be739955cf83179d9b9a6ce9832bc5f9a917a006f765cb53a1f/build-0.8.0.tar.gz"
aget "https://files.pythonhosted.org/packages/4d/19/e11fcc88288f68ae48e3aa9cf5a6fd092a88e629cb723465666c44d487a0/pep517-0.13.0.tar.gz"
aget "https://files.pythonhosted.org/packages/2f/23/aaf147a5bc31c8be286f07d862b3699d7b49e3411fb75087525b5c31ab3e/pybind11-2.10.1.tar.gz"
aget "https://files.pythonhosted.org/packages/a2/b8/6a06ff0f13a00fc3c3e7d222a995526cbca26c1ad107691b6b1badbbabf1/wheel-0.38.4.tar.gz"
aget "https://files.pythonhosted.org/packages/95/ba/7384cb4db4ed474d4582944053549e02ec25da630810e4a23454bc9fa617/mpmath-1.2.1.tar.gz"
aget "https://files.pythonhosted.org/packages/d4/cf/3965bddbb4f1a61c49aacae0e78fd1fe36b5dc36c797b31f30cf07dcbbb7/mpmath-1.2.1-py3-none-any.whl"
aget "https://files.pythonhosted.org/packages/3d/7d/d05864a69e452f003c0d77e728e155a89a2a26b09e64860ddd70ad64fb26/psutil-5.9.4.tar.gz"
aget "https://files.pythonhosted.org/packages/c0/3f/d7af728f075fb08564c5949a9c95e44352e23dee646869fa104a3b2060a3/tomli-2.0.1.tar.gz"
aget "https://files.pythonhosted.org/packages/48/a3/0bd844c54ae8141642088b7ae09dd38fec2ec7faa9b7d25bb6a23c1f266f/gast-0.5.3.tar.gz"
aget "https://files.pythonhosted.org/packages/8e/e3/4314ed332414c059aed340fe2097d5ca065cc8d165c08554c83eb96a59c4/pyproject-metadata-0.6.1.tar.gz"
aget "https://files.pythonhosted.org/packages/d0/43/f038b5009f93bcd77b1b8da9e6d424b739ab17aec9726f3a99eba23d53ca/setuptools_scm-7.0.5.tar.gz"
aget "https://files.pythonhosted.org/packages/14/e7/50cbac38f77eca8efd39516be6651fdb9f3c4c0fab8cf2cf05f612578737/beniget-0.4.1.tar.gz"
aget "https://files.pythonhosted.org/packages/6b/f7/c240d7654ddd2d2f3f328d8468d4f1f876865f6b9038b146bec0a6737c65/packaging-22.0.tar.gz"
aget "https://files.pythonhosted.org/packages/e3/a7/8f4e456ef0adac43f452efc2d0e4b242ab831297f1bac60ac815d37eb9cf/typing_extensions-4.4.0.tar.gz"
aget "https://files.pythonhosted.org/packages/71/22/207523d16464c40a0310d2d4d8926daffa00ac1f5b1576170a32db749636/pyparsing-3.0.9.tar.gz"
aget "https://files.pythonhosted.org/packages/4b/89/eaa3a3587ebf8bed93e45aa79be8c2af77d50790d15b53f6dfc85b57f398/distro-1.8.0.tar.gz"
aget "https://files.pythonhosted.org/packages/10/e5/be08751d07b30889af130cec20955c987a74380a10058e6e8856e4010afc/flit_core-3.8.0.tar.gz"
aget "https://files.pythonhosted.org/packages/e5/69/882ee5c9d017149285cab114ebeab373308ef0f874fcdac9beb90e0ac4da/ply-3.11.tar.gz"
aget "https://files.pythonhosted.org/packages/ef/75/2bc7bef4d668f9caa9c6ed3f3187989922765403198243040d08d2a52725/setuptools-59.8.0.tar.gz"
)

if [ -d Grid-paboyle ] ; then
    ( cd Grid-paboyle ; git pull )
else
    git clone https://github.com/paboyle/Grid.git Grid-paboyle
fi

if [ -e Grid-paboyle/configure ] ; then
    echo "Grid bootstrapped."
else
    ( cd Grid-paboyle ; git clean -f ; ./bootstrap.sh )
fi

if [ -d Grid-lehner ] ; then
    ( cd Grid-lehner ; git pull https://github.com/jinluchang/Grid.git )
else
    git clone https://github.com/jinluchang/Grid.git Grid-lehner
fi

if [ -e Grid-lehner/configure ] ; then
    echo "Grid-lehner bootstrapped."
else
    ( cd Grid-lehner ; git clean -f ; ./bootstrap.sh ; ls -l Eigen )
fi

if [ -d gpt ] ; then
    ( cd gpt ; git pull https://github.com/jinluchang/gpt.git )
else
    git clone https://github.com/jinluchang/gpt.git
fi

if [ -d Hadrons ] ; then
    ( cd Hadrons ; git pull )
else
    git clone https://github.com/aportelli/Hadrons.git Hadrons
fi

if [ -e Hadrons/configure ] ; then
    echo "Hadrons bootstrapped."
else
    ( cd Hadrons ; git clean -f ; ./bootstrap.sh )
fi

if [ -d Grid-tblum ] ; then
    ( cd Grid-tblum ; git pull )
else
    git clone https://github.com/tblum2/Grid.git Grid-tblum
fi

if [ -e Grid-tblum/configure ] ; then
    echo "Grid-tblum bootstrapped."
else
    ( cd Grid-tblum ; git clean -f ; ./bootstrap.sh )
fi

if [ -d Hadrons-tblum ] ; then
    ( cd Hadrons-tblum ; git pull )
else
    git clone https://github.com/tblum2/Hadrons.git Hadrons-tblum
fi

if [ -e Hadrons-tblum/configure ] ; then
    echo "Hadrons-tblum bootstrapped."
else
    ( cd Hadrons-tblum ; git clean -f ; ./bootstrap.sh )
fi

cd $wd
