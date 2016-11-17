# Building and Running LoWe components

As it was mentioned in the How It works document LoWe consists of two components:

1. LoWeAgent, an agent program that's running on Bash on Ubuntu on Windows, WSL
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
    loweagent [-e program] [-o file.log] [-l Debug|Info|Error] [-c] [-h] [program_mode]

Description:
    Listens and reacts to syscalls specified by the program_mode parameter.
    program_mode parameter refers to an item of loweagent.conf that specified which
    /dev-s are to be tracked and handled.
    If program_mode is not specified then it is inferred from the program name specifid by -e.

Options:
    -e, -exec       Executes the program specified as argument. If it contains a whitespace
                    (eg. arg specified) then it must be quoted like this: "mplayer video.mkv"
                    If -e option is not specified then the program must be started in a
                    separate bash window
    -o, -outfile    Send log messages to this file instead of stdout
    -l, -loglevel   Set Debug, Info or Error loglevel. Default: Info
    -c, -catchall   Catch all syscalls that are not handled otherwise and log events
    -h, -help       Display this help
```

### Startup modes and options

LoWeAgent can start supervising an application in two ways:

1. It attaches to an application that has just started
2. Starts the application itself

In the first mode the program_mode is a mandatory parameter. Currently mplayer and x program modes are supported. It ensures that the required devices are "emulated" by LoWe however it doesn't actually start the program itself. loweagent.conf lists what programs are being chased as newly started processes by LoWe so that it can attach to them as quick as possible even before the first device access made that needs to be tricked.

In the second mode the -e parameter is specified. In this case the program (eg. -e X) is started by LoWeAgent that ensures that from the very first step all operations can be traced and tricked. If the program is parameterized then the parameter must be quoted (eg. -e "mplayer my_favourite_vide.mkv"). When -e is specified the program_mode parameter is not mandatory since the program_mode will be automatically deduced from the program name based on loweagent.conf.

### Other options

By default LoWeAgent writes log messages to stdout, those can be forwarded to a file via the -o option. Debug, Info (default), Error loglevels are supported via the -l option. 

The -c option instructs LoWeAgent to catch all relevant syscalls that are not actually handled by LoWe. This is for logging purposes only.

# LoWeExposer

LoWeExposer in a Windows WPF executable written in C# targetting .NET Framework 4.6+.

## Building LoWeExposer

Having the source code already downloaded, LoWeExposer can be build via Visual Studio 2015 by loading the ```LoWeExposer.sln``` or with the ```build.cmd``` script from a cmd where msbuild is in the PATH (eg. Developer Command Prompt).

After running build.cmd the ```out``` directory will contain the required executable and dll files.

## Running LoWeExposer

LoWeExposer.exe can be started from the out directory created by the build. Port no. 12345 of 127.0.0.1 is reserved by LoWeExposer. If this port is already allocated by an other program LoWeExposer will fail.

