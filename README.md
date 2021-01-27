# fimi Server

**Check out the [documentation of fimi](https://creichel.github.io/fimi-Documentation/) for full information about the whole system.**

fimi is an application which tries to resemble a coach while you doing workout by analyzing your posture and giving you feedback about what you should watch more often. To run it, you need a **webcam**, a big screen and your smartphone (and some room space). You control the big screen with your smartphone by simply scanning the displayed QR code (being in the same wifi net is necessary).

fimi generates a 3d pose estimation based on the webcam image and checks every frame your posture based on a fixed set of rules per exercise. In the end, you receive a small summary of your results on your smartphone.

The application also compares your posture with your previous trainings while still being designed with privacy in mind. All your information is stored only temporarily while running the application on your PC or Mac, whereas the training data is stored on your smartphone directly. This also means that you could theoretically log in at your friend's house and have your profile with you.

This is the pose estimation server of the fimi fitness mirror application. It sends repeatedly pose estimation data that were recorded previously with the [fimi Server](https://github.com/creichel/fimi-Server).

## Used technology
- [XNECT C++ Library by Max Planck Institute](https://gvv.mpi-inf.mpg.de/projects/XNect/), revised and adapted
- [Caffe for Windows](https://github.com/happynear/caffe-windows)
  - [NVIDIA CUDA v9.2](https://developer.nvidia.com/cuda-92-download-archive?target_os=Windows&target_arch=x86_64&target_version=10&target_type=exenetwork)
  - [Python 2.7](https://www.python.org/ftp/python/2.7.18/python-2.7.18.amd64.msi)
  - [OpenCV 3.1](https://opencv.org)
  - [GFlags](https://github.com/gflags/gflags)
  - [HDF5](https://www.hdfgroup.org/solutions/hdf5/)
  - [LevelDB](https://github.com/google/leveldb)
  - [LMDB](http://www.lmdb.tech/doc/starting.html)
  - [Protobuf](https://github.com/protocolbuffers/protobuf)
  - [Snappy](https://github.com/google/snappy)
  - [GCC](http://gcc.gnu.org/git/gcc.git)
    - [GFortran](https://gcc.gnu.org/wiki/GFortran)
    - [LibQuadMath](https://gcc.gnu.org/onlinedocs/libquadmath/)
  - [ZLib](https://zlib.net)
  - [OpenBLAS](https://www.openblas.net)
- [WebSockeetPP](https://github.com/zaphoyd/websocketpp)

## Install it (on a machine with GeForece RTX-2080 TI)

### 1. Prerequisites
1. Python 2.7 ([x64](https://www.python.org/ftp/python/2.7.18/python-2.7.18.amd64.msi) version)
2. [Visual Studio 2015](https://my.visualstudio.com/Downloads?q=visual%20studio%202015&wt.mc_id=o~msft~vscom~older-downloads) with Visual C++, Python and Git Plugin
3. [NVIDIA CUDA 9.2](https://developer.nvidia.com/cuda-92-download-archive?target_os=Windows&target_arch=x86_64&target_version=10&target_type=exenetwork)
4. [CMake](https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-win64-x64.msi)
5. numpy x64 via `pip install numpy`

### 2. Build Caffe
This is based on https://tkcheng.wordpress.com/2019/04/11/caffe-on-windows-rtx-2080ti-cuda9-2-vs2015/

1. After you have installed all dependencies above, clone Caffe (from XNECT branch: https://github.com/creichel/caffe/tree/net/xnect).
2. Call
  ```bash
  .\scripts\build_win.cmd
  ```
  so Caffe downloads automatically the dependencies it needs.

Building Caffe will fail since we want to use a customized version of boost that supports our CUDA version and architecture.

For this, open `~\.caffe\dependencies\libraries_v140_x64_py35_1.1.0\libraries\include\boost-1_61\boost\config\compiler\nvcc.hpp` and comment out the last three lines:

```
//#if !defined(__CUDACC_VER__) || (__CUDACC_VER__ < 70500)
//#   define BOOST_NO_CXX11_VARIADIC_TEMPLATES
//#endif
```

### 3. Build XNECT
1. Add dependencies variables to `PATH`
```
C:\Users\<your-username>\.caffe\dependencies\libraries_v140_x64_py27_1.1.0\libraries\lib
C:\Users\<your-username>\.caffe\dependencies\libraries_v140_x64_py27_1.1.0\libraries\x64\vc14
```

2. Clone full git repository:
```bash
git clone --recurse-submodules -j8 https://github.com/creichel/fimi-Server.git
```
3. Build it by calling
```
cd fimi-Server\evaluator\extern\xnect ; mkdir build ; cd build ; cmake -G "Visual Studio 14 Win64" -C ~\.caffe\dependencies\libraries_v140_x64_py27_1.1.0\libraries\caffe-builder-config.cmake ..
```

## Start it
Open the generated `pose_estimator.exe` executive in the `\bin\Release` folder.

## Adjust it

### XNECT pose estimation
Check out the parameters in `\data\FullBodyTracker\XNECT` and adjust them to your needs.

### Modes
The server has three different modes which can be switched by changing the parameter `mode` in `\src\main.cpp` to one of the following options:
- **`Mode::LIVE`**: Capture the current webcam image and process the pose estimation on it
- **`Mode::VIDEOINPUT`**: Fetch the video file stored in the parameter `videoFilePath` and process this. It also creates an output video and saves it in the path of the executable with the name `YYYY-MM-DD-HH-MM-SS-pose_estimation_recording.avi`.
- **`Mode::SIMULATION_RECORDING`**: This is useful when you want to use mock data and the [fimi Mock Server](https://github.com/creichel/fimi-Mock-Server) for further development of the [fimi Client](https://github.com/creichel/fimi-Client). It fetches the video file stored in the parameter `videoFilePath` and processes this. It creates a `test.mock` file which contains per line: the delay in milliseconds as first value and the vector points as following.

### Other Settings
You can set furthermore the following settings:
- `PORT_NUMBER`: The port which will be opened for the WebSocket Server to send the pose estimation to it's listeners.
- `REPEAT_VIDEO`: Set this to `1` if you want to keep the video repeating while playing in `Mode::VIDEOINPUT` mode. `0` otherwise.
- `SHOW_WINDOW`: Set this to `0` to hide the window and gain some of the processing speed and power. `1` otherwise. Default is `1`.

## Troubleshooting issues
- **Caffe does not run with Python version >3.5**: Please note that caffe only supports Python 3.5 for now. If you try it first with any higher version. delete the `build` folder and restart the process.
- **Missing dependencies on executable start**: In some cases, you need to add and build further dependencies, like `zlib`. If you need and do so, don't forget to add the path to the package into `path` of your system environment variables.
