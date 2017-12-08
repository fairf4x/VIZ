# VIZ
Graphical tool for modeling PDDL domains. 

## Compilation - Linux

### Prerequisities
It is possible to compile VIZ for linux platform with following libraries:

1. development kit for Qt 4 (development libraries and qmake - Makefile generator for Qt)
2. development libraries for the [igraph](http://igraph.org/c/) library

You will also need some C++ compiler.

Note:

When using Debian based distro, you can obtain all development libraries from repository.
e.g.:

```
$ apt-get install libigraph0-dev libqt4-dev qt4-qmake
```

### Compilation process
1. Suppose you cloned VIZ source from [GitHub](https://github.com/fairf4x/VIZ)

2. Next step is to create .pro file which is used by qmake for Makefile generation.
Run command:
```
$ qmake -project -o VIZ.pro
```
this will create file ``VIZ.pro``

3. Now you need to alter ``VIZ.pro`` file. The file should contain these lines at the begining:
```
TEMPLATE = app
TARGET = VIZ
DEPENDPATH += .
INCLUDEPATH += .
```
Following lines are to be added (alter the path to igraph liubraries according to your system):
```
QT += xml
INCLUDEPATH += . /YOUR/IGRAPH/SOURCES/include/
LIBS += -ligraph
```

4. Generation of Makefile with qmake is now possible:
```
$ qmake -makefile
```

5. Now just run make:
```
$ make
```

If everything is OK you will have compiled binary named VIZ.
In order to run it, shared library for igraph called ``libigraph.so`` should be available.
