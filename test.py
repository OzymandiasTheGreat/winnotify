import time
import notify


def callback_from_icon():

	print('Notification from icon clicked!')


def callback_from_resource():

	print('Notification from resource clicked!')


def callback_from_buffer():

	print('Notification from buffer clicked!')


print('Initializing from icon: ', notify.init('python.ico', callback_from_icon))
time.sleep(3)
print(
	'Notifying from icon: ', notify.notify(
		'This is a test notification from icon file.',
		'Test #1', 'python.ico', False, 3,
		notify.dwInfoFlags.NIIF_USER | notify.dwInfoFlags.NIIF_LARGE_ICON))
time.sleep(5)
print('Cleaning up: ', notify.uninit())


print('Initializing from resource: ', notify.init(('wmploc.dll', 29518), callback_from_resource))
time.sleep(3)
print(
	'Notifying from resource: ', notify.notify(
		'This is a test notification from resource.',
		'Test #2', ('wmploc.dll', 29654), False, 3,
		notify.dwInfoFlags.NIIF_USER | notify.dwInfoFlags.NIIF_LARGE_ICON))
time.sleep(5)
print('Cleaning up: ', notify.uninit())


with open('python.ico', 'rb') as fd:
	print('Notifying from buffer: ', notify.init(fd.read(), callback_from_buffer))
time.sleep(3)
with open('python.ico', 'rb') as fd:
	print(
		'Notifying form buffer: ', notify.notify(
			'This is a test notification from buffer.',
			'Test #3', fd.read(), False, 3,
			notify.dwInfoFlags.NIIF_USER | notify.dwInfoFlags.NIIF_LARGE_ICON))
time.sleep(5)
print('Cleaning up: ', notify.uninit())
