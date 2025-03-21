import pytest
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


def test_regular_mean():
    """
    Test a binning with squre bins and input.

    The binning is regular as every bins have the same size.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 3
    b.mYFactor = 3
    b.mOperation = processlib.Tasks.Binning.MEAN

    src = processlib.Data()
    src.buffer = numpy.arange(6*6, dtype=numpy.ushort).reshape((6, 6))
    dst = b.process(src)
    expected = numpy.array([[63, 90],[225, 252]]) // 9
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_non_regular_mean():
    """
    Test a binning with rect bins and rect input.

    The binning is non regular as the input is not a multiple of the binning.

    The result is clopped to only use full bins.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 3
    b.mYFactor = 3
    b.mOperation = processlib.Tasks.Binning.MEAN

    src = processlib.Data()
    src.buffer = numpy.ones(8*8, dtype=numpy.ushort).reshape((8, 8))
    dst = b.process(src)
    expected = numpy.array([[1, 1],[1, 1]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_rect_mean():
    """
    Test a binning with rect bins and rect input.

    Check that there is no swap between height and width params.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 3
    b.mYFactor = 2
    b.mOperation = processlib.Tasks.Binning.MEAN

    src = processlib.Data()
    src.buffer = numpy.arange(6*4, dtype=numpy.ushort).reshape((4, 6))
    dst = b.process(src)
    expected = numpy.array([[24, 42],[96, 114]]) // 6
    numpy.testing.assert_almost_equal(dst.buffer, expected)


def test_overflow_mean():
    """
    Check that there is no overflow during the mean reduction.

    The input and output are still UINT8.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = 2
    b.mYFactor = 2
    b.mOperation = processlib.Tasks.Binning.MEAN

    src = processlib.Data()
    src.buffer = numpy.array([[255, 255], [255, 251]], dtype=numpy.uint8)
    dst = b.process(src)
    expected = numpy.array([[254]])
    numpy.testing.assert_almost_equal(dst.buffer, expected)
    assert dst.buffer.dtype == numpy.uint8


@pytest.mark.parametrize("input_shape, bin_shape, expected_result, expected_shape", [
    pytest.param((1024, 1024), (2, 2), 1, (512, 512), id="big_square"),
    pytest.param((2048, 1024), (2, 2), 1, (1024, 512), id="big_rect"),
    pytest.param((1024, 2048), (2, 2), 1, (512, 1024), id="big_rect2"),
])
def test_mean_robustness(input_shape, bin_shape, expected_result, expected_shape):
    """
    Check that there is no menory corruption.
    """
    b = processlib.Tasks.Binning()
    b.mXFactor = bin_shape[0]
    b.mYFactor = bin_shape[1]
    b.mOperation = processlib.Tasks.Binning.MEAN

    src = processlib.Data()
    src.buffer = numpy.ones(input_shape, dtype=numpy.uint8)
    dst = b.process(src)
    assert dst.buffer.dtype == numpy.uint8
    if expected_shape is not None:
        assert dst.buffer.shape == expected_shape
    if expected_result is not None:
        numpy.testing.assert_allclose(dst.buffer, expected_result)
