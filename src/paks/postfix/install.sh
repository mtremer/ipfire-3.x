#!/bin/bash
. /opt/pakfire/lib/functions.sh

extract_files

# Set postfix's hostname
postconf -e "myhostname=$(hostname -f)"
