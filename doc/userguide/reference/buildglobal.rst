
==========================
``globalSettings`` Section
==========================

The ``globalSettings`` section recognizes the following settings keys:

* `cacheMode`_
* `jobParallelism`
* `builderWaitTimeoutInMillis`
* `bootstrapDirectoryPath`
* `packageDirectoryPathList`
* `packageMap`

Settings
--------

``cacheMode``
.............

:Config Type: string
:Allowed Values:
   .. table::
      :align: left

      =====       ===========
      Value       Description
      =====       ===========
      Default     Use the default setting.
      Persistent  Cache artifacts in a persistent store in the workspace.
      InMemory    Cache artifacts in memory only.
      =====       ===========
:Default Value: "Default"

``jobParallelism``
..................

:Config Type: number
:Min Value: -1
:Default Value: 4

``builderWaitTimeoutInMillis``
..............................

:Config Type: number
:Min Value: 0
:Default Value: 1000

``bootstrapDirectoryPath``
..........................

:Config Type: string

``packageDirectoryPathList``
............................

:Config Type: seq

``packageMap``
..............

:Config Type: map
