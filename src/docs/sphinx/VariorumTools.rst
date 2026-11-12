..
   # Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
   # Variorum Project Developers. See the top-level LICENSE file for details.
   #
   # SPDX-License-Identifier: MIT

###########################
 Integrating with Variorum
###########################

Variorum is a node-level library that can be integrated easily with higher-level
system software such as schedulers and runtime systems, to create a portable
`HPC PowerStack <https://variorum.readthedocs.io/en/latest/PowerStack.html>`_.
As part of our efforts to support a hierarchical, dynamic and open-source
portable power management stack, we have integrated Variorum with various
open-source system software. The `JSON API
<https://variorum.readthedocs.io/en/latest/api/json.html>`_ enables
Variorum to interface with higher-level system software in a portable and easy
manner.

******************
 ECP Integrations
******************

Current integration efforts include a `Kokkos <https://kokkos.org>`_ connector
for power monitoring, a `Caliper <https://software.llnl.gov/Caliper/>`_ service
for method-level power data, a `Flux <http://flux-framework.org>`_ power
management module for scheduling, and `Sandia's OVIS Lightweight Distributed
Metric Service (LDMS) <https://github.com/ovis-hpc/ovis-wiki/wiki>`_ monitoring
plugin. These enable Variorum to be used at scale across multiple layers of the
PowerStack, application/user layer, resource management layer, and system-level
layer. Upcoming integrations also include developing a Variorum interface for
`PowerAPI <https://pwrapi.github.io>`_ and `Intel GEOPM
<https://geopm.github.io>`_.

Links to Variorum's integrations with each of these frameworks can be found
below. Note that these integrations are in early development stages and are
expected to be updated to support more features and tests.

   -  `Variorum Kokkos connector
      <https://github.com/kokkos/kokkos-tools/tree/develop/profiling/variorum-connector>`_
   -  `Variorum Caliper service
      <https://github.com/LLNL/Caliper/tree/master/src/services/variorum>`_
   -  `Flux System Power Manager Module with Variorum
      <https://github.com/flux-framework/flux-power-mgr>`_
   -  `LDMS Power Monitoring Plugin with Variorum
      <https://github.com/ovis-hpc/ovis/tree/OVIS-4/ldms/src/contrib/sampler>`_

*************************************
 Contributing Integrations with JSON
*************************************

In order for tools to interact with Variorum, a simple JANSSON based parser is
sufficient. Our existing integration implementations, which are linked `here
<https://variorum.readthedocs.io/en/latest/VariorumTools.html#ecp-integrations>`_,
are a good starting point.

The format of the JSON object has been documented in :doc:`api/json` and
includes total node power, as well as CPU, memory, and GPU power when supported.
It also includes the hostname and timestamp.

We describe a simple example of how the data from the JSON object can be
extracted from Variorum in the client tool below, and we point developers to the
Kokkos and Flux integration links shown above for more detailed examples. We
have used the JANSSON library for our purposes, but the JSON object can also be
retrieved in a similar manner by other JSON libraries and supporting tools.

.. code:: c

   #include <jansson.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <variorum/variorum.h>

   void parse_variorum_data()
   {
       // Define a JSON object to retrieve data from Variorum in
       json_t *power_obj = NULL;
       char *s = NULL;

       // Define a local variable for the value of interest. For example, the
       // client side tool may only be interested in node power, or it may be
       // collecting samples for integration.
       double power_node;

       // Define a variable to capture appropriate return code from Variorum
       int ret;

       // Call the Variorum JSON API
       ret = variorum_get_power_json(&s);
       if (ret != 0)
       {
           printf("Variorum get node power API failed.\n");
           free(s);
           exit(-1);
       }

       // Extract the value of interest from the JSON object by using the
       // appropriate get function. Documentation of these can be found in the
       // JANSSON library documentation.
       power_obj = json_loads(s, JSON_DECODE_ANY, NULL);
       void *iter = json_object_iter(power_obj);
       json_t *node_obj = json_object_iter_value(iter);
       power_node = json_real_value(json_object_get(node_obj, "power_node_watts"));
       printf("Node power is: %lf\n", power_node);

       // Decrement references to JSON object, required for JANSSON library.
       json_decref(power_obj);

       // Deallocate the string
       free(s);
   }
