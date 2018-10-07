from collections import Sequence, ByteString
from enum import IntFlag
from . import _notify


__all__ = ['dwInfoFlags', 'init', 'notify', 'uninit', 'isinitted', 'isvisible']


class dwInfoFlags(IntFlag):

	"""Use the tray icon (icon passed to init)."""
	NIIF_NONE = 0x00000000
	"""Use the standard information icon."""
	NIIF_INFO = 0x00000001
	"""Use the standard warning icon."""
	NIIF_WARNING = 0x00000002
	"""Use the standard error icon."""
	NIIF_ERROR = 0x00000003
	"""Use user provided icon (icon passed to notify)."""
	NIIF_USER = 0x00000004
	"""Silent notification (do not produce a sound)."""
	NIIF_NOSOUND = 0x00000010
	"""Use large icon (48x48)."""
	NIIF_LARGE_ICON = 0x00000020
	"""See MSDN for explanation about this one."""
	NIIF_RESPECT_QUIET_TIME = 0x00000080


def init(icon, callback):
	"""Initialize the module and setup notification callback.

	Args:
		icon (str): The icon to show in the tray area while notification is
			shown. Can be either :obj:`str` specifying the path to ico/png
			file, :obj:`Sequence` where first member is the path to binary
			(dll/exe) and the second is the ordinal number of the resource, or
			bytes like object containing image (ico/png) data.
		callback (callable): A callable object that will be called with no
			arguments if the user clicks on a notification.
	"""

	if isinstance(icon, str):
		return _notify.InitFromIcon(icon, callback)
	elif isinstance(icon, ByteString):
		return _notify.InitFromBuffer(icon, callback)
	elif isinstance(icon, Sequence):
		return _notify.InitFromResource(icon, callback)
	else:
		raise TypeError('Unrecognized icon type.')


def notify(body, title, icon, realtime, timeout, flags):
	"""Display a notification.

	Args:
		body (str): A string of up to 256 characters. This is the main text
			of a notification.
		title (str): A string of up to 64 characters. This is the bold text
			at the top of a notification.
		icon (str): The icon to show in the notification when using the
			NIIF_USER flag. Can be either :obj:`str` specifying path to
			ico/png file, :obj:`Sequence` where first member is the path
			to binary and the second is the ordinal number of the resource,
			or bytes like object containing image (ico/png) data.
		realtime (bool): Whether notification should be displayed in realtime.
			For exact definition of what this means see MSDN.
		timeout (int): Timeout in seconds for how long the notification is to
			be displayed. Ignored on Windows 10.
		flags (dwInfoFlags): Flags specifying icon to use and other parameters
			of a notification.
	"""

	if isinstance(icon, str):
		return _notify.NotifyFromIcon(body, title, icon, realtime, timeout, flags)
	elif isinstance(icon, ByteString):
		return _notify.NotifyFromBuffer(body, title, icon, realtime, timeout, flags)
	elif isinstance(icon, Sequence):
		return _notify.NotifyFromResource(body, title, icon, realtime, timeout, flags)
	else:
		raise TypeError('Unrecognized icon type.')


def uninit():
	"""Deinitialize the module, closing mainloop and freeing used resources.
	"""

	return _notify.Uninit()


def isinitted():
	"""Check if module is initialized.
	"""

	return _notify.IsInitted()


def isvisible():
	"""Check whether a notification is currently displayed.
	"""

	return _notify.IsVisible()
