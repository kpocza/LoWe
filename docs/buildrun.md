# Building and Running LoWe components

As it was mentioned in the How It works document LoWe consists of two components:

1. LoWeAgent, an agent program that's running on Bash on Ubuntu on Windows, lxss
2. LoWeExposer, a Windows application that provides graphics, mouse, X keyboard and sound support

# LoWeAgent

LoWeAgent is an ELF executable written in  C++.

## Building LoWeAgent

On top of the default packages installed on Ubuntu on Windows g++, make and libconfig-dev will be needed.

So execute the following commands:

```
sudo apt install g++
sudo apt install make
sudo apt install libconfig-dev
```

Get the source code from ```https://github.com/kpocza/LoWe``` with your preferred method like 

```git clone https://github.com/kpocza/LoWe.git``` .

Execute

```
make
```

in the LoWe/src/LoWeAgent folder in  Bash prompt.

The resulting executable with the respective config file will be put in the ```out``` directory.

## Running LoWeAgent

To run LoWeAgent libconfig package needs to be installed on top of the defaults (libconfig-dev already installs libconfig9).

```sudo apt install libconfig9```

The loweagent application can be run then from the out directory.

Command line options are as follows:

```
$ ./loweagent
Usage:
    loweagent [-l Debug|Info|Error] [-o file.log] [-c] [-h] program_mode

Description:
    Listens and reacts to syscalls specified by the program_mode parameter.
    program_mode parameter refers to an item of loweagent.conf that specified which
    /dev-s are to be tracked and handled.
    
Options:
    -l, -loglevel:    Set Debug, Info or Error loglevel. Default: Info
    -o, -outfile:     Send log messages to this file instread of the stdout
    -c, -catchall:    Catch all syscalls that are not handled otherwise and provide log information
    -h, -help:        Display this help
```

LoWeAgent requires the program_mode parameter to be specified. Currently mplayer and x program modes are supported. It ensures that the required devices are "emulated" by LoWe and doesn't actually start the program itself. Please refer to loweagent.conf for more details.

By default LoWeAgent writes log messages to stdout, those can be forwarded to a file via the -o option. Debug, Info, Error loglevels are supported (Info is the default). 

The -c option instructs LoWeAgent to catch all relevant syscalls that are not actually handled by LoWe. This is for logging purposes only.

# LoWeExposer

LoWeExposer in a Windows WPF executable written in C# targetting .NET Framework 4.6+.

## Building LoWeExposer

Having the source code already downloaded, LoWeExposer can be build via Visual Studio 2015 by loading the ```LoWeExposer.sln``` or with the ```build.cmd``` script from a cmd where msbuild is in the PATH (eg. Developer Command Prompt).

After running build.cmd the ```out``` directory will contain the required executable and dll files.

## Running LoWeExposer

LoWeExposer.exe can be started from the out directory created by the build. Port no. 12345 of 127.0.0.1 is reserved by LoWeExposer. If this port is already allocated by an other program LoWeExposer will fail.

