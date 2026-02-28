# Copyright (C) 2026  aSocial Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Author: Rabit (@rabits)

# Building tests
option(BUILD_TESTS "Enable tests to be built" OFF)

if(BUILD_TESTS)
    include(CTest)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Core Test)
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_link_libraries(${TEST_NAME} PRIVATE Qt::Core Qt::Test)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

        # Using the same project's target params for test executable
        get_target_property(project_defs ${PROJECT_NAME} COMPILE_DEFINITIONS)
        target_compile_definitions(${TEST_NAME} PRIVATE ${project_defs})
        get_target_property(project_incdirs ${PROJECT_NAME} INCLUDE_DIRECTORIES)
        target_include_directories(${TEST_NAME} PRIVATE ${project_incdirs})
        get_target_property(project_linklibs ${PROJECT_NAME} LINK_LIBRARIES)
        target_link_libraries(${TEST_NAME} PRIVATE ${project_linklibs})
        get_target_property(project_deps ${PROJECT_NAME} MANUALLY_ADDED_DEPENDENCIES)
        add_dependencies(${TEST_NAME} ${project_deps})

        add_dependencies(${TEST_NAME} ${PROJECT_NAME}_impl)
        target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME}_impl)
        target_link_directories(${TEST_NAME} PRIVATE ${LIBS_RESULT_DIR}/lib)
        target_include_directories(${TEST_NAME} PRIVATE ${LIBS_RESULT_DIR}/include)
    endforeach()
endif()
