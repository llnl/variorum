..
   # Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
   # Variorum Project Developers. See the top-level LICENSE file for details.
   #
   # SPDX-License-Identifier: MIT

##########
 Variorum
##########

Variorum is an extensible, vendor-neutral library for exposing power and
performance capabilities of low-level hardware dials across diverse
architectures in a user-friendly manner. It is part of the :doc:`Argo`, and is a
key component for node-level power management in the :doc:`PowerStack`. Variorum
provides vendor-neutral APIs such that the user can query or control hardware
dials without needing to know the underlying vendor's implementation (for
example, model-specific registers or sensor interfaces). These APIs enable
application developers to gain a better understanding of power, energy, and
performance through various metrics. Additionally, the APIs may enable system
software to control hardware dials to optimize for a particular goal. Variorum
focuses on ease of use and reduced integration burden in applications, which it
accomplishes by providing:

-  Examples which demonstrate how to use Variorum in a stand-alone program.

-  A performance analysis sampler that runs alongside the application.

-  A JSON API to allow integration with higher-level system software, such as
   job schedulers, distributed monitoring frameworks, or application-level
   runtime systems.

****************************
 Variorum Project Resources
****************************

**Online Documentation** https://variorum.readthedocs.io/

**Github Source Repo** http://github.com/llnl/variorum

**Issue Tracker** http://github.com/llnl/variorum/issues

*****************
 Lead Developers
*****************

-  Stephanie Brink (LLNL)
-  Aniruddha Marathe (LLNL)
-  Tapasya Patki (LLNL)
-  Barry Rountree (LLNL)
-  Kathleen Shoga (LLNL)

*****************
 Code Of Conduct
*****************

See :doc:`CodeOfConduct`.

*****************
 Acknowledgments
*****************

This research was supported by the Exascale Computing Project (17-SC-20-SC), a
joint project of the U.S. Department of Energy's Office of Science and National
Nuclear Security Administration, responsible for delivering a capable exascale
ecosystem, including software, applications, and hardware technology, to support
the nation's exascale computing imperative.

************************
 Variorum Documentation
************************

.. toctree::
   :maxdepth: 2
   :caption: Basics

   QuickStart
   BuildingVariorum

.. toctree::
   :maxdepth: 2
   :caption: Usage

   Overview
   VariorumAPI
   Examples
   HWArchitectures
   VarMonitor
   Utilities

.. toctree::
   :maxdepth: 2
   :caption: API Docs

   api/print_functions
   api/cap_functions
   api/enable_disable_functions
   api/advanced_topology_functions
   api/json

.. toctree::
   :maxdepth: 2
   :caption: Contributing

   ContribGuide
   VariorumDevel
   UnitTesting
   VariorumTools

.. toctree::
   :maxdepth: 2
   :caption: Reference

   Argo
   PowerStack
   Publications

.. toctree::
   :maxdepth: 2
   :caption: Other

   Contributors
   CodeOfConduct
   Releases
   Licenses

********************
 Indices and tables
********************

-  :ref:`genindex`
-  :ref:`modindex`
-  :ref:`search`
