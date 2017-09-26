# encfspp

Easy to use library, used to simplify mounting of
[encfs](https://github.com/vgough/encfs) directories from your code.

## Building

You need conan (see http://conan.io) and cmake to build the project.

```sh
mkdir build && cd build
conan remote add hoxnox https://api.bintray.com/conan/hoxnox/conan
conan install --build=missing ..
cmake ..
make
```

Conan scripts conforms [vendor agreements](http://blog.hoxnox.com/vendoring_with_conan).
So you can build on a standalone server.

### Exmaple

```c++
#include <string>
#include <EncfsMounter.hpp>

int main(int argc, char* argv[])
{
	EncfsMounter::set_readpassphrase(
		[](std::string prompt) -> std::string { return "testpass"; });
	EncfsMounter mounter("/encrypted", "/mountpoint", "/etc/secrets/config");
	std::ofstream file("/mountpoint/testfile2");
	file << "test";
    return 0;
}
```

