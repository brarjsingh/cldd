cldd
====

Control Logging and Data Acquisition Daemon

Meant to function as a highly configurable system for interacting with physical
sensors and actuators. The CLD library (currently at gitorious.org/cld/libcld)
is responsible for a lot of the actual hardware interfacing, data logging, and
feedback control using a simple PID algorithm.

Building
--------

This uses ZeroMQ for which the vapi needs to be installed manually.

```
git clone https://github.com/geoffjay/zmq-vapi
VER=`valac --version | sed 's/^.*\([0-9]\.[0-9][0-9]\).*$/\1/'`
sudo cp zmq-vapi/libzmq.{deps,vapi} /usr/share/vala-$VER/vapi/
```
