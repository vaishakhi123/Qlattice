cdef class Coordinate:

    def __cinit__(self):
        pass

    def __init__(self, x = None):
        if isinstance(x, Coordinate):
            self.xx = (<Coordinate>x).xx
        elif isinstance(x, (list, tuple,)):
            assert len(x) == 4
            self.xx = cc.Coordinate(x[0], x[1], x[2], x[3])
        else:
            assert x is None

    def __imatmul__(self, Coordinate v1):
        self.xx = v1.xx
        return self

    def copy(self, cc.bool is_copying_data = True):
        cdef Coordinate x = Coordinate()
        if is_copying_data:
            x.xx = self.xx
        return x

    def __copy__(self):
        return self.copy()

    def __deepcopy__(self, memo):
        return self.copy()

    def __repr__(self):
        return f"Coordinate({self.to_list()})"

    def to_list(self):
        """
        Return a list composed of the 4 components of the coordinate.
        """
        return [ self.xx[0], self.xx[1], self.xx[2], self.xx[3], ]

    def from_list(self, list x):
        """
        set value based on a list composed of the 4 components of the coordinate.
        """
        assert isinstance(x, (list, tuple,))
        assert len(x) == 4
        self.xx = cc.Coordinate(x[0], x[1], x[2], x[3])

    def __getstate__(self):
        return self.to_list()

    def __setstate__(self, state):
        self.from_list(state)

    def sqr(self):
        """
        Return the square sum of all the components as ``long``.
        """
        return cc.sqr(self.xx)

    def __getitem__(self, int key):
        assert 0 <= key
        assert key < 4
        return self.xx[key]

    def __setitem__(self, int key, int val):
        assert 0 <= key
        assert key < 4
        cdef int* p_val = &self.xx[key]
        p_val[0] = val

    def from_index(self, long index, Coordinate size):
        self.xx = cc.coordinate_from_index(index, size.xx)

    def to_index(self, Coordinate size):
        return cc.index_from_coordinate(self.xx, size.xx)

def coordinate_from_index(long index, size):
    cdef Coordinate x = Coordinate()
    if isinstance(size, Coordinate):
        x.xx = cc.coordinate_from_index(index, (<Coordinate>size).xx)
        return x
    else:
        x.xx = cc.coordinate_from_index(index, cc.Coordinate(size[0], size[1], size[2], size[3]))
        return x.to_list()

def index_from_coordinate(x, size):
    if isinstance(x, Coordinate) and isinstance(size, Coordinate):
        return cc.index_from_coordinate((<Coordinate>x).xx, (<Coordinate>size).xx)
    else:
        return cc.index_from_coordinate(cc.Coordinate(x[0], x[1], x[2], x[3]),
                                        cc.Coordinate(size[0], size[1], size[2], size[3]))
