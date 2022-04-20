This guide might be different, dependents on what system it's running. Guide written for Ubuntu 20.

## Requirements

Drogon must be installed -> https://github.com/drogonframework/drogon/wiki/ENG-02-Installation<br>
That's it, Drogon will install everything that required for us

## Compiling

```
git clone https://github.com/Rynnya/aru
cd aru
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Running

After running code above binary will be compiled (if no error occurred) to run aru<br>
You also need to copy config.json from main folder to that binary and change values to yours
```
./aru
```

