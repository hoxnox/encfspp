# libfuse

Easy to use library, used to simplify mounting of
[encfs](https://github.com/vgough/encfs) directories from the code.

## Building

Use cmake to build the library.

### Building for Android

1. Install Android NDK according to your OS
   [android NDK](https://developer.android.com/ndk/downloads/index.html)

2. Prepare standalone toolchain for your Android:

   ```sh
   /opt/android-ndk/build/tools/make-standalone-toolchain.sh --arch=arm \
   	--ndk-dir=/opt/android-ndk --install-dir=/home/user/devel/android/toolchain \
   	--platform=android-21 --system=linux-x86_64
   ```

   set path accordingly

   ```sh
   export PATH=/home/user/devel/android/toolchain/bin:$PATH
   ```

3. Go to sources, configure and build:

   ```sh
   mkdir build && cd build
   cmake -DANDROID_TOOLCHAIN=/home/user/devel/android/toolchain ..
   make
   ```
### Building in standalone environment

The library targets Android with NDK platform 21, so we have to patch
[libfuse]() and [encfs]() source code to meet [bionic]() requirements.

You can build on a standalone server (without Internet connection).
Create vendor directory with the following structure (links to download
you can see in CMakeLists.txt file):

```
vendor
├── libfuse
│   └── libfuse
│       └── fuse-2.9.7.tar.gz
├── openssl
│   └── openssl
│       └── openssl-1.0.1t.tar.gz
└── vgough
    └── encfs
            └── encfs-1.9-rc1.tar.gz
```

Use `VENDOR_DIR` during configuration:

   ```sh
   cmake -DANDROID_TOOLCHAIN=/home/user/devel/android/toolchain -DVENDOR_DIR=/home/user/vendor
   ```

