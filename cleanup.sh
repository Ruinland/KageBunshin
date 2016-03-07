#!/bin/sh
sudo umount $1/{proc,run,dev/pts,dev,tmp,sys}
sudo umount $1
