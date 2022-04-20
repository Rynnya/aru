Running aru on windows isn't good idea, as installing database connector is kinda trouble

## Requirements

* Visual Studio 2017
* vcpkg

## Preparing

You must clone into folder where you want to vcpkg be installed

```
git clone https://github.com/Microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
vcpkg integrate install
```

If last command failes - try to run with evaluation

## Compiling

```
git clone https://github.com/Rynnya/aru
cd aru
.\windows_setup_build.bat
```

After that, in build folder, you will found SLN file (if everything was installed properly)<br>
You might compile code from Visual Studio, or by running code below
```
cd build
cmake --build . --config Release
```

## Running

After running code above binary will be compiled (if no error occurred) to run aru<br>
You also need to copy config.json from main folder to that binary and change values to yours
```
cd Release
.\aru.exe
```
