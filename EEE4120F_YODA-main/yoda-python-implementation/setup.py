from setuptools import setup, Extension
import sysconfig

# Get the actual Python include path
python_include = sysconfig.get_path('include')
print(f"Using Python include path: {python_include}")

module = Extension(
    'rdtsc',
    sources=['rdtsc.c'],
    include_dirs=[python_include],
)

setup(
    name='rdtsc',
    version='1.0',
    ext_modules=[module],
)