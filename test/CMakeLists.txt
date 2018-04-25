############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2017
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

# Copy tests input files
configure_file(crl8_0.edf crl8_0.edf COPYONLY)

# Build and run cpp tests
set (test_src test_data test_background_substraction test_bpm test_process)
processlib_run_tests(${test_src})

# Build and run python tests
set(python_test_src BpmTest CallBackTest ProcessTest RoiCounterTest)
processlib_run_python_tests(${python_test_src})
