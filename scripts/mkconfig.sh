#!/bin/bash

USUM=$(tr -dc A-Za-z0-9 </dev/urandom | head -c 200 | sha256sum | cut -d "-" -f1 | tr -d ' ')
SIGNAL=$RANDOM

cat << EOF
#define USUM "$USUM"
#define SIGNAL $SIGNAL 
#define DEBUG false
EOF
