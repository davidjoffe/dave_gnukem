#!/bin/sh
# dj2022 small convenience helper script to get or update data subfolder (you need git installed for this)
#
# NB this gets latest cloned git repo with ".git" folder - if you're distributing a RELEASE you may want to delete the ".git" subfolders
#
# If you want to optionally use a different data folder name to the default djDATADIR)
#

djDATADIR="data"
djDATA_URL="https://github.com/davidjoffe/gnukem_data"
djDATADIR2="datasrc"
djDATA_URL2="https://github.com/davidjoffe/gnukem_datasrc"


echo 
echo "This is a small convenience helper script to get or update data subfolder. You need git for this."
echo "The data subfolder is necessary for the game to run."
echo "The datasrc subfolder is NOT necessary for the game to run but is intended only if you want to do tasks like edit sprites."
echo "NB this gets latest cloned git repo with .git folder. If you're distributing a RELEASE you may want to delete the .git subfolders."
echo 
echo "This script will do:"
echo git clone "${djDATA_URL}" "${djDATADIR}"
echo git clone "${djDATA_URL2}" "${djDATADIR2}"
echo "If the subfolders exist it will update with git pull."
echo 
echo "(If you want to use a different data folder name you can but then you may need to use the -datadir commandline option for the game or adapt the Makefile)"
echo 

read -p "ARE YOU SURE YOU WANT TO PROCEED? (y/n) " keyread
case $keyread in 
	y ) echo ok, we will proceed;;
	n ) echo exiting...;
		exit;;
	* ) echo invalid response;
		exit 1;;
esac


echo DATA FOLDER "${djDATADIR}"
if [ -d "$djDATADIR" ]; then
	echo Updating data folder "${djDATADIR}" ...
	echo cd "${djDATADIR}"
	cd "${djDATADIR}"
	# show current folder
	pwd
	echo git pull
	git pull
	echo cd ..
	cd ..
else
	echo Cloning data folder ...
	git clone "${djDATA_URL}" "${djDATADIR}"
fi


echo 
echo DATASRC FOLDER "${djDATADIR2}" 
echo "Also getting datasrc but these are NOT necessary for game to run (datasrc folder are for those who might want to do things like edit sprites)"

if [ -d "$djDATADIR2" ]; then
	echo Updating datasrc folder "${djDATADIR2}" ...
	echo cd "${djDATADIR2}"
	cd "${djDATADIR2}"
	# show current folder
	pwd
	echo git pull
	git pull
	echo cd ..
	cd ..
else
	echo Cloning datasrc folder ...
	git clone "${djDATA_URL2}" "${djDATADIR2}"
fi
