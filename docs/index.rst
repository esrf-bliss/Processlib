=========================================
Processlib : Library for Image Processing
=========================================

Processlib is a companion library of LImA (**L** ibrary for **Im** age **A** cquisition) project that provides image processing.

Processlib is a C++ library but the library also comes with a Python_ binding.

We provide Conda binary package for Windows and Linux for some cameras. Check out our `Conda channel <https://anaconda.org/esrf-bcu>`_.

If you want to get in touch with the LIMA community, please send an email to lima@esrf.fr. You may also want to subscribe to our mailing list by sending a message to `sympa@esrf.fr <mailto:sympa@esrf.fr?subject=subscribe%20lima>`_ with ``subscribe lima`` as subject.

For the latest changes, refers to the :download:`Release Notes <../ReleaseNotes.txt>`.

Note that this documentation is also available in `pdf`_ and `epub`_ format.

.. toctree::
  :maxdepth: 3
  :caption: Installation

  requirements
  build_install
  install_tango_device_server

.. toctree::
  :maxdepth: 1
  :caption: Reference Documentation

  cpp_api
  python_api
  
.. toctree::
  :maxdepth: 2
  :caption: How to contribute

  howto_contribute

.. _pdf: http://readthedocs.org/projects/lima-doc/downloads/pdf/latest/
.. _epub: http://readthedocs.org/projects/lima-doc/downloads/epub/latest/
.. _release notes: ./ReleaseNotes.txt

.. _Python: http://python.org
.. _PyTango: http://github.com/tango-cs/pytango
