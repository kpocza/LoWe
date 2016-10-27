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

in the LoWe/src/LoWeAgent folder in  Bash on Ubuntu on Windows prompt.

The resulting executable with the respective config file will present in the ```out``` directory.

## Running LoWeAgent

To run LoWeAgent libconfig package needs to be installed on top of the defaults.

```sudo apt install libconfig9```

The loweagent application can be run then from the out directory.

# LoWeExposer

LoWeExposer in a Windows WPF executable written in C# targetting .NET Framework 4.6+.

## Building LoWeExposer

Having the source code already downloaded, LoWeExposer can be build via Visual Studio 2015 by loading the ```LoWeExposer.sln``` or with the ```build.cmd``` script.

After running build.cmd the ```out``` directory will contain the required executable and dll files.

## Running LoWeExposer

LoWeExposer.exe can be started from the out directory created by the build.
