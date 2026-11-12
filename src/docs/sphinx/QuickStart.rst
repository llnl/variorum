..
   # Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
   # Variorum Project Developers. See the top-level LICENSE file for details.
   #
   # SPDX-License-Identifier: MIT

###################
 Quick Start Guide
###################

The simplest way to install Variorum is using the default CMake settings.
Building Variorum requires the ``hwloc`` and ``jansson`` libraries. The default
build targets the Intel platform and assumes ``msr-safe`` kernel module access.

.. code:: bash

   # install hwloc and jansson dependencies
   sudo apt-get install libhwloc15 libhwloc-dev libjansson4 libjansson-dev

   git clone https://github.com/LLNL/variorum.git

   cd variorum
   mkdir build install
   cd build

   cmake -DCMAKE_INSTALL_PREFIX=../install ../src
   make -j8
   make install

Note that ``HWLOC_DIR`` and ``JANSSON_DIR`` may need to be set correctly if
installing from scratch. Variorum also supports ``host-config`` files, which
make the build process easier by setting all necessary paths in one place.
Details on using host configuration files can be found `here
<https://variorum.readthedocs.io/en/2022_update_docs/BuildingVariorum.html#host-config-files>`_.

Please ensure that the dependencies for each platform are met before building
Variorum. These include the kernel module ``msr-safe`` for Intel systems,
``msr``, ``amd_energy_driver`` and HSMP driver for AMD sytems, OPAL firmware and
sensors for IBM, and NVML for NVIDIA. Details of each of these can be found in
the respective vendor pages, see :doc:`HWArchitectures`.

For more details about building and installing Variorum, see
:doc:`BuildingVariorum`, which provides detailed information about building
Variorum for specific hosts, Variorum's other CMake options and installing with
``spack``.

Function-level descriptions of Variorum's APIs as well as the architectures that
have implementations in Variorum are provided in the following sections:

-  :doc:`api/print_functions`
-  :doc:`api/cap_functions`
-  :doc:`api/enable_disable_functions`
-  :doc:`api/advanced_topology_functions`
-  :doc:`api/json`

For beginners, the `ECP Variorum Lecture Series
<https://www.exascaleproject.org/event/variorum-class-series/>`_ is beneficial.
