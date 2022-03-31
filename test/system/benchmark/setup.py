from setuptools import setup, find_packages


setup(
    name="PIBF",
    version="0.2.0",
    license="MIT",
    author="Isaac Baek",
    author_email="isaac.baek@samsung.com",
    description="POS Integrated Benchmark Framework",
    long_description=open("README.md").read(),
    packages=find_packages(exclude=["test"]),
    install_requires=["matplotlib>=3.3.4"]
)
