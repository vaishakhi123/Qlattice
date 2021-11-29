// general_funs.h
// Gen Wang
// Jan. 2021

#ifndef GENERAL_FUNS_H
#define GENERAL_FUNS_H
#pragma once


#include <string.h>
#include <sys/resource.h>
#include <mpi.h>
#include <time.h>
#include <typeinfo>

#include "float_type.h"
#include<type_traits>

#include <iterator>
#include <sys/sysinfo.h>
#include "utils_read_txt.h"
#include "utils_vector_GPU.h"

namespace qlat
{

template<typename Ty>
void get_MPI_type(Ty& a, MPI_Datatype& curr, unsigned int& size, int mode = 1)
{
  curr = MPI_BYTE; size = 1;
  bool is_same_v = false;

  is_same_v = std::is_same< Ty , int>::value;
  if(is_same_v){curr = MPI_INT;size=sizeof(int);return ;}

  is_same_v = std::is_same< Ty , uint32_t>::value;
  if(is_same_v){curr = MPI_UNSIGNED;size=sizeof(uint32_t);return ;}

  is_same_v = std::is_same< Ty , unsigned int>::value;
  if(is_same_v){curr = MPI_UNSIGNED;size=sizeof(unsigned int);return ;}

  is_same_v = std::is_same< Ty , long>::value;
  if(is_same_v){curr = MPI_LONG;size=sizeof(long);return ;}

  is_same_v = std::is_same< Ty , unsigned long>::value;
  if(is_same_v){curr = MPI_UNSIGNED_LONG;size=sizeof(unsigned long);return ;}

  is_same_v = std::is_same< Ty , float>::value;
  if(is_same_v){curr = MPI_FLOAT;size=sizeof(float);return ;}

  is_same_v = std::is_same< Ty , double>::value;
  if(is_same_v){curr = MPI_DOUBLE;size=sizeof(double);return ;}

  is_same_v = std::is_same< Ty , std::int8_t>::value;
  if(is_same_v){curr = MPI_INT8_T;size=sizeof(std::int8_t);return ;}

  is_same_v = std::is_same< Ty , std::int16_t>::value;
  if(is_same_v){curr = MPI_INT16_T;size=sizeof(std::int16_t);return ;}

  is_same_v = std::is_same< Ty , std::int32_t>::value;
  if(is_same_v){curr = MPI_INT32_T;size=sizeof(std::int32_t);return ;}

  is_same_v = std::is_same< Ty , std::int64_t>::value;
  if(is_same_v){curr = MPI_INT64_T;size=sizeof(std::int64_t);return ;}

  if(mode == 1){print0("Type not found 1 !!!! \n");MPI_Barrier(get_comm());fflush(stdout);qassert(false);}
  
  /////Complex types sum and equal
  if(mode == 2){
  if(sizeof(Ty) == sizeof(qlat::ComplexF)){curr = MPI_FLOAT  ;size = sizeof( float) ;return ;}
  if(sizeof(Ty) == sizeof(qlat::Complex )){curr = MPI_DOUBLE ;size = sizeof(double) ;return ;}}
   
  print0("Type not found 2 !!!! \n");MPI_Barrier(get_comm());fflush(stdout);qassert(false);

}


template<typename Ty>
void sum_all_size(Ty *src,Ty *sav,long size, int GPU=0, MPI_Comm* commp=NULL)
{
  qlat::vector_gpu<Ty > res;//// buf.resize(size, GPU);
  ///Ty *res;/////qlat::vector<Ty >buf;
  if(src == sav){
    res.resize(size, GPU);
    //if(GPU == 0){res = (Ty *)aligned_alloc_no_acc(size*sizeof(Ty));}
    //else{gpuMalloc(res, size, Ty);}
  }else{res.p = sav;}////small modify for pointers
  if(qlat::get_num_node() == 1){
    if(src == sav){return;}
    if(src != sav){
      cpy_data_thread(sav, src, size, GPU, true);
      ////#ifdef QLAT_USE_ACC
      ////if(GPU==0)memcpy(sav,src,size*sizeof(Ty));
      ////if(GPU==1){cudaMemcpy(sav, src, size*sizeof(Ty), cudaMemcpyDeviceToDevice);}
      ////#else
      ////memcpy(sav, src, size*sizeof(Ty));
      ////#endif
    return;}
  }

  MPI_Datatype curr = MPI_DOUBLE;unsigned int M_size = sizeof(double);Ty atem;//Ty atem=0;
  ////get_MPI_type(atem, curr, M_size, 1);
  get_MPI_type(atem, curr, M_size, 2);
  qassert(sizeof(Ty)%M_size == 0);int fac = sizeof(Ty)/M_size;

  Ty* tem_src = NULL; Ty* tem_res = NULL;
  std::vector<Ty > tem_sHIP,tem_rHIP;
  bool do_copy = false;

  #ifdef __NO_GPU_DIRECT__
  #ifdef QLAT_USE_ACC
  if(GPU == 1){do_copy = true;}
  #endif
  #endif

  if(do_copy == false){tem_src = src;tem_res = res.data();}
  if(do_copy == true ){
    tem_sHIP.resize(size);tem_rHIP.resize(size);

    cpy_data_thread(&tem_sHIP[0], src, size, 3, true);
    tem_src = &tem_sHIP[0];tem_res = &tem_rHIP[0];
  }
  
  if(commp == NULL){MPI_Allreduce(tem_src,tem_res, size * fac, curr, MPI_SUM, get_comm());}
  else{MPI_Allreduce(tem_src,tem_res, size * fac, curr, MPI_SUM, *commp);}

  if(do_copy == true){
    cpy_data_thread(res.data(), &tem_rHIP[0], size, 2, true);
  }


  if(src == sav)
  {
    cpy_data_thread(sav, res.data(), size, GPU, true);
    //#ifdef QLAT_USE_ACC
    //if(GPU==0){memcpy(sav,res.data(),size*sizeof(Ty));}  ////free(res);
    //if(GPU==1){cudaMemcpy(sav, res.data(), size*sizeof(Ty), cudaMemcpyDeviceToDevice);} ///gpuFree(res);res = NULL;
    //#else
    //memcpy(sav,res.data(),size*sizeof(Ty));////free(res);res = NULL;
    //#endif
  }
  if(src != sav){res.p = NULL;}
}

template<typename Ty>
void sum_all_size(Ty *src,long size, int GPU=0, MPI_Comm* commp=NULL)
{
  sum_all_size(src,src,size, GPU, commp);
}

template<typename Ty>
void Bcast_all_Nt(Ty *src,long size,const qlat::Geometry &geo)
{
  if(qlat::get_num_node() == 1){return;}
  int Nt = geo.node_site[3];
  int Nmpi  = qlat::get_num_node();

  const Coordinate vg = geo.total_site();
  const int nt = vg[3];

  if(nt/Nt != Nmpi){
    sum_all_size(src,size);
    return;
  }

  int mt = nt/Nt;
  int rank  = qlat::get_id_node();
  long size_c = sizeof(Ty)*size/mt;

  unsigned short t0 = 0;
  {
    Coordinate xl = geo.coordinate_from_index(0);
    xl[3] = 0;
    Coordinate xg = geo.coordinate_g_from_l(xl);
    t0 = xg[3];
  }

  std::vector<int > send,recv,spls,rpls;
  send.resize(Nmpi);
  recv.resize(Nmpi);
  spls.resize(Nmpi);
  rpls.resize(Nmpi);

  std::fill(send.begin(), send.end(), 0);
  std::fill(recv.begin(), recv.end(), 0);
  std::fill(spls.begin(), spls.end(), 0);
  std::fill(rpls.begin(), rpls.end(), 0);

  for(int ti=0;ti<mt;ti++){
    int tini = ti*Nt;
    if(t0 == tini){
      for(int ri=0;ri<Nmpi;ri++)if(ri != rank)
      {
        send[ri] = size_c;
        spls[ri] = size_c*ti;
      }
    }

    if(t0 != tini){
      int ri_recv = ti;
      recv[ri_recv] = size_c;
      rpls[ri_recv] = size_c*ti;
    }
  }

  MPI_Alltoallv(src,(int*) &send[0],(int*) &spls[0], MPI_CHAR,
                src,(int*) &recv[0],(int*) &rpls[0], MPI_CHAR, get_comm());

}

////Need add explanations
template<typename Ty>
void Redistribute_all_Nt(Ty *src,long size,const qlat::Geometry &geo, int GPU=0)
{
  if(qlat::get_num_node() == 1){return;}
  int Nt = geo.node_site[3];
  int Nmpi  = qlat::get_num_node();

  const Coordinate vg = geo.total_site();
  const int nt = vg[3];

  int mt = nt/Nt;
  if(mt != Nmpi){print0("Not supported !");qassert(false);return;}

  int rank  = qlat::get_id_node();
  long size_c = sizeof(Ty)*size/mt;

  std::vector<int > send,recv,spls,rpls;
  send.resize(Nmpi);
  recv.resize(Nmpi);
  spls.resize(Nmpi);
  rpls.resize(Nmpi);

  for(int ri=0;ri<Nmpi;ri++)
  {
    send[ri] = size_c;
    spls[ri] = size_c*ri;

    recv[ri] = size_c;
    rpls[ri] = size_c*ri;
  }

  //Ty* buf;
  //if(GPU == 0){buf = (Ty *)aligned_alloc_no_acc(size*sizeof(Ty));}
  //if(GPU == 1){gpuMalloc(buf, size, Ty);}
  qlat::vector_gpu<Ty > buf; buf.resize(size, GPU);

  {
  ////TIMER("MPI call CPU");
  MPI_Alltoallv(src   ,(int*) &send[0],(int*) &spls[0], MPI_CHAR,
            buf.data(),(int*) &recv[0],(int*) &rpls[0], MPI_CHAR, get_comm());
  }

  #ifdef QLAT_USE_ACC
  if(GPU == 0){
    #pragma omp parallel for
    for(long isp=0;isp<size;isp++){src[isp] = buf[isp];}
    ///delete [] buf;
    //free(buf);
  }
  if(GPU == 1){
    /////qacc_for(isp, size,{ src[isp] = buf[isp];});
    cudaMemcpy(src, buf.data(), size*sizeof(Ty), cudaMemcpyDeviceToDevice);
    ////gpuFree(buf);
  }
  #else
  #pragma omp parallel for
  for(long isp=0;isp<size;isp++){src[isp] = buf[isp];}
  //delete [] buf;
  //free(buf);
  #endif

}

inline void abort_sum(double flag, std::string stmp=std::string(""))
{
  sum_all_size(&flag,1);
  if(flag > 0)
  {
    abort_r(stmp);
  }
}

inline void fflush_MPI(){
  MPI_Barrier(get_comm());
  fflush(stdout);
}

template<typename Ty>
void MPI_Alltoallv_mode(Ty* src0, int* send, int* spls, Ty* res0, int* recv, int* rpls, MPI_Comm& comm, int mode=0, int GPU = 0)
{
  Ty* src = NULL;Ty* res = NULL;

  std::vector<Ty > tem_src,tem_res;
  bool do_copy = false;
  #ifdef __NO_GPU_DIRECT__
  #ifdef QLAT_USE_ACC
  if(GPU == 1){do_copy = true;}
  #endif
  #endif

  if(do_copy == false){src = src0; res = res0;}

  ////resize buffers
  long max_src = 0;
  long max_res = 0;
  if(do_copy == true){
    int num_node;MPI_Comm_size(comm, &num_node);
    for(int n = 0; n < num_node; n++){
      long cur_size = spls[n]/sizeof(Ty) + send[n]/sizeof(Ty);
      if(cur_size > max_src){max_src = cur_size;}
      cur_size = rpls[n]/sizeof(Ty) + recv[n]/sizeof(Ty);
      if(cur_size > max_res){max_res = cur_size;}
    }

    tem_src.resize(max_src);tem_res.resize(max_res);
    cpy_data_thread(&tem_src[0], src0, max_src, 3);
    cpy_data_thread(&tem_res[0], res0, max_res, 3);
    src = &tem_src[0]; res = &tem_res[0];
  }

  if(mode == 0){
    MPI_Alltoallv(src, send, spls, MPI_CHAR,
                  res, recv, rpls, MPI_CHAR, comm);
  }
  if(mode == 1){
    int num_node;MPI_Comm_size(comm, &num_node);
    int id_node;MPI_Comm_rank(comm, &id_node);
    std::vector<MPI_Request> send_reqs(num_node);
    int mpi_tag = id_node;
    int c1 = 0;
    for(int n = 0; n < num_node; n++){
      if(send[n]!=0){MPI_Isend(&src[spls[n]/sizeof(Ty)], send[n], MPI_CHAR, n, mpi_tag + n, comm, &send_reqs[c1]);c1 += 1;}
    }

    for(int n = 0; n < num_node; n++){
      if(recv[n]!=0){MPI_Recv( &res[rpls[n]/sizeof(Ty)], recv[n], MPI_CHAR, n, mpi_tag + n, comm, MPI_STATUS_IGNORE);}
    }
    if(c1!=0){MPI_Waitall(c1, send_reqs.data(), MPI_STATUS_IGNORE);}
  }


  if(do_copy == true){cpy_data_thread(res0, &tem_res[0], max_res, 2);}

}

////Only cpu verstion
////flag = 1 --> biva * sizeF * civ * size_inner --> biva * civ * sizeF * size_inner
inline void reorder_civ(char* src,char* res,int biva,int civ,size_t sizeF,int flag,int size_inner)
{
  //TIMER("reorder_civ vectors char");
  if(biva == 0 or civ == 0 or sizeF == 0 or size_inner == 0){return ;}

  std::vector<char* > psrc;psrc.resize(civ);
  std::vector<char > tmp;tmp.resize(biva*sizeF*civ*size_inner);
  if(size_inner <= 1){abort_r("size_innter too small ! \n");return;}

  if(flag == 1){memcpy((char*)&tmp[0],(char*)&src[0],sizeof(char)*biva*sizeF*civ*size_inner);}
 
  for(size_t bi=0;bi<size_t(biva);bi++)
  {
    for(int ci=0;ci<civ;ci++)
    {
      if(flag==0)psrc[ci] = &src[(bi*sizeF*civ + ci*sizeF + 0)*size_inner];
      if(flag==1)psrc[ci] = &res[(bi*sizeF*civ + ci*sizeF + 0)*size_inner];
    }

    #pragma omp parallel for
    for(LInt si=0;si<sizeF;si++)
    for(int ci=0;ci<civ;ci++)
    {
      if(flag==0){
        memcpy(&tmp[(bi*sizeF*civ + si*civ + ci)*size_inner],&psrc[ci][si*size_inner],sizeof(char)*size_inner);
      }
      if(flag==1){
        memcpy(&psrc[ci][si*size_inner],&tmp[(bi*sizeF*civ + si*civ + ci)*size_inner],sizeof(char)*size_inner);
      }
    }
  }
 
  if(flag == 0){memcpy((char*)&res[0],(char*)&tmp[0],biva*sizeF*civ*size_inner);}
}

///flag = 1 --> biva * sizeF * civ * size_inner --> biva * civ * sizeF * size_inner
#ifdef QLAT_USE_ACC
template <typename Ty, bool flag, int Threads, int Biva>
__global__ void move_index_global(Ty* src, Ty* res, long sizeF, int civ, int inner)
{
  __shared__ Ty buf[Threads*Biva];

  int    tid = threadIdx.x;
  long s0    = blockIdx.x*blockDim.x;

  int Total = Threads*civ*inner;
  if(s0 + Threads > sizeF){Total = (sizeF - s0) * civ*inner;}

  int nB    = (Total + Threads-1)/Threads;
  int nC    = (Total + Biva*Threads-1)/(Biva*Threads);

  int ci, si, i0;
  long z0 = 0;long off = 0;long off1 = 0;
  for(int ni=0;ni < nC; ni++)
  {
    if(z0 >= Total){break;}
    if(flag){
    off = z0 + tid;
    for(int xi=0;xi<Biva;xi++)
    {
      if(off < Total){buf[xi*Threads + tid] = src[s0*civ*inner + off];off += Threads;}
    }
    __syncthreads();
    }

    off = tid;
    for(int xi=0;xi<nB;xi++)
    {
      ci = off/(Threads*inner);
      si = (off/inner)%Threads;
      i0 = off%inner;

      off1 = (si*civ + ci)*inner + i0 - z0;
      if(off1 >= 0)
      if((off1 < Threads*Biva) and (off1 < (Total - z0)) )
      {
        if( flag){res[(ci*sizeF+s0+si)*inner + i0] = buf[off1];}
        if(!flag){buf[off1] = src[(ci*sizeF+s0+si)*inner + i0];}
      }
      off += Threads;
    }
    __syncthreads();

    if(!flag){
    off = z0 + tid;
    for(int xi=0;xi<Biva;xi++)
    {
      if(off < Total){res[s0*civ*inner + off] = buf[xi*Threads + tid];off += Threads;}
    }
    __syncthreads();
    }

    z0 += Threads*Biva;
  }

}
#endif

////TODO change into Ty*
struct move_index
{
  //bool GPU;

  vector_gpu<char > buf;
  ////size_t buf_size;
  //qlat::vector<char* > pciv;

  //move_index(bool GPU_set=false){
  //  #ifndef QLAT_USE_ACC
  //  GPU = false;
  //  #else
  //  GPU = GPU_set;
  //  #endif
  //  buf = NULL;
  //  buf_size = 0;
  //}

  //void set_mem(int civ, size_t Bsize)
  //{
  //  TIMERA("move_index set_mem");
  //  if(buf_size != Bsize){
  //    free_mem();
  //    if(GPU){gpuMalloc(buf, Bsize, char);}
  //    else{buf = (void *)aligned_alloc_no_acc(Bsize);}
  //    buf_size = Bsize;
  //  }
  //  //////psrc.resize(civ);
  //}


  ////flag = 1 --> biva * sizeF * civ * size_inner --> biva * civ * sizeF * size_inner
  template <typename Ty >
  void dojob(Ty* src,Ty* res,int biva,int civ,long sizeF,int flag, int size_inner, bool GPU = false)
  {
  if(biva == 0 or civ == 0 or sizeF == 0 or size_inner == 0){return ;}
  /////size_t sizeF = sizeF0;

  ////size_t bufN = biva*civ*size_inner*sizeof(Ty)*sizeF;
  size_t Off = civ*sizeF*size_inner;
  #if PRINT_TIMER>5
  TIMER_FLOPS("reorder index");
  timer.flops += biva*Off*sizeof(Ty);
  #endif

  ////TIMERB("reorder index");
  if(size_inner < 1){qlat::displayln_info(qlat::ssprintf("size_inner too small %d !\n", size_inner));
    MPI_Barrier(get_comm());fflush(stdout);qassert(false);
  }

  if(src == res){buf.resize(Off*sizeof(Ty), GPU);}
  //pciv.resize(civ);
  Ty* s0;Ty *s1;
  //#ifdef QLAT_USE_ACC
  //if(GPU)
  if(src == res)if((Off*sizeof(Ty)) % sizeof(qlat::ComplexF) != 0){
    qlat::displayln_info(qlat::ssprintf("size not divided by 16, too small. \n"));qassert(false);}
  ///#endif

  for(int bi=0;bi<biva;bi++){
    s0 = &src[bi*Off];
    if(src == res){s1 = (Ty*)buf.data();}else{s1 = (Ty*) &res[bi*Off];}
    #ifdef QLAT_USE_ACC
    if(GPU){

      {
      const int Threads = 32;const int Biva =  (16*16+sizeof(Ty)-1)/sizeof(Ty);
      long Nb = (sizeF + Threads -1)/Threads;
      dim3 dimBlock(    Threads,    1, 1);
      dim3 dimGrid(     Nb,    1, 1);
      if(flag==0)move_index_global<Ty, false , Threads, Biva><<< dimGrid, dimBlock >>>(s0, s1, sizeF, civ, size_inner);
      if(flag==1)move_index_global<Ty, true  , Threads, Biva><<< dimGrid, dimBlock >>>(s0, s1, sizeF, civ, size_inner);
      qacc_barrier(dummy);
      }

      if(src == res){
      long Nvol = long(Off*sizeof(Ty)/sizeof(qlat::ComplexF));
      cpy_data_thread((qlat::ComplexF*) &res[bi*Off], (qlat::ComplexF*) s1, Nvol, 1);
      }

    continue ;}
    #endif

    #pragma omp parallel for
    for(long   si=0;si<sizeF;si++)
    for(int    ci=0;ci<civ;ci++)
    {
      Ty* p0=NULL;Ty* p1=NULL;
      if(flag == 1){
        p0 = (Ty*) &s0[(si*civ   + ci)*size_inner];
        p1 = (Ty*) &s1[(ci*sizeF + si)*size_inner];
      }
      if(flag == 0){
        p0 = (Ty*) &s0[(ci*sizeF + si)*size_inner];
        p1 = (Ty*) &s1[(si*civ   + ci)*size_inner];
      }
      memcpy(p1, p0, sizeof(Ty)*size_inner);
    }

    if(src == res){
      long Nvol = long(Off*sizeof(Ty)/sizeof(qlat::ComplexF));
      cpy_data_thread((qlat::ComplexF*) &res[bi*Off], (qlat::ComplexF*) s1, Nvol, 0);
    }

  }


  }

  void free_mem(){
    buf.resize(0);
  }

  ~move_index(){
    free_mem();
  }

};


inline void set_GPU(){
  #ifdef QLAT_USE_ACC
  int num_node;MPI_Comm_size(get_comm(), &num_node);
  int id_node;MPI_Comm_rank(get_comm(), &id_node);

  int num_gpus = 0;
  cudaGetDeviceCount(&num_gpus);
  ////cudaDeviceReset();
  cudaSetDevice(id_node % num_gpus);
  int gpu_id = -1; 
  cudaGetDevice(&gpu_id);
  //printf("CPU node %d (of %d) uses CUDA device %d\n", id_node, num_node, gpu_id);
  fflush(stdout);
  MPI_Barrier(get_comm());
  #endif

}

inline void set_GPU_threads(int mode=0){
  //////Set up gpu map to cpu
  #ifdef QLAT_USE_ACC
  int num_gpus = 0;
  cudaGetDeviceCount(&num_gpus);
  cudaDeviceReset();
  if(mode == 0){
  #pragma omp parallel
  {
    unsigned int cpu_thread_id = omp_get_thread_num();
    unsigned int num_cpu_threads = omp_get_num_threads();
    cudaSetDevice(cpu_thread_id % num_gpus);
    int gpu_id = -1; 
    cudaGetDevice(&gpu_id);
    printf("CPU thread %d (of %d) uses CUDA device %d\n", cpu_thread_id, num_cpu_threads, gpu_id);
  }}

  if(mode == 1){
  #pragma omp parallel
  {
    unsigned int cpu_thread_id = omp_get_thread_num();
    unsigned int num_cpu_threads = omp_get_num_threads();
    if(cpu_thread_id%num_gpus != 0){print0("Wrong mapping of omp !\n");qassert(false);}
    int Nthreads = cpu_thread_id/num_gpus;

    cudaSetDevice(cpu_thread_id / Nthreads);
    int gpu_id = -1; 
    printf("CPU thread %d (of %d) uses CUDA device %d\n", cpu_thread_id, num_cpu_threads, gpu_id);
    cudaGetDevice(&gpu_id);
  }}

  #endif

}

inline int init_mpi_thread(int* argc, char **argv[], int mode = 3)
{
  int provided;
  if(mode == 0)MPI_Init_thread(argc, argv, MPI_THREAD_SINGLE, &provided);
  if(mode == 1)MPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &provided);
  if(mode == 2)MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided);
  if(mode == 3)MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);

  int num_node;
  MPI_Comm_size(MPI_COMM_WORLD, &num_node);
  int id_node;
  MPI_Comm_rank(MPI_COMM_WORLD, &id_node);
  if (0 == id_node) {
    displayln("qlat::begin(): " +
              ssprintf("MPI Initialized. num_node = %d", num_node));
  }

  return num_node;
}

inline void begin_thread(
    int* argc, char** argv[],
    const std::vector<Coordinate>& size_node_list = std::vector<Coordinate>())
// begin Qlat and initialize a new comm
{
  const int num_node = init_mpi_thread(argc, argv);
  Coordinate size_node;
  for (int i = 0; i < (int)size_node_list.size(); ++i) {
    size_node = size_node_list[i];
    if (num_node == product(size_node)) {
      break;
    }   
  }
  if (num_node != product(size_node)) {
    size_node = plan_size_node(num_node);
  }
  begin_comm(get_comm(), size_node);
}

inline void print_mem_info(std::string stmp = "")
{
  print0("%s, ",stmp.c_str());
  #ifdef QLAT_USE_ACC
  double freeD = 0;double totalD=0;
  size_t freeM = 0;size_t totalM = 0;
  cudaMemGetInfo(&freeM,&totalM);
  freeD = freeM*pow(0.5,30);totalD = totalM*pow(0.5,30);
  #endif
  struct sysinfo s_info;
  sysinfo(&s_info);
  #ifdef QLAT_USE_ACC
  print0("===CPU free %.3e GB, total %.3e GB; GPU free %.3e GB, total %.3e GB. \n"
          , s_info.freeram*pow(0.5,30),s_info.totalram*pow(0.5,30),freeD,totalD);

  #else
  print0("===CPU free %.3e GB, total %.3e GB. \n"
          , s_info.freeram*pow(0.5,30),s_info.totalram*pow(0.5,30));
  #endif
}


inline int read_vector(const char *filename, std::vector<double > &dat)
{
  int prods = 0; 
  unsigned long Vsize = 0; 
  ////{synchronize();fflush(stdout);}

  if(qlat::get_id_node() == 0)
  {
    Vsize = get_file_size_o(filename);
    if(Vsize == 0){prods = 0;}else{prods=Vsize;}
    Vsize = Vsize/8;
  }
  sum_all_size(&prods,1);
  if(prods==0)return prods;

  sum_all_size((int*)&Vsize,1);
  dat.resize(Vsize);

  if(qlat::get_id_node() == 0)
  {
    FILE* filer = fopen(filename, "rb");
    unsigned long count = 1024*1024;
    unsigned long sizec = 0; 
    unsigned long offr  = 0; 
    for(unsigned int iv=0;iv<Vsize;iv++)
    {    
      if(offr >= Vsize*8)break;
      char* buf = (char *)&dat[offr/8];
      if((offr + count) <= (Vsize*8)){sizec = count;}
      else{sizec = Vsize*8 - offr;}

      fread(buf, 1, sizec, filer);
      offr = offr + sizec;
    }    

    fclose(filer);
  }

  sum_all_size(&dat[0],Vsize);
  return prods;

}

template<typename Yl>
void p_vector(const Yl teml)
{
  std::cout << teml << " ";
};

template<typename Ty>
void p_vector(const std::vector<Ty> teml)
{
  for(int i=0;i< teml.size();i++)
  {
    p_vector(teml[i]);
  }
  std::cout << std::endl;
};

template<typename Ty>
void p_vector(const qlat::vector<Ty> teml)
{
  for(int i=0;i< teml.size();i++)
  {
    p_vector(teml[i]);
  }
  std::cout << std::endl;
};

template<typename Ty>
inline void random_Ty(Ty* a, long N0,int GPU=0, int seed = 0)
{
  if(N0 == 0)return;
  qlat::RngState rs(qlat::get_id_node() + 1 + seed);

  double ini = qlat::u_rand_gen(rs);
  #ifdef QLAT_USE_ACC
  if(GPU == 1){
    size_t bfac = size_t(std::sqrt(N0));
    qacc_for(isp, long(N0/bfac + 1),{
     for(size_t i=0;i<size_t(bfac);i++){
      size_t off = isp*bfac + i;
      if(off < size_t(N0)){
        a[off] = Ty(std::cos((ini+isp)*0.5) , (5.0/(isp+1))*ini*0.1);
      }
    }
    });
    return ;
  }
  #endif

  #pragma omp parallel for
  for(size_t isp=0;isp< size_t(N0);isp++)
  {
     a[isp] = Ty(std::cos((ini+isp)*0.5) , (5.0/(isp+1))*ini*0.1);
  }

}

inline void random_EigenM(Evector& a,int GPU=0, int seed = 0)
{
  Complexq* buf = a.data();
  random_Ty(buf, a.size(), GPU, seed);
}

inline void random_EigenM(EigenM& a, int GPU=0, int seed = 0)
{
  int N0 = a.size();if(N0 == 0)return ;
  for(size_t i=0;i < size_t(N0);i++){random_EigenM(a[i], GPU,  seed + i);}
}

inline void zeroE(Evector& a,int GPU=0, bool dummy=true)
{
  zero_Ty(a.data(), a.size(), GPU, dummy);
}

inline void zeroE(EigenM& a,int GPU=0, bool dummy=true)
{
  for(LInt iv=0;iv<a.size();iv++){zeroE(a[iv], GPU, false);}
  if(dummy)qacc_barrier(dummy);
}

template <class T>
void random_link(GaugeFieldT<T >& g, int seed = -1)
{
  timeval tm;gettimeofday(&tm, NULL);
  int use_seed = 0;
  if(seed == -1){use_seed= int(tm.tv_sec);}else{use_seed = seed;}
  qlat::RngState rs(qlat::get_id_node() + 1 + use_seed);
  double ini = qlat::u_rand_gen(rs);

  int dir_limit = 4;
  const Geometry& geo = g.geo();

  qacc_for(isp,  geo.local_volume(),{
    for (int dir = 0; dir < dir_limit; ++dir) {
      const Coordinate xl = geo.coordinate_from_index(isp);
      ColorMatrixT<T >& link = g.get_elem(xl, dir);
      for(int ci=0; ci<9; ci++){
        link.p[ci] = T(std::cos((ini+isp)*0.5) , ((5.0+ci)/(isp+1))*ini*0.1);;
      } 
    }   
  }); 

}

template <class T>
void random_prop(Propagator4dT<T >& prop)
{
  timeval tm;gettimeofday(&tm, NULL);
  ////print0("print time %.3f\n", tm.tv_sec);
  qlat::RngState rs(qlat::get_id_node() + 1 +int(tm.tv_sec));
  double ini = qlat::u_rand_gen(rs);

  int dir_limit = 4;
  const Geometry& geo = prop.geo();

  qacc_for(isp,  geo.local_volume(),{
    qlat::WilsonMatrixT<T >& v0 =  prop.get_elem(isp);
    for(int ci=0;ci<12*12;ci++){
      v0.p[ci] = T(std::cos((ini+isp)*0.5) , ((5.0+ci)/(isp+1))*ini*0.1); 
    }
  }); 
}

size_t get_threads(size_t thread, size_t Total, int def=1)
{
  for(size_t temb=thread;temb<Total;temb++)
  {
    if(Total%temb == 0)
    {
      return temb;
    }
  }
  return def;
}

std::vector<unsigned int > get_num_power(const size_t x,const std::vector<unsigned int >& a)
{
  std::vector<unsigned int > re; re.resize(a.size());
  for(unsigned int ai=0;ai<a.size();ai++)
  {
    unsigned int    fac   = a[ai];
    unsigned int    count = 0;
    size_t use   = x;
    for(unsigned int j = 0;j < 300; j++)
    {
      if(use % fac == 0){
        count += 1;
        use = use/fac;}
      else{
        re[ai] = count;
        break;
      }
    }
  }
  size_t test = 1;
  for(unsigned int ai=0;ai<a.size();ai++){
    test = test * std::pow(a[ai], re[ai]);
  }
  assert(test == x);
  return re;
}

//////Most power in T direction
Coordinate spread_powT(const int n, const Coordinate& Lat, const std::vector<unsigned int >& a)
{
  std::vector<unsigned int > Mpow = get_num_power(n, a);
  std::vector<std::vector<unsigned int > > Lpow;
  Lpow.resize(4);for(int i=0;i<4;i++){Lpow[i] =  get_num_power(Lat[i], a);}
  Coordinate re;
  for(int i=0;i<4;i++)re[i] = 1;

  for(LInt i=0;i<Mpow.size();i++){
    int num = Mpow[i];
    if(num != 0){
      int suma = Lpow[0][i] + Lpow[1][i] + Lpow[2][i] + Lpow[3][i] ;
      assert(num < suma);
      unsigned int tem = num;
      for(unsigned int ni=0;ni<4;ni++){
        if(tem >= Lpow[4-ni-1][i]){
          re[4-ni-1] *= int(std::pow(a[i], Lpow[4-ni-1][i]));
          tem = tem - Lpow[4-ni-1][i];
        }
        else{
          re[4-ni-1] *= (unsigned int)(std::pow(a[i], tem));
          tem = 0;
        }
        ////if(tem >= Lpow[4-ni-1][i]){}
        if(tem == 0)break;
      }
    }
  }

  assert(re[0]*re[1]*re[2]*re[3] == n);
  return re;

}

Coordinate guess_nodeL(int n, const Coordinate& Lat, const int mode = 0)
{

  std::vector<unsigned int > a;a.resize(8);
  a[0] = 2;a[1] = 3;a[2] = 5;a[3] = 7;a[4] =11;a[5] =13;a[6] =17;a[7] =19;
  Coordinate re = spread_powT(n, Lat,a);
  return re;
}

void add_nodeL(std::vector<Coordinate>& size_node_list)
{
  size_node_list.push_back(Coordinate(1, 1, 1,  1));
  size_node_list.push_back(Coordinate(1, 1, 1,  2));
  size_node_list.push_back(Coordinate(1, 1, 3,  1));
  ////size_node_list.push_back(Coordinate(1, 1, 2,  2));
  size_node_list.push_back(Coordinate(1, 1, 1,  4));
  size_node_list.push_back(Coordinate(1, 1, 3,  2));
  size_node_list.push_back(Coordinate(1, 2, 3,  1));
  size_node_list.push_back(Coordinate(1, 2, 4,  1));
  size_node_list.push_back(Coordinate(1, 1, 1, 12));
  size_node_list.push_back(Coordinate(1, 1, 1, 16));
  size_node_list.push_back(Coordinate(1, 1, 6,  4));
  size_node_list.push_back(Coordinate(1, 1, 1, 32));
  size_node_list.push_back(Coordinate(2, 4, 4, 2 ));

  size_node_list.push_back(Coordinate(4, 4, 8, 16));
  size_node_list.push_back(Coordinate(4, 8, 8, 16));
  size_node_list.push_back(Coordinate(4, 4, 4,  6));
  size_node_list.push_back(Coordinate(4, 4, 4, 12));
  size_node_list.push_back(Coordinate(4, 4, 8, 12));
  size_node_list.push_back(Coordinate(4, 8, 8, 16));
}

inline void geo_to_nv(const qlat::Geometry& geo, std::vector<int >& nv, std::vector<int > &Nv, std::vector<int > &mv)
{
  Nv.resize(4);nv.resize(4);mv.resize(4);
  for(int i=0;i<4;i++){Nv[i]=geo.node_site[i];nv[i] = geo.node_site[i] * geo.geon.size_node[i];}
  for(int i=0;i<4;i++){mv[i] = nv[i]/Nv[i];}
}
inline void geo_to_nv(const qlat::Geometry& geo, qlat::vector_acc<int >& nv, qlat::vector_acc<int > &Nv, qlat::vector_acc<int > &mv)
{
  Nv.resize(4);nv.resize(4);mv.resize(4);
  for(int i=0;i<4;i++){Nv[i]=geo.node_site[i];nv[i] = geo.node_site[i] * geo.geon.size_node[i];}
  for(int i=0;i<4;i++){mv[i] = nv[i]/Nv[i];}
}



inline void begin_Lat(int* argc, char** argv[], inputpara& in, int mode_dis = -1)
{
  if(mode_dis == 0 or mode_dis == 1)
  {
    int n_node = init_mpi(argc, argv);
    in.load_para(*argc, *argv);
    Coordinate Lat(in.nx, in.ny, in.nz, in.nt);
    Coordinate spreadT = guess_nodeL(n_node, Lat);
    ///3D begin
    ////begin_comm(MPI_COMM_WORLD , spreadT);

    ///4D begin
    int id_node, n;
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_node);
    int t =  id_node/(spreadT[0]*spreadT[1]*spreadT[2]);
    int z = (id_node/(spreadT[0]*spreadT[1]))%(spreadT[2]);
    int y = (id_node/(spreadT[0]))%(spreadT[1]);
    int x = (id_node%(spreadT[0]));
    ///int new_id = ((z*spreadT[1] + y)*spreadT[0] + x)*spreadT[3] + t;
    int new_id = ((x*spreadT[1] + y)*spreadT[2] + z)*spreadT[3] + t;
    if(mode_dis == 0)begin(id_node, spreadT);
    if(mode_dis == 1)begin(new_id, spreadT);
  }

  if(mode_dis == -1)
  {
    std::vector<Coordinate> size_node_list;
    add_nodeL(size_node_list);
    begin(argc, argv, size_node_list);
    in.load_para(*argc, *argv);
  }

  set_GPU();

  omp_set_num_threads(omp_get_max_threads());
  print0("===nthreads %8d %8d, max %8d \n",qlat::qacc_num_threads(),omp_get_num_threads(),omp_get_max_threads());

  fflush_MPI();
  print_mem_info();


}

std::vector<long > job_create(long total, long each)
{
  if(total < 1 or each < 1){
    print0("Give me valid job types total %ld, each %ld \n", total, each);
    abort_r();}
  /////std::vector<long > a = job_create(total, each);
  std::vector<long > a;a.resize(0);
  long jobN  = (total + each - 1)/each;
  int i0 = 0; int dj = each;
  for(int ji = 0; ji < jobN ; ji++)
  {
    if(i0 >= total){break;}
    if(i0 + dj > total){dj = total - i0;}
    a.push_back(i0);
    a.push_back(dj);
    i0 += dj;
  }

  return a;
}

inline void allocate_buf(std::vector<qlat::vector_gpu<Complexq > > & buf, size_t n0, size_t n1)
{
  TIMERA("CUDA Buf mem allocation");
  buf.resize(n0);
  for(LInt i=0;i<buf.size();i++){
    buf[i].resize(n1);
    /////gpuMalloc(buf[i], n1, Complexq);
  }
}

inline void allocate_buf(std::vector<qlat::vector_acc<Complexq > > & buf, size_t n0, size_t n1)
{
  TIMERA("CUDA Buf mem allocation");
  buf.resize(0);
  buf.resize(n0);
  for(LInt i=0;i<buf.size();i++){
    buf[i].resize(0);
    buf[i].resize(n1);
  }
}

inline void allocate_buf(std::vector<Complexq* >& buf, size_t n0, size_t n1)
{
  TIMERA("CUDA Buf mem allocation");
  buf.resize(n0);
  for(LInt i=0;i<buf.size();i++){
    gpuMalloc(buf[i], n1, Complexq);
  }
}

inline Complexq inv_self(const Complexq& lam, double m, double rho,int one_minus_halfD=1)
{
  //Complexq tem = (one_minus_halfD>0)?(1-lam/2)/(rho*lam+m*(1-lam/2)):1.0/(rho*lam+m*(1-lam/2));
  std::complex<double > tem(lam.real(),lam.imag());
  std::complex<double > v0 = (one_minus_halfD>0)?(1.0-tem/2.0)/(rho*tem+m*(1.0-tem/2.0)):1.0/(rho*tem+m*(1.0-tem/2.0));
  Complexq res(v0.real(),v0.imag());
  return res;
}

template<typename Ty>
qlat::vector_acc<Ty* > EigenM_to_pointers(std::vector<qlat::vector_gpu<Ty > >& src)
{
  qlat::vector_acc< Ty* >  res;
  res.resize(src.size());
  for(LInt iv=0;iv<src.size();iv++)
  {
    res[iv] = src[iv].data();
  }
  return res;
}

template<typename Ty>
qlat::vector_acc<Ty* > EigenM_to_pointers(std::vector<qlat::vector_acc<Ty > >& src)
{
  qlat::vector_acc<Ty* >  res;
  res.resize(src.size());
  for(LInt iv=0;iv<src.size();iv++)
  {
    res[iv] = src[iv].data();
  }
  return res;
}



}

#endif
