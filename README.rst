winnotify
==========

This package provides a Python extension module written in C to show native
Windows notifications.
Notifications appear as toasts on Windows 10 and as balloons on earlier
versions.

There is already `win10toast`_ Python module which does more or less the
same thing, but as it depends on ``pywin32`` it is not usable in ``mingw``
environment, as ``pywin32`` does not compile under ``mingw``.

.. _win10toast: https://github.com/jithurjacob/Windows-10-Toast-Notifications/

Installation
-------------

Just do::

	pip install winnotify

or alternatively run::

	python setup.py install

from root directory.

Requirements
-------------

``winnotify`` does not depend on any 3rd party modules.
You will need Python 3.6 or later.

If you're compiling from source you'll need ``mingw`` or Windows SDK.

Example
--------

::

	>>> import notify
	>>> notify.init('python.ico', callback)
	>>> notify.notify('This notification body', 'Title', 'python.ico', False,
	... 5, notify.dwInfoFlags.NIIF_USER | notify.dwInfoFlags.NIIF_LARGE_ICON)
	>>> notify.uninit()

To check if notification is currently visible::

	>>> notify.isvisible()
	True

For complete documentation see ``help()``. Supported file types are ``ico`` and
``png``, either as standalone files, resources in binaries or memory buffers.
