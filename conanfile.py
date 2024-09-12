from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.cmake import CMakeToolchain
from conan.tools.cmake import CMakeDeps

class BJScript(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("zlib/1.3.1")
        
    def generate(self):
        cmake = CMakeDeps(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def layout(self):
        cmake_layout(self)