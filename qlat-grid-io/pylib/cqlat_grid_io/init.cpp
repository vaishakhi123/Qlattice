#include "lib.h"

#include <qlat-grid-io/qlat-grid-io.h>

EXPORT(begin_with_grid, {
  using namespace qlat;
  PyObject* p_sargs = NULL;
  PyObject* p_size_node_list = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_sargs, &p_size_node_list)) {
    return NULL;
  }
  static std::vector<std::string> sargs =
      py_convert_data<std::vector<std::string> >(p_sargs);
  const std::vector<Coordinate> node_size_list =
      py_convert_data<std::vector<Coordinate> >(p_size_node_list);
  // make cargs
  static std::vector<const char*> cargs(sargs.size() + 1);
  for (long i = 0; i < (long)sargs.size(); ++i) {
    cargs[i] = sargs[i].c_str();
  }
  cargs.back() = NULL;
  //
  int argc = (int)sargs.size();
  char** argv = (char**)&cargs[0];
  //
  grid_begin(&argc, &argv, node_size_list);
  Py_RETURN_NONE;
})

EXPORT(end_with_grid, {
  using namespace qlat;
  bool is_preserving_cache = false;
  if (!PyArg_ParseTuple(args, "b", &is_preserving_cache)) {
    return NULL;
  }
  grid_end(is_preserving_cache);
  Py_RETURN_NONE;
})
