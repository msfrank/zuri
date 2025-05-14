from os.path import join

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class Zuri(ConanFile):
    name = 'zuri'
    version = '0.0.1'
    license = 'BSD-3-Clause, AGPL-3.0-or-later'
    url = 'https://github.com/msfrank/zuri'
    description = 'Programming language and development environment'

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {'shared': [True, False], 'compiler.cppstd': ['17', '20'], 'build_type': ['Debug', 'Release']}
    default_options = {'shared': True, 'compiler.cppstd': '20', 'build_type': 'Debug'}

    exports_sources = (
        'CMakeLists.txt',
        'bin/*',
        'cmake/*',
        'pkg/*',
        'share/*',
        )

    requires = (
        'lyric/0.0.1',
        'tempo/0.0.1',
        # requirements from timbre
        'absl/20250127.1@timbre',
        'boost/1.88.0@timbre',
        'cppterminal/20231011.1@timbre',
        'fmt/9.1.0@timbre',
        'flatbuffers/23.5.26@timbre',
        'gtest/1.14.0@timbre',
        'icu/77.1@timbre',
        'libedit/20180525.1@timbre',
        'rocksdb/8.5.3@timbre',
        'uv/1.44.1@timbre',
        )

    def validate(self):
        check_min_cppstd(self, "20")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables['ZURI_PACKAGE_VERSION'] = self.version
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs.append(join("lib", "cmake", "zuri"))
