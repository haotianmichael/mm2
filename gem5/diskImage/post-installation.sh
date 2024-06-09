#!/bin/bash

# SPDX-License-Identifier: BSD 3-Clause

echo 'Post Installation Started'


mv /home/gem5/m5 /sbin
ln -s /sbin/m5 /sbin/gem5

# copy and run outside (host) script after booting
cat /home/gem5/runscript.sh >> /root/.bashrc

echo 'Post Installation Done'
