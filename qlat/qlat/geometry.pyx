# cython: binding=True, embedsignature=True, c_string_type=unicode, c_string_encoding=utf8

from qlat_utils.all cimport *
from . cimport everything as cc
from cpython cimport Py_buffer
from cpython.buffer cimport PyBUF_FORMAT

import qlat_utils as q
import numpy as np

### -------------------------------------------------------------------

cdef class Geometry:

    def __cinit__(self):
        self.xx = cc.Geometry()
        self.cdata = <cc.Long>&(self.xx)

    def __init__(self, Coordinate total_site=None, int multiplicity=1):
        """
        if total_site is None: create geo uninitialized
        elif multiplicity is None: create geo with multiplicity = 1
        """
        if total_site is not None:
            self.xx.init(Coordinate(total_site).xx, multiplicity)

    def __imatmul__(self, Geometry v1):
        cc.assign_direct(self.xx, v1.xx)
        return self

    def copy(self, is_copying_data=True):
        x = Geometry()
        if is_copying_data:
            x @= self
        return x

    def __copy__(self):
        return self.copy()

    def __deepcopy__(self, memo):
        return self.copy()

    def total_site(self):
        cdef Coordinate x = Coordinate()
        cc.assign_direct(x.xx, self.xx.total_site())
        return x

    def total_volume(self):
        return self.xx.total_volume()

    def local_site(self):
        cdef Coordinate x = Coordinate()
        cc.assign_direct(x.xx, self.xx.local_site())
        return x

    def local_volume(self):
        return self.xx.local_volume()

    def multiplicity(self):
        return self.xx.multiplicity

    def eo(self):
        return self.xx.eo

    def expansion_left(self):
        cdef Coordinate x = Coordinate()
        cc.assign_direct(x.xx, self.xx.expansion_left)
        return x

    def expansion_right(self):
        cdef Coordinate x = Coordinate()
        cc.assign_direct(x.xx, self.xx.expansion_right)
        return x

    def id_node(self):
        return self.xx.geon.id_node

    def num_node(self):
        return self.xx.geon.num_node

    def coor_node(self):
        cdef Coordinate x = Coordinate()
        cc.assign_direct(x.xx, self.xx.geon.coor_node)
        return x

    def size_node(self):
        cdef Coordinate x = Coordinate()
        cc.assign_direct(x.xx, self.xx.geon.size_node)
        return x

    def __str__(self):
        return self.show()

    def __repr__(self):
        return self.show()

    def show(self):
        cdef Coordinate total_site = self.total_site()
        cdef int multiplicity = self.multiplicity()
        cdef Coordinate expan_left = self.expansion_left()
        cdef Coordinate expan_right = self.expansion_right()
        cdef int eo = self.eo()
        cdef Coordinate zero = Coordinate()
        if expan_left == zero and expan_right == zero and eo == 0:
            return f"Geometry({total_site.to_list()}, {multiplicity})"
        else:
            return f"Geometry({total_site.to_list()}, {multiplicity}, expan_left={expan_left.to_list()}, expan_right={expan_right.to_list()}, eo={eo})"

    def coordinate_g_from_l(self, Coordinate xl not None):
        cdef Coordinate xg = Coordinate()
        cc.assign_direct(xg.xx, self.xx.coordinate_g_from_l(xl.xx))
        return xg

    def coordinate_l_from_g(self, Coordinate xg not None):
        cdef Coordinate xl = Coordinate()
        cc.assign_direct(xl.xx, self.xx.coordinate_l_from_g(xg.xx))
        return xl

    def index_from_coordinate(self, Coordinate xg not None):
        return self.xx.index_from_coordinate(xg.xx)

    def coordinate_from_index(self, cc.Long index):
        cdef Coordinate xl = Coordinate()
        cc.assign_direct(xl.xx, self.xx.coordinate_from_index(index))
        return xl

    def is_local(self, Coordinate xl not None):
        return self.xx.is_local(xl.xx)

    def is_local_xg(self, Coordinate xg not None):
        """
        return a global coordinate is inside the local volume or not
        """
        cdef cc.Coordinate xl_xx = self.xx.coordinate_l_from_g(xg.xx)
        return self.xx.is_local(xl_xx)

    def xg_arr(self):
        """
        return xg for all local sites
        shape = (geo.local_volume(), 4,)
        """
        from .field_selection import mk_xg_field
        f_xg = mk_xg_field(self)
        xg_arr = np.asarray(f_xg, dtype=np.int32)
        cdef cc.Long local_volume = self.local_volume()
        return xg_arr.reshape((local_volume, 4,))

    def __getstate__(self):
        """
        Only work when single node (or if all nodes has the same data).
        Do not support expansion.
        """
        total_site = self.total_site()
        multiplicity = self.multiplicity()
        expan_left = self.expansion_left()
        expan_right = self.expansion_right()
        return [ total_site, multiplicity, expan_left, expan_right, ]

    def __setstate__(self, state):
        """
        Only work when single node (or if all nodes has the same data).
        Do not support expansion.
        """
        self.__init__()
        cdef Coordinate total_site
        cdef cc.Int multiplicity
        cdef Coordinate expan_left
        cdef Coordinate expan_right
        [ total_site, multiplicity, expan_left, expan_right, ] = state
        self.xx.init(total_site.xx, multiplicity)
        self.xx.expansion_left = expan_left.xx
        self.xx.expansion_right = expan_right.xx

### -------------------------------------------------------------------

def geo_reform(Geometry geo, int multiplicity=1, expansion_left=None, expansion_right=None):
    cdef Coordinate el, er
    if expansion_left is None:
        el = Coordinate()
    elif isinstance(expansion_left, int):
        e = expansion_left
        el = Coordinate([ e, e, e, e, ])
    elif isinstance(expansion_left, list):
        el = Coordinate(expansion_left)
    else:
        assert isinstance(expansion_left, Coordinate)
        el = <Coordinate>expansion_left
    if expansion_right is None:
        er = Coordinate()
    elif isinstance(expansion_right, int):
        e = expansion_right
        er = Coordinate([ e, e, e, e, ])
    elif isinstance(expansion_right, list):
        er = Coordinate(expansion_right)
    else:
        assert isinstance(expansion_left, Coordinate)
        er = <Coordinate>expansion_left
    cdef Geometry geo_new = Geometry()
    geo_new.xx = cc.geo_reform(geo.xx, multiplicity, el.xx, er.xx)
    return geo_new

def geo_eo(Geometry geo, int eo=0):
    cdef Geometry geo_new = Geometry()
    cc.assign_direct(geo_new.xx, cc.geo_eo(geo.xx, eo))
    return geo_new

### -------------------------------------------------------------------


