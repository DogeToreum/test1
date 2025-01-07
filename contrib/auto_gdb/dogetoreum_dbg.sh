#!/bin/bash
# use testnet settings,  if you need mainnet,  use ~/.dogetoreumcore/dogetoreumd.pid file instead
dogetoreum_pid=$(<~/.dogetoreumcore/testnet3/dogetoreumd.pid)
sudo gdb -batch -ex "source debug.gdb" dogetoreumd ${dogetoreum_pid}
