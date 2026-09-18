.. # Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
   # Variorum Project Developers. See the top-level LICENSE file for details.
   #
   # SPDX-License-Identifier: MIT

##########
 JSON API
##########

Variorum's JSON APIs return JANSSON-generated strings for integration with
external tools such as schedulers, monitoring frameworks, and runtime systems.
Each public JSON function takes a ``char **`` output argument. On success, the
function stores an allocated JSON string in that output argument; callers should
free the string after use.

All node-level JSON objects use the same top-level shape. The root key is the
actual hostname of the node where Variorum is running, and the value is the
node object:

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           ...
       }
   }

Unsupported fields or domains may be omitted. Some platform implementations use
``-1.0`` for measurements that are part of the vendor-neutral schema but cannot
be measured directly on that platform. Per-socket keys are emitted by the
platform backend and may appear as ``Socket_N`` or ``socket_N``.

Function Reference
==================

The public JSON function declarations and architecture support lists are
generated from ``variorum/variorum.h``:

.. toctree::
   :maxdepth: 1

   json_support_functions

Power
=====

``variorum_get_power_json(char **get_power_obj_str)`` returns node, socket, CPU,
memory, GPU, APU, and I/O power values when supported by the current build and
platform. The node power value may be reported directly by the hardware or
estimated by summing available domains.

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           "power_node_watts": <real>,
           "<socket_key>": {
               "power_cpu_watts": <real>,
               "power_mem_watts": <real>,
               "power_gpu_watts": <real or per-GPU object>,
               "power_apu_watts": <per-APU object>,
               "power_io_watts": <real>
           }
       }
   }

For Intel platforms, total node power is estimated when the hardware does not
directly report it. IBM Power9 reports total node power through the PWRSYS
sensor. GPU-only builds may report GPU power without ``power_node_watts``.

Power Domains
=============

``variorum_get_node_power_domain_info_json(char **get_domain_obj_str)`` returns
the measurable and controllable power domains for the current platform, including
units and control ranges where available.

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           "control": {
               "power_cpu": {
                   "min": <real>,
                   "max": <real>,
                   "units": "Watts"
               }
           },
           "unsupported": [
               "power_node"
           ],
           "measurement": {
               "power_cpu": {
                   "units": "Watts"
               },
               "power_mem": {
                   "units": "Watts"
               }
           }
       }
   }

The exact domains differ by architecture. A domain may appear under
``unsupported`` when the vendor-neutral API names it but the current platform
cannot measure or control it.

Thermals
========

``variorum_get_thermals_json(char **get_thermal_obj_str)`` returns temperature
readouts for CPU, memory, GPU, or APU sensors when supported.

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           "<socket_key>": {
               "CPU": {
                   "Core": {
                       "temp_celsius_core_0": <real>
                   },
                   "Mem": {
                       "temp_celsius_dimm_0": <real>
                   }
               },
               "GPU": {
                   "temp_celsius_gpu_0": <real>
               }
           }
       }
   }

Frequency
=========

``variorum_get_frequency_json(char **get_frequency_obj_str)`` returns CPU and
GPU frequency information when supported.

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           "<socket_key>": {
               "CPU": {
                   "cpu_avg_freq_mhz": <real>,
                   "core": {
                       "core_0_avg_freq_mhz": <real>
                   }
               },
               "GPU": {
                   "gpu_0_freq_mhz": <real>
               }
           }
       }
   }

Utilization
===========

``variorum_get_utilization_json(char **get_util_obj_str)`` returns CPU, memory,
and GPU utilization when supported. CPU utilization is computed from
``/proc/stat`` and memory utilization from ``/proc/meminfo``. GPU utilization is
obtained through the vendor GPU management library available in the build.

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           "CPU": {
               "total_util%": <real>,
               "user_util%": <real>,
               "system_util%": <real>
           },
           "memory_util%": <real>,
           "GPU": {
               "Socket_0": {
                   "GPU0_util%": <real>
               }
           }
       }
   }

Energy
======

``variorum_get_energy_json(char **get_energy_obj_str)`` returns accumulated
energy values when supported.

.. code-block:: text

   {
       "<hostname>": {
           "timestamp": <integer>,
           "energy_node_joules": <real>,
           "<socket_key>": {
               "energy_cpu_joules": <real>,
               "energy_mem_joules": <real>,
               "energy_gpu_joules": <real>,
               "energy_apu_joules": <per-APU object>
           }
       }
   }

Examples
========

See the JSON examples in ``src/examples``:

-  ``variorum-get-power-json-example.c``
-  ``variorum-get-node-power-domain-info-json-example.c``
-  ``variorum-get-thermals-json-example.c``
-  ``variorum-get-frequency-json-example.c``
-  ``variorum-get-utilization-json-example.c``
-  ``variorum-get-energy-json-example.c``
-  ``variorum-integration-using-json-example.c``
