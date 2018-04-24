###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
#  Copyright (C) : 2009-2017
#  European Synchrotron Radiation Facility
#  CS40220 38043 Grenoble Cedex 9 
#  FRANCE
# 
#  Contact: lima@esrf.fr
# 
#  This is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
# 
#  This software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

function(processlib_set_library_soversion lib_name version_file)
  
    file(STRINGS ${version_file}  version)
    # for lib version as 1.2.3 soverion is fixed to 1.2
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" soversion "${version}")
  
    set_target_properties(${lib_name} PROPERTIES VERSION "${version}" SOVERSION "${soversion}")
  
endfunction()


function(processlib_run_python_tests test_src)

	foreach(file ${test_src})
    add_test(NAME ${file}
      COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/${file}.py)
    if(WIN32)
        # Add the dlls to the %PATH%
        string(REPLACE ";" "\;" ESCAPED_PATH "$ENV{PATH}")
        set_tests_properties(${file} PROPERTIES ENVIRONMENT "PATH=${ESCAPED_PATH}\;$<SHELL_PATH:$<TARGET_FILE_DIR:processlib>>;PYTHONPATH=$<SHELL_PATH:${CMAKE_BINARY_DIR}/python>\;$<SHELL_PATH:$<TARGET_FILE_DIR:python_module_processlib>>")
    else()
        set_tests_properties(${file} PROPERTIES ENVIRONMENT "PYTHONPATH=$<SHELL_PATH:${CMAKE_BINARY_DIR}/python>$<SHELL_PATH:$<TARGET_FILE_DIR:python_module_processlib>>")
    endif()
	endforeach(file)

endfunction()