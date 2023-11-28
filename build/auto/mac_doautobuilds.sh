#!/bin/bash
#dj2023 new helper for attempting to increasingly automate builds
# This is also to help automate build testing e.g. to help catch regressions

echo ----------------------------------------------------
pwd
#echo $djDIR

echo ---------- git pull --all
git pull --all
echo ---------- git info
git rev-list --count HEAD
git log -1
git status
echo ----------

echo Clean
make clean
echo Build
make -j8

echo ----------------------------------------------------

