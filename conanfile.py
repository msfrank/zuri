from os.path import join

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load

class Zuri(ConanFile):
    name = 'zuri'

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'runtime_distribution_root': ['ANY', None],
        'enable_sanitizer': [True, False, None],
        'sanitizer': ['address', 'thread', 'memory', 'ub', 'leak', None],
        'enable_profiler': [True, False, None],
        'enable_docker_build': [True, False, None],
        'docker_program': ['ANY', None],
        'docker_requires_sudo': [True, False, None],
        'docker_platform_id': ['ANY', None],
        'docker_base_image': ['ANY', None],
        'docker_tempo_image': ['ANY', None],
        'docker_lyric_image': ['ANY', None],
        'docker_registry': ['ANY', None],
        }
    default_options = {
        'runtime_distribution_root': None,
        'enable_sanitizer': None,
        'sanitizer': None,
        'enable_profiler': None,
        'enable_docker_build': None,
        'docker_program': None,
        'docker_requires_sudo': None,
        'docker_platform_id': None,
        'docker_base_image': None,
        'docker_tempo_image': None,
        'docker_lyric_image': None,
        'docker_registry': None,
        }

    exports = ('meta/*',)

    exports_sources = (
        'CMakeLists.txt',
        'bin/*',
        'cmake/*',
        'lib/*',
        'meta/*',
        'pkg/*',
        'share/*',
        )

    requires = (
        'lyric/0.0.1',
        'tempo/0.0.1',
        # requirements from timbre
        'absl/20250127.1@timbre',
        'antlr/4.9.3@timbre',
        'boost/1.88.0@timbre',
        'cppterminal/20250728.1@timbre',
        'curl/8.5.0@timbre',
        'fmt/9.1.0@timbre',
        'flatbuffers/23.5.26@timbre',
        'gtest/1.14.0@timbre',
        'icu/77.1@timbre',
        'libedit/20180525.1@timbre',
        'rocksdb/10.4.2@timbre',
        'sqlite/3.49.2@timbre',
        'uv/1.51.0@timbre',
        )

    def _get_meta(self, key):
        return load(self, join(self.recipe_folder, "meta", key))

    def set_version(self):
        self.version = self._get_meta('version')
        self.license = self._get_meta('license')
        self.url = self._get_meta('url')
        self.description = self._get_meta('description')

    def validate(self):
        check_min_cppstd(self, "20")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        antlr = self.dependencies['antlr'].buildenv_info.vars(self)
        flatbuffers = self.dependencies['flatbuffers'].buildenv_info.vars(self)

        tc = CMakeToolchain(self)
        tc.cache_variables['ANTLR_TOOL_JAR'] = antlr.get('ANTLR_TOOL_JAR')
        tc.cache_variables['FLATBUFFERS_FLATC'] = flatbuffers.get('FLATBUFFERS_FLATC')

        if self.options.runtime_distribution_root:
            tc.cache_variables['RUNTIME_DISTRIBUTION_ROOT'] = self.options.runtime_distribution_root
        if self.options.enable_sanitizer:
            tc.cache_variables['ENABLE_SANITIZER'] = self.options.enable_sanitizer
        if self.options.sanitizer:
            tc.cache_variables['SANITIZER'] = self.options.sanitizer
        if self.options.enable_profiler:
            tc.cache_variables['ENABLE_PROFILER'] = self.options.enable_profiler
        if self.options.enable_docker_build:
            tc.cache_variables['ENABLE_DOCKER_BUILD'] = self.options.enable_docker_build
        if self.options.docker_program:
            tc.cache_variables['DOCKER_PROGRAM'] = self.options.docker_program
        if self.options.docker_requires_sudo is not None:
            tc.cache_variables['DOCKER_REQUIRES_SUDO'] = self.options.docker_requires_sudo
        if self.options.docker_platform_id:
            tc.cache_variables['DOCKER_PLATFORM_ID'] = self.options.docker_platform_id
        if self.options.docker_base_image:
            tc.cache_variables['DOCKER_BASE_IMAGE'] = self.options.docker_base_image
        if self.options.docker_tempo_image:
            tc.cache_variables['DOCKER_TEMPO_IMAGE'] = self.options.docker_tempo_image
        if self.options.docker_lyric_image:
            tc.cache_variables['DOCKER_LYRIC_IMAGE'] = self.options.docker_lyric_image
        if self.options.docker_registry:
            tc.cache_variables['DOCKER_REGISTRY'] = self.options.docker_registry

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
