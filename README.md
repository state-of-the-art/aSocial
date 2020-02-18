# aSocial

Opensource distributed social network application with focus on security, control and local store of social information.

* [Site page about it](https://www.state-of-the-art.io/projects/asocial/)

## About

In general purpose of the application is to replace all the centralized social networks & messangers and make sure the
users will never store their information in one place. aSocial was born from the idea of complete control over social
information, liberate from spam/ad, simplify connection between people and fight censoring. Storing the information
on the user's devices, using strong cryptography with integrated ability of plausible deniability, integration the
Bitcoin network will allow us to solve the issues we see every day today with our huge centralized social providers.

### History

The project was started as typical social 12/28/2013 and mutated to distributed and found itself 10/17/2014. The POC
was focused to UI, but some basic components was also prepared. Stopped due to breaking Qt changes, not ready Lightning
network (second layer of Bitcoin for microtransactions) & and overall project complexity was stopped 03/30/2016.

The lessions has been learned and rerun the aSocial development was started with SDD and searching for people who can
understand & help with thinking and development.

## Usage

Please check out the wiki page: [Wiki](https://github.com/state-of-the-art/asocial/wiki)

## Requirements

Not much, a regular application - it will support the major mobile & desktop operation systems, will be started with
Android/Linux.

## Application

The application itself is native divided into 2 parts - background backend and UI frontend. Backend working with
a simple key-value database, frontend working with encrypted sql database to process profiles data.

### Core features

Basic features of the social platform:

* *Modules* - add or remove components or use the other vendor component as plugin
* *History* - store not just dates of item, but also any edit of the items
* *Persons* - personal & legal entities, existing (with profiles) or imaginary
* *Events* - anything happend, happening, going to happen in the life
* *Messages* - monetized conversation threads or any other interaction like profile updates, news
* *Overlays* - your vision or additional information for existing or new entities, could be password protected
* *Synchronization* - make sure your devices replicates the information / profiles you have
* *Sharing* - ability to share your files with friends from any device you have

### Additional features

Based on core features aSocial support the embedded and external additional features:

* *Tree of life* - usual thing, your relatives and relations
* *Testament* - special message, will be triggered to send on some condition
* *Contracts* - special message, signed by 2 parties, could have penalties, money and end dates
* *Web-version* - access to web interface from web browser

### Plans

You can see all the feature requests/bugs on the github page:

* [Milestones](https://github.com/state-of-the-art/asocial/milestones)
* [Issues](https://github.com/state-of-the-art/asocial/issues)

## OpenSource

This is an experimental project - main goal is to test State Of The Art philosophy on practice.

We would like to see a number of independent developers working on the same project issues
for the real money (attached to the ticket) or just for fun. So let's see how this will work.

### License

Repository and it's content is covered by `Apache v2.0` - so anyone can use it without any concerns.

If you will have some time - it will be great to see your changes merged to the original repository -
but it's your choise, no pressure.

### Build

Build process is quite hard, but requires a minimum dependencies (cmake will get all the requirements
automatically).

#### Build in docker

##### For desktop

1. Clone the repository:
    ```
    host$ git clone https://github.com/state-of-the-art/asocial.git
    ```
2. Run the docker container:
    ```
    host$ docker run -it --rm --name project-build --volume="${PWD}/asocial:/home/user/project:ro" rabits/qt:5.14-desktop
    ```
3. Install the required dependencies (tclsh - sqlcipher build):
    ```
    docker$ sudo apt update ; sudo apt install -y libssl-dev clang-format-10 tclsh
    ```
4. Create build directory:
    ```
    docker$ mkdir build && cd build
    ```
5. Generate the build scripts
    ```
    docker$ cmake ../project -G Ninja "-DCMAKE_PREFIX_PATH:PATH=${QT_DESKTOP}"
    ```
6. Build the binaries:
    ```
    docker$ cmake --build .
    ```
7. You can find the compiled binaries in the current build directory

##### For android

1. Clone the repository:
    ```
    host$ git clone https://github.com/state-of-the-art/asocial.git
    ```
2. Run the android docker container:
    ```
    host$ docker run -it --rm --name project-build --volume="${PWD}/asocial:/home/user/project:ro" rabits/qt:5.14-android
    ```
3. Install the required dependencies (tclsh - sqlcipher build):
    ```
    docker$ sudo apt update ; sudo apt install -y libssl-dev clang-format-10 tclsh
    ```
4. Create build directory:
    ```
    docker$ mkdir build && cd build
    ```
5. Generate the build scripts
    ```
    docker$ cmake ../project -G Ninja "-DCMAKE_PREFIX_PATH:PATH=${QT_ANDROID}" "-DCMAKE_TOOLCHAIN_FILE:PATH=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake" "-DANDROID_ABI:STRING=${ANDROID_NDK_TOOLCHAIN_ABI}" -DANDROID_NATIVE_API_LEVEL:STRING=29
    ```
6. Build the binaries:
    ```
    docker$ cmake --build .
    ```
7. Debug APK will be created automatically with help of `tools/build-apk.sh` - and you will see where it is.

## Privacy policy

The whole purpose of this project - is to make sure your personal information will be controlled
by you, there is no way the aSocial (except for external plugins, please see their privacy policy)
will share your information without your permission with someone. But please be carefull - once
shared information usually will stay in the Internet forever, so think twice what are you doing.
