#!/bin/bash
set -ex
mkdir -p "$DESTDIR/usr/bin"
cp pedar "$DESTDIR/usr/bin/pedar"
cp io.p "$DESTDIR/usr/bin/io.p"
