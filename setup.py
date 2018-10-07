from sysconfig import get_platform
from setuptools import setup, Extension


with open('README.rst') as fd:
	long_description = fd.read()


classifiers = [
	'Development Status :: 5 - Production/Stable',
	'Environment :: Win32 (MS Windows)',
	'Intended Audience :: Developers',
	'License :: OSI Approved :: MIT License',
	'Operating System :: Microsoft :: Windows',
	'Programming Language :: Python :: 3',
	'Programming Language :: Python :: 3.6',
	'Programming Language :: Python :: 3.7',
	'Programming Language :: Python :: 3 :: Only',
	'Programming Language :: Python :: Implementation :: CPython']


libs = ['user32', 'shell32', 'comctl32', 'gdi32']
if get_platform().startswith('mingw'):
	libs.append('python3.6m')
else:
	libs.append('python3')


_notify = Extension(
	'notify._notify',
	sources=['src/_notify.c'],
	libraries=libs)


setup(
	name='winnotify',
	version='0.1.0',
	description='C extension to show native Windows notifications.',
	long_description=long_description,
	url='https://github.com/OzymandiasTheGreat/winnotify',
	author='Tomas Ravinskas',
	author_email='tomas.rav@gmail.com',
	classifiers=classifiers,
	packages=['notify'],
	ext_modules=[_notify])
