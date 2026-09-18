..
   # Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
   # Variorum Project Developers. See the top-level LICENSE file for details.
   #
   # SPDX-License-Identifier: MIT

###################
 Building Variorum
###################

Variorum can be built from source with CMake or with ``spack``. Building
Variorum creates the ``libvariorum`` library, the ``var_monitor`` monitoring
tool, and Variorum examples.

********************
 Build Dependencies
********************

The build dependencies for a **minimal build** with no parallel components
require the following:

   -  C
   -  hwloc
   -  jansson
   -  Access to vendor-specific drivers or modules (see :doc:`HWArchitectures`).

For a **parallel build** with MPI/OpenMP, Variorum depends on:

   -  rankstr (only required for MPI/OpenMP builds)

The CMake variables (``ENABLE_MPI={ON,OFF}`` and ``ENABLE_OPENMP={ON,OFF}``)
control the building of parallel examples. If ``ENABLE_MPI=ON``, an MPI compiler
is required.

hwloc (Required)
================

`hwloc <https://www.open-mpi.org/projects/hwloc/>`_ is an open-source project
providing a portable abstraction of the hierarchical topology of modern
architectures.

Variorum leverages hwloc for detecting hardware topology. When reading/writing a
register on a particular hardware thread, hwloc can map that to the correct
physical socket.

jansson (Required)
==================

`jansson <https://digip.org/jansson/>`_ is an open-source C library for
encoding, decoding and manipulating JSON data.

Variorum leverages JANSSON to provide a JSON-based API that can retrieve power
data for external tools/software.

rankstr (Optional)
==================

`rankstr <https://github.com/ECP-VeloC/rankstr>`_ is an open-source C library
providing functions that identify unique strings across an MPI communicator.

Variorum leverages rankstr to split a communicator into subcommunicators by
hostname. The allows for a single control or monitor process in Variorum to for
example, enforce a power or frequency limit on a node or to print the hardware
counters once on a node.

*********************
 Building with CMake
*********************

Variorum can be built and installed as follows after cloning from GitHub:

.. code:: bash

   # install hwloc and jansson dependencies
   sudo apt-get install libhwloc15 libhwloc-dev libjansson4 libjansson-dev

   git clone https://github.com/llnl/variorum

   cd variorum
   mkdir build install
   cd build

   cmake -DCMAKE_INSTALL_PREFIX=../install ../src
   make -j8
   make install

*******************
 Host Config Files
*******************

To handle build options, third party library paths, etc., we rely on CMake's
initial-cache file mechanism. We call these initial-cache files ``host-config``
files, as we typically create a file for each platform or specific hosts if
necessary. These can be passed to CMake via the ``-C`` command line option as
shown below:

.. code:: bash

   cmake {other options} -C ../host-configs/{config_file}.cmake ../src

An example is provided in ``host-configs/boilerplate.cmake`` to create your own
configuration file. Example configuration files named by machine hostname, the
``SYS_TYPE`` environment variable, and platform name (via ``uname``) are also
provided in the ``host-configs`` directory. These files use standard CMake
commands. CMake ``set`` commands need to specify the root cache path as follows:

.. code:: cmake

   set(CMAKE_VARIABLE_NAME {VALUE} CACHE PATH "")

*********************
 CMake Build Options
*********************

Variorum's build system supports the following CMake options:

-  ``HWLOC_DIR`` - Path to an HWLOC install.

-  ``JANSSON_DIR`` - Path to a JANSSON install.

-  ``SPHINX_EXECUTABLE`` - Path to sphinx-build binary (required for
   documentation).

-  ``VARIORUM_WITH_AMD_CPU (default=OFF)`` - Enable Variorum build for AMD CPU
   architecture.

-  ``VARIORUM_WITH_AMD_GPU (default=OFF)`` - Enable Variorum build for AMD GPU
   architecture.

-  ``VARIORUM_WITH_AMD_APU (default=OFF)`` - Enable Variorum build for AMD APU
   architecture (e.g. MI300A). Requires ``ROCM_DIR`` to be set, or the AMD SMI
   library to be discoverable on the system.

-  ``VARIORUM_WITH_NVIDIA_GPU (default=OFF)`` - Enable Variorum build for Nvidia
   GPU architecture.

-  ``VARIORUM_WITH_IBM_CPU (default=OFF)`` - Enable Variorum build for IBM CPU
   architecture.

-  ``VARIORUM_WITH_ARM_CPU (default=OFF)`` - Enable Variorum build for ARM CPU
   architecture.

-  ``VARIORUM_WITH_INTEL_CPU (default=ON)`` - Enable Variorum build for Intel
   CPU architecture.

-  ``VARIORUM_WITH_INTEL_GPU (default=OFF)`` - Enable Variorum build for Intel
   discrete GPU architecture.

-  ``ENABLE_FORTRAN (default=ON)`` - Enable Fortran compiler for building
   example integration with Fortran application, Fortran compiler must exist.

-  ``ENABLE_PYTHON (default=ON)`` - Enable Python wrappers for adding PyVariorum
   examples.

-  ``ENABLE_MPI (default=OFF)`` - Enable MPI compiler for building MPI examples,
   MPI compiler must exist.

-  ``ENABLE_OPENMP (default=ON)`` - Enable OpenMP extensions for building OpenMP
   examples.

-  ``ENABLE_WARNINGS (default=OFF)`` - Build with compiler warning flags -Wall
   -Wextra -Werror, used primarily by developers.

-  ``BUILD_DOCS (default=ON)`` - Controls if the Variorum documentation is built
   (when sphinx and doxygen are found).

-  ``BUILD_SHARED_LIBS (default=ON)`` - Controls if shared (ON) or static (OFF)
   libraries are built.

-  ``BUILD_TESTS (default=ON)`` - Controls if unit tests are built.

-  ``VARIORUM_DEBUG (default=OFF)`` - Enable Variorum debug statements, useful
   if values are not translating correctly.

-  ``USE_MSR_SAFE_BEFORE_1_5_0 (default=OFF)`` - Use msr-safe prior to v1.5.0,
   dependency of Intel architectures for accessing counters from userspace.

************************************
 Building on Multiple Architectures
************************************

Several HPC systems have compute nodes that are composed of components or
hardware from different vendors. An example is the Sierra supercomputer at LLNL,
where the CPUs are based on IBM platform (Power9), and the GPUs are from NVIDIA
(Volta). Because power and energy dials are vendor-specific, we need to enable a
`multi-architecture` build for such platforms. As of version 0.7, we have added
support for such builds within Variorum. These can now be specified with turning
the CMake options for the associated devices, for example.
``VARIORUM_WITH_NVIDIA_GPU`` and ``VARIORUM_WITH_IBM_CPU`` can both be set to
``ON`` during the same build.

Support for multi-architecture builds is still new and limited, especially with
the JSON-based APIs. We are working toward adding more features in the near
future.

*********************
 Building with Spack
*********************

To install Variorum with all options (and also build all of its dependencies as
necessary) run:

.. code:: bash

   spack install variorum

The Variorum spack package provides several `variants
<http://spack.readthedocs.io/en/latest/basic_usage.html#specs-dependencies>`_
that customize the options and dependencies used to build Variorum (see table
below). Variants are enabled using ``+`` and disabled using ``~``.

+----------------+----------------------------------------+-------------------------------+
| Variant        | Description                            | Default                       |
+================+========================================+===============================+
| **shared**     | Build Variorum as shared library       | ON (+shared)                  |
+----------------+----------------------------------------+-------------------------------+
| **docs**       | Build Variorum's Documentation         | OFF (~docs)                   |
+----------------+----------------------------------------+-------------------------------+
| **log**        | Enable Variorum's logging              | OFF (~log)                    |
|                | infrastructure                         |                               |
+----------------+----------------------------------------+-------------------------------+
| **build_type** | Specify build type                     | Release with Debug Info       |
|                |                                        | (build_type=RelWithDebugInfo) |
+----------------+----------------------------------------+-------------------------------+

***********
 Debugging
***********

Setting the ``VARIORUM_LOG`` environment variable at runtime to
``VARIORUM_LOG=1`` will print out debugging information.
