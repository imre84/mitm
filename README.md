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

you can start the app without any parameters, in this case the app will start with the settings saved previously. In the apsence of previous settings the default listen address is 127.0.0.1:8080

you can change the listen address by starting the executable like this:

    ./mitm 0.0.0.0 3128

in which case the new settings are applied and saved into the settings.

If there is no CACERT present the app will create one on startup. check the CA directory if you want to create a new one for yourself.
