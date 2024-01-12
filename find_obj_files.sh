#!/bin/sh

find src -iname '*.cpp' -type f | sed 's/\.cpp$/.o \\/' | sort
