# mitm by pentek.imre@gmail.com

## external code

- Qt
- cmake
- hunter: https://hunter.readthedocs.io/en/latest/index.html
- openssl (sorry, I didn't want to install dev packages, this is going to be compiled by hunter)
- apart from those, external code can be found by issuing grep -HRniF 'EXTERNAL CODE'

## compiling

the easiest method should be opening the CMakeLists.txt with qt creator
apart from that, this is a well-behaving cmake project, if you have qt dev libraries installed this might just work fine:

    mkdir build && cd build && cmake .. && make

## running

To get help you can issue

    ./mitm help

The app can be ran from any directory, not just from the same directory.

the rest should be self-explanatory.

If there is no CACERT present the app will create one on startup. Check the workDir if you want to create a new one for yourself.

Ways to test the app:

    https_proxy=127.0.0.1:8080 wget --ca-certificate=${workDir}/00cacert/cert -t 1 -O /dev/stdout

and

    echo bla|openssl s_client -proxy 127.0.0.1:8080 -connect ${host}:${port} -quiet 2>&1
