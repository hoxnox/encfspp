from nxtools import NxConanFile
from conans import CMake, tools
from shutil import copy


class EncfsppConan(NxConanFile):
    name = "encfspp"
    description = "C++ wrapper on encfs"
    version = "0.0.2"
    url = "https://github.com/hoxnox/encfspp"
    license = "http://github.com/hoxnox/encfspp/blob/master/LICENSE"
    settings = "os", "compiler", "build_type", "arch"
    options = {"tests":[True, False], "log":"ANY"}
    default_options = "tests=False", "log=", "libfuse:shared=False"
    build_policy = "missing"
    requires = ("libencfs/1.9.2@hoxnox/stable", "libfuse/2.9.7@hoxnox/stable")
    generators = "cmake"

    def requirements(self):
        if self.options.tests:
            self.requires("gtest/1.8.0@hoxnox/stable")
        if len(str(self.options.log)) == 0:
            self.requires("easyloggingpp/9.89@hoxnox/stable")

    def do_source(self):
        self.retrieve("95db0338f9ef18aec3cda806c7357ccc9d46ac53f3d7e7f57b21e5ce9963733b",
            [
                'vendor://hoxnox/encfspp/encfspp-{version}.tar.gz'.format(version=self.version),
                'https://github.com/hoxnox/encfspp/archive/{version}.tar.gz'.format(version=self.version)
            ], "encfspp-{v}.tar.gz".format(v = self.version))

    def do_build(self):
        cmake = CMake(self)
        cmake.build_dir = "{staging_dir}/src-encfspp".format(staging_dir=self.staging_dir)
        tools.untargz("encfspp-{v}.tar.gz".format(v=self.version), cmake.build_dir)
        copy('conanbuildinfo.cmake', cmake.build_dir)
        cmake_defs = {"CMAKE_INSTALL_PREFIX": self.staging_dir,
                      "WITH_TESTS": "1" if self.options.tests else "0"}
        if len(str(self.options.log)) != 0:
            cmake_defs.update({"WITH_LOG": self.options.log})
        cmake_defs.update(self.cmake_crt_linking_flags())
        cmake_defs.update(self.cmake_crt_linking_flags())
        cmake.configure(defs=cmake_defs, source_dir="encfspp-{v}".format(v=self.version))
        cmake.build(target="install")

    def do_package_info(self):
        self.cpp_info.libs = ["encfspp"]
        self.deps_cpp_info["libencfs"].libs.append("readpassphrase")



