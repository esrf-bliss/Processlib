import numpy
import processlib


def test_square_regular_2x2():
    """
    Test a binning 2x2 with a share input.

    The binning is regular as every bins have the same size.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 2
    b.mYFactor = 2

    src = processlib.Data()
    src.buffer = numpy.arange(4*4, dtype=numpy.ushort).reshape((4, 4))
    dst = b.process(src)
    expected = numpy.array([[10, 18],[42, 50]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_square_regular_3x3():
    """
    Test a binning 3x3 with a share input.

    The binning is regular as every bins have the same size.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 3
    b.mYFactor = 3

    src = processlib.Data()
    src.buffer = numpy.arange(6*6, dtype=numpy.ushort).reshape((6, 6))
    dst = b.process(src)
    expected = numpy.array([[63, 90],[225, 252]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_rect_3x2():
    """
    Test a binning with rect bins and rect input.

    The binning is regular as every bins have the same size.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 3
    b.mYFactor = 2

    src = processlib.Data()
    src.buffer = numpy.arange(6*4, dtype=numpy.ushort).reshape((4, 6))
    dst = b.process(src)
    expected = numpy.array([[24, 42],[96, 114]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_non_regular_2x2():
    """
    Test a binning with rect bins and rect input.

    The binning is non regular as the input is not a multiple of the binning.

    The result is clopped to only use full bins.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 2
    b.mYFactor = 2

    src = processlib.Data()
    src.buffer = numpy.ones(5*5, dtype=numpy.ushort).reshape((5, 5))
    dst = b.process(src)
    expected = numpy.array([[4, 4],[4, 4]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_non_regular_3x3():
    """
    Test a binning with rect bins and rect input.

    The binning is non regular as the input is not a multiple of the binning.

    The result is clopped to only use full bins.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 3
    b.mYFactor = 3

    src = processlib.Data()
    src.buffer = numpy.ones(8*8, dtype=numpy.ushort).reshape((8, 8))
    dst = b.process(src)
    expected = numpy.array([[9, 9],[9, 9]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)
