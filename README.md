# VIZ
Graphical tool for modeling PDDL domains. This work is originally a student project. Back in 2010 it was presented at KEPS conference ([pdf](https://github.com/fairf4x/VIZ/blob/master/doc/KEPS_2010.pdf)).
Technical documentation is available at [wiki](https://github.com/fairf4x/VIZ/wiki/Technical-documentation).

If you find it useful please cite:
```
@inproceedings{KEPS:2010,
  title={Visual design of planning domains},
  author={J. Vodr\'{a}\v{z}ka and L. Chrpa},
  booktitle={KEPS 2010: Workshop on Knowledge Engineering for Planning and Scheduling},
  year={2010}
}
```

## Compilation - Linux

### Prerequisities
It is possible to compile VIZ for linux platform with following libraries:

1. development kit for Qt 5 (development libraries and qmake - Makefile generator for Qt)
2. development libraries for the [igraph](http://igraph.org/c/) library

You will also need some C++ compiler.

Note:

When using Debian based distro, you can obtain all development libraries from repository.
e.g.:

```
$ apt-get install libigraph0-dev qtbase5-dev qt5-qmake
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
DEPENDPATH += . forms include res src
INCLUDEPATH += . include
```
Following lines are to be added (alter the path to igraph liubraries according to your system):
```
QT += xml widgets
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
