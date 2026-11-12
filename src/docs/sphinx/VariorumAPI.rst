..
   # Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
   # Variorum Project Developers. See the top-level LICENSE file for details.
   #
   # SPDX-License-Identifier: MIT

##############
 Variorum API
##############

Variorum supports vendor-neutral power and energy management through its rich
API. Please refer to the top-level API, as well as the specific descriptions of
the JSON API and the Best Effort Power Capping API. The JSON API allows system
software interacting with Variorum to obtain data in a portable, vendor-neutral
manner.

***************
 Top-level API
***************

The top-level API for Variorum is in the ``variorum.h`` header file.
Function-level descriptions as well as the architectures that have
implementations in Variorum are described in the following sections:

-  :doc:`api/print_functions`
-  :doc:`api/cap_functions`
-  :doc:`api/enable_disable_functions`
-  :doc:`api/advanced_topology_functions`
-  :doc:`api/json`

*******************
 Variorum Wrappers
*******************

As of v0.6.0, Variorum also supports Fortran and Python APIs for Variorum, these
can be found in the ``src/wrappers`` directory. By default, these wrappers will
be enabled. The Fortran wrapper is built and installed if Fortran is found and
enabled. For the Python module (called ``pyVariorum``), a ``pip`` based install
or setting of ``PYTHONPATH`` is needed. Please refer to the ``README`` in the
``src/wrappers/python`` directory for the details. Examples on the usage of
these wrappers can be found in the ``src/examples/fortran-examples`` and the
``src/examples/python-examples`` directories, respectively.

**********
 JSON API
**********

The current JSON API depends on the JANSSON-C library and has a vendor-neutral
format. It can be used to integrate Variorum with higher-level system software
such as schedulers, monitoring frameworks, and runtime systems. See
:doc:`api/json` for the JSON format guide and function reference.

***************************
 Best Effort Power Capping
***************************

We support setting best effort node power limits in a vendor-neutral manner.
This interface has been developed from the point of view of higher-level tools
that utilize Variorum on diverse architectures and need to make node-level
decisions. When the underlying hardware does not directly support a node-level
power cap, a best-effort power cap is determined in software to provide an
easier interface for higher-level tools (e.g. Flux, Kokkos, etc).

For example, while IBM Witherspoon inherently provides the ability to set a
node-level power cap in watts in hardware through its OPAL infrastructure, Intel
architectures currently do not support a direct node level power cap through
MSRs. Instead, on Intel architectures, fine-grained CPU and DRAM level power
caps can be dialed in using MSRs. Note that IBM Witherspoon does not provide
fine-grained capping for CPU and DRAM level, but allows for a power-shifting
ratio between the CPU and GPU components on a socket (see `IBM documentation
<https://variorum.readthedocs.io/en/latest/IBM.html>`_).

Our API, ``variorum_cap_best_effort_node_power_limit()``, allows us to set a
best effort power cap on Intel architectures by taking the input power cap
value, and uniformly distributing it across sockets as CPU power caps.
Currently, we do not set memory power caps, but we plan to develop better
techniques for best-effort software capping in the future.
