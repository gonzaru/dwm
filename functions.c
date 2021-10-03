/*
 * by Gonzaru
 * Distributed under the terms of the GNU General Public License v3
 */

/* My customized functions to add dwm functionality without patching dwm.c */

/* global macros */
#define FIRST_TAG  1 << 0
#define LAST_TAG   1 << (LENGTH(tags) - 1)
#define SCRATCH_TAG 1 << 7

/* default temporary directory path */
#define DEFAULT_TMPDIR "/tmp"

/* function declarations */
void debug(const char *errstr, ...);
void fakepresskey(Window win, char *typemask, char *strkey);
void focusclient(const Arg *arg);
void focuslast(const Arg *arg);
void focusmaster(const Arg *arg);
void focusmonmaster(const Arg *arg);
char *getwindowclass(Window w);
char *getwindowname(Window w);
int ismaster(Client *c);
void nexttag(const Arg *arg);
void organize(const Arg *arg);
void pressmousecursor(const Arg *arg);
void prevtag(const Arg *arg);
void putcurmaster(void);
void putcurtag(void);
void putcurwin(void);
void putfilemfact(const Arg *arg);
void putkeeptags(void);
void reloadbrowser(const Arg *arg);
void reloaddwm(const Arg *arg);
void savekeeptags(const Arg *arg);
void scratchpadmon(const Arg *arg);
void setasmaster(Client *c);
int setwindownameclass(Window w, char *sn, char *sc);
void showurgent(const Arg *arg);
void spawnsh(const char *cmd);
void toggleborder(const Arg *arg);
void togglefullscr(const Arg *arg);
void togglemousecursor(const Arg *arg);
void writecurmaster(void);
void writecurtag(void);
void writecurwin(void);
void writemfact(void);
void zoomfirst(const Arg *arg);
void zoomlast(const Arg *arg);
void zoommon(const Arg *arg);

/* debug info */
void debug(const char *errstr, ...)
{
  va_list ap;

  va_start(ap, errstr);
  vfprintf(stderr, errstr, ap);
  va_end(ap);
}

void fakepresskey(Window win, char *typemask, char *strkey)
{
  XEvent ev;

  memset(&ev, 0x00, sizeof(ev));
  ev.xkey.display = dpy;
  ev.xkey.window = win;
  ev.xkey.subwindow = None;

  if (strcmp(typemask, "ControlMask") == 0) {
    ev.xkey.state = ControlMask;
  } else if (strcmp(typemask, "ShiftMask") == 0) {
    ev.xkey.state = ShiftMask;
  } else if (strcmp(typemask, "Mod1Mask") == 0) {
    ev.xkey.state = Mod1Mask;
  } else if (strcmp(typemask, "Mod4Mask") == 0) {
    ev.xkey.state = Mod4Mask;
  }

  ev.xkey.keycode = XKeysymToKeycode(dpy, XStringToKeysym(strkey));
  ev.xkey.same_screen = True;

  /* press */
  ev.xkey.type = KeyPress;
  XSendEvent(dpy, win, True, KeyPressMask, &ev);
  XFlush(dpy);
  /* 100ms don't ignore XSendEvent() */
  usleep(100000);

  /* release */
  ev.xkey.type = KeyRelease;
  XSendEvent(dpy, win, True, KeyReleaseMask, &ev);
  XFlush(dpy);
  /* 100ms don't ignore XSendEvent() */
  usleep(100000);
}

void focusclient(const Arg *arg)
{
  Arg a;
  Monitor *m;
  Client *c;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (strcmp(getwindowclass(c->win), arg->v) == 0) {
        if (m != selmon) {
          unfocus(selmon->sel, True);
          selmon = c->mon;
        }
        a.ui = c->tags;
        view(&a);
        focus(c);
        arrange(c->mon);
        return;
      }
    }
  }
}

/* focus last client */
void focuslast(const Arg *arg)
{
  Client *c;
  Client *cl;

  if (!selmon->sel || selmon->sel->isfloating) {
    return;
  }

  cl = selmon->sel;
  for (c = selmon->clients; c; c = c->next) {
    if (ISVISIBLE(c) && !c->isfloating) {
      cl = c;
    }
  }
  if (cl) {
    focus(cl);
  }
}

/* focus master */
void focusmaster(const Arg *arg)
{
  Client *c;

  if (!selmon->sel || !selmon->nmaster) {
    return;
  }

  c = nexttiled(selmon->clients);
  if (c) {
    focus(c);
  }
}

void focusmonmaster(const Arg *arg)
{
  focusmon(arg);
  focusmaster(arg);
}

/* get client window class */
char *getwindowclass(Window w)
{
  XClassHint ch = { NULL, NULL };

  XGetClassHint(dpy, w, &ch);
  return ch.res_class;
}

/* get client window name */
char *getwindowname(Window w)
{
  XClassHint ch = { NULL, NULL };

  XGetClassHint(dpy, w, &ch);
  return ch.res_name;
}

/* check if client is master */
int ismaster(Client *c)
{
  Client *cm;

  if (!c || !c->mon->nmaster || c->isfloating) {
    return 0;
  }

  cm = nexttiled(c->mon->clients);
  return (cm == c) ? 1 : 0;
}

/* next tag */
void nexttag(const Arg *arg)
{
  Arg a;
  unsigned int curtag;
  unsigned int nextag;

  curtag = selmon->tagset[selmon->seltags];
  nextag = curtag << 1;
  if (nextag > LAST_TAG) {
    nextag = FIRST_TAG;
  }
  a.ui = nextag;
  view(&a);
}

void organize(const Arg *arg)
{
  FILE *file = NULL;
  char filepath[256];
  char *user = getenv("USER");
  char *tmpdir;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-organize.lock", tmpdir, user);
  if ((file = fopen(filepath, "r"))) {
    fclose(file);
    spawnsh("echo 'dwm organize already executed!' | dmenu");
    return;
  }

  /* organize tags after reload */
  putkeeptags();

  /* set fmfact for all monitors */
  putfilemfact(arg);

  /* go to latest monitor current tag */
  putcurtag();

  /* set as master to latest current master window */
  putcurmaster();

  /* focus to latest current window */
  putcurwin();

  sprintf(filepath, "%s/%s-dwm-organize.lock", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  fclose(file);
}

void pressmousecursor(const Arg *arg)
{
  XEvent event;
  int button_n = 0;

  /*
  button 1 = left
  button 2 = middle
  button 3 = right
  button 4 = scroll up
  button 5 = scroll down
  button 6 = scroll left
  button 7 = scroll right
  */

  if (strcmp(arg->v, "leftclick") == 0) {
    button_n = 1;
  } else if ((strcmp(arg->v, "middleclick") == 0)) {
    button_n = 2;
  } else if ((strcmp(arg->v, "rightclick") == 0)) {
    button_n = 3;
  } else if ((strcmp(arg->v, "scrollupclick") == 0)) {
    button_n = 4;
  } else if ((strcmp(arg->v, "scrolldownclick") == 0)) {
    button_n = 5;
  } else if ((strcmp(arg->v, "scrollleftclick") == 0)) {
    button_n = 6;
  } else if ((strcmp(arg->v, "scrollrightclick") == 0)) {
    button_n = 7;
  }

  memset(&event, 0x00, sizeof(event));
  event.xbutton.button = button_n;
  event.xbutton.same_screen = True;
  event.xbutton.subwindow = root;

  while (event.xbutton.subwindow) {
    event.xbutton.window = event.xbutton.subwindow;
    XQueryPointer(dpy, event.xbutton.window,
      &event.xbutton.root, &event.xbutton.subwindow,
      &event.xbutton.x_root, &event.xbutton.y_root,
      &event.xbutton.x, &event.xbutton.y,
      &event.xbutton.state);
  }

  /* press */
  event.type = ButtonPress;
  XSendEvent(dpy, PointerWindow, True, ButtonPressMask, &event);
  XFlush(dpy);

  if (button_n == 1 || button_n == 2 || button_n == 3) {
    usleep(100000);
  } else if (button_n == 4 || button_n == 5 || button_n == 6 || button_n == 7) {
    usleep(1);
  }

  /* release */
  event.type = ButtonRelease;
  event.xbutton.state = 0x100;
  XSendEvent(dpy, PointerWindow, True, ButtonReleaseMask, &event);
  XFlush(dpy);
}

/* previous tag */
void prevtag(const Arg *arg)
{
  Arg a;
  unsigned int curtag;
  unsigned int prevtag;

  curtag = selmon->tagset[selmon->seltags];
  prevtag = curtag >> 1;
  if (prevtag < FIRST_TAG) {
    prevtag = LAST_TAG;
  }
  a.ui = prevtag;
  view(&a);
}

void putcurmaster(void)
{
  FILE *file;
  char filepath[256];
  char line[256];
  char *user = getenv("USER");
  char *tmpdir;
  Monitor *m;
  Client *c;
  int fmon;
  long unsigned int fwin;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curmaster.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  while (!feof(file)) {
    if (fgets(line, sizeof line - 1, file)) {
      sscanf(line, "%d %lud", &fmon, &fwin);
      for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
          if (c->win == fwin) {
            if (c->mon != selmon) {
              unfocus(selmon->sel, True);
              selmon = c->mon;
              focus(NULL);
            }
            setasmaster(c);
            arrange(c->mon);
            break;
          }
        }
      }
    }
  }
  fclose(file);
}

void putcurtag(void)
{
  FILE *file;
  char filepath[256];
  char line[256];
  char *user = getenv("USER");
  char *tmpdir;
  Monitor *m;
  int fmon;
  int ftag;
  Arg a;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curtag.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  while (!feof(file)) {
    if (fgets(line, sizeof line - 1, file)) {
      sscanf(line, "%d %d", &fmon, &ftag);
      for (m = mons; m; m = m->next) {
        if (m->num == fmon) {
          if (m != selmon) {
            unfocus(selmon->sel, True);
            selmon = m;
            focus(NULL);
          }
          if (m->tagset[m->seltags] != ftag) {
            m->tagset[m->seltags] = ftag;
          }
          a.ui = ftag;
          view(&a);
          arrange(m);
        }
      }
    }
  }
  fclose(file);
}

void putcurwin(void)
{
  FILE *file;
  char filepath[256];
  char line[256];
  char *user = getenv("USER");
  char *tmpdir;
  Arg a;
  Monitor *m;
  Client *c;
  int fmon;
  long unsigned int fwin;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curwin.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  if (fgets(line, sizeof line - 1, file)) {
    sscanf(line, "%d %lud", &fmon, &fwin);
    for (m = mons; m; m = m->next) {
      for (c = m->clients; c; c = c->next) {
        if (c->win == fwin) {
          if (c->mon != selmon) {
            unfocus(selmon->sel, True);
            selmon = c->mon;
            focus(NULL);
          }
          if (!ISVISIBLE(c)) {
            a.ui = c->tags;
            view(&a);
          }
          focus(c);
          arrange(c->mon);
          fclose(file);
          return;
        }
      }
    }
  }
  fclose(file);
}

void putfilemfact(const Arg *arg)
{
  Monitor *m;
  FILE *file;
  char filepath[256];
  char line[256];
  char *user = getenv("USER");
  char *tmpdir;
  int fmon;
  float fmfact;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curmfact.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  if (fgets(line, sizeof line - 1, file)) {
    sscanf(line, "%d %f", &fmon, &fmfact);
    for (m = mons; m; m = m->next) {
      if (m->num == fmon) {
        if (m->mfact != fmfact) {
          m->mfact = fmfact;
          arrange(m);
        }
      }
    }
  }
  fclose(file);
}

void putkeeptags(void)
{
  Arg a;
  Monitor *m;
  Client *c;
  FILE *file;
  char filepath[256];
  char line[256];
  char *user = getenv("USER");
  char *tmpdir;
  int fmon;
  int ftag;
  int fmaster;
  int fsel;
  int ffloating;
  long unsigned int fwin;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-state-tags.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  while (!feof(file)) {
    if (fgets(line, sizeof line - 1, file)) {
      sscanf(line, "%d %d %d %d %d %lud", &fmon, &ftag, &fmaster, &fsel, &ffloating, &fwin);
      for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
          if (c->win == fwin) {
            if (c->mon->num != fmon) {
              a.i = c->mon->num;
              sendmon(c, dirtomon(a.i));
            }
            if (c->tags != ftag) {
              c->tags = ftag;
            }
            if (ffloating && c->isfloating != ffloating) {
              c->isfloating = ffloating;
            }
            if (fsel) {
              focus(c);
            }
            arrange(c->mon);
          }
        }
      }
    }
  }
  fclose(file);
  arrange(selmon);
}

void reloadbrowser(const Arg *arg)
{
  Monitor *m;
  Client *cc, *c;

  Window w_ret;
  int current_x, current_y;
  int win_x, win_y;
  int browser_x, browser_y;
  unsigned int mask_ret;

  /* get current x, y mouse pointer */
  if (!XQueryPointer (dpy, root, &w_ret, &w_ret, &current_x, &current_y, &win_x, &win_y, &mask_ret)) {
    return;
  }

  cc = selmon->sel;
  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (strcmp(getwindowclass(c->win), arg->v) == 0 || (strcmp(defbrowser, arg->v) == 0)) {
        focus(c);
        browser_x = c->x + (c->w / 2);
        browser_y = c->y + (c->h / 2);
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, browser_x, browser_y);
        fakepresskey(c->win, "ControlMask", "r");
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, current_x, current_y);
        focus(cc);
        return;
      }
    }
  }
}

void reloaddwm(const Arg *arg)
{
  writemfact();
  writecurtag();
  writecurmaster();
  writecurwin();
  savekeeptags(arg);
  spawnsh("${HOME}/bin/wmreload");
}

void savekeeptags(const Arg *arg)
{
  Monitor *m;
  Client *c;
  FILE *file = NULL;
  char filepath[256];
  char *user = getenv("USER");
  char *tmpdir;

  writemfact();
  writecurtag();
  writecurmaster();
  writecurwin();

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-organize.lock", tmpdir, user);
  remove(filepath);

  sprintf(filepath, "%s/%s-dwm-state-tags.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      fprintf(file, "%d %d %d %d %d %lud %s %s\n", c->mon->num,
        c->tags, (ismaster(c) == 1) ? 1 : 0, (c == selmon->sel) ? 1 : 0,
        (c->isfloating) ? 1 : 0, c->win, getwindowclass(c->win), c->name);
    }
  }
  fclose(file);
}

void scratchpadmon(const Arg *arg)
{
  Monitor *m;
  Client *c;
  const char *scratchcmd = "xterm -u8 -class 'Scratchpad'";
  int match = 0;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (strcmp("Scratchpad", getwindowclass(c->win)) == 0) {
        match = 1;
        if (m != selmon) {
          sendmon(c, selmon);
          arrange(c->mon);
          setasmaster(c);
        } else {
          if (!ISVISIBLE(c)) {
            c->tags = selmon->tagset[selmon->seltags];
            arrange(c->mon);
            setasmaster(c);
          } else {
            if (c->tags != SCRATCH_TAG && c == selmon->sel) {
              c->tags = SCRATCH_TAG;
              arrange(c->mon);
              focus(NULL);
            } else {
              setasmaster(c);
            }
          }
        }
        return;
      }
    }
  }
  /* launch it if not present */
  if (!match) {
    spawnsh(scratchcmd);
  }
}

/* put client in master area */
void setasmaster(Client *c)
{
  Client *cc;
  int nc = 0;

  if (!c || c->isfloating) {
    return;
  }

  /* check if has more than 1 client */
  for (cc = selmon->clients; cc && nc < 2; cc = cc->next) {
      if (ISVISIBLE(cc) && !cc->isfloating) {
        nc++;
      }
  }
  if (nc >= 2) {
    pop(c);
  }
}

/* set client windowclass */
int setwindownameclass(Window w, char *sn, char *sc)
{
  XClassHint ch = { NULL, NULL };

  ch.res_name = sn;
  ch.res_class = sc;
  return XSetClassHint(dpy, w, &ch);
}

/* http://permalink.gmane.org/gmane.comp.misc.suckless/8087 */
void showurgent(const Arg *arg)
{
  Monitor *m;
  Client *c;
  Arg a;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (c->isurgent) {
        if (m != selmon) {
          unfocus(selmon->sel, True);
          selmon = c->mon;
          focus(NULL);
        }
        a.ui = c->tags;
        view(&a);
        focus(c);
        arrange(c->mon);
        return;
      }
    }
  }
}

/* exec shell command */
void spawnsh(const char *cmd)
{
  const char *shcmd[] = { "/bin/sh", "-c", cmd, NULL };
  Arg a = {.v = shcmd };

  spawn(&a);
}

void toggleborder(const Arg *arg)
{
  Client *c = selmon->sel;
  XWindowChanges wc;

  if (!c) {
    return;
  }

  if (c->bw) {
    c->oldbw = c->bw;
    c->bw = 0;
  } else {
    c->bw = c->oldbw;
  }
  wc.border_width = c->bw;
  XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
}

void togglefullscr(const Arg *arg)
{
  if (!selmon->sel) {
    return;
  }

  setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}

void togglemousecursor(const Arg *arg)
{
  Window w_ret;
  int root_x, root_y;
  int win_x, win_y;
  unsigned int mask_ret;
  /* pointer position to "hide" */
  int pointer_x = selmon->mx + 5;
  int pointer_y = 0;
  /* position */
  static int root_x_pos = 0;
  static int root_y_pos = 0;

  /* only if bar is on */
  if (!selmon->showbar) {
    return;
  }

  /* get current x, y mouse pointer */
  if (!XQueryPointer (dpy, root, &w_ret, &w_ret, &root_x, &root_y, &win_x, &win_y, &mask_ret)) {
    return;
  }

  /* toggle mouse pointer position */
  if (root_y == 0 && root_x == pointer_x) {
    XWarpPointer(dpy, None, root, 0, 0, 0, 0, root_x_pos, root_y_pos);
  } else {
    root_x_pos = root_x;
    root_y_pos = root_y;
    XWarpPointer(dpy, None, root, 0, 0, 0, 0, pointer_x, pointer_y);
  }
}

void writecurmaster(void)
{
  FILE *file;
  Monitor *m;
  Client *c;
  char filepath[256];
  char *user = getenv("USER");
  char *tmpdir;

  if (!selmon->sel || selmon->sel->isfloating) {
    return;
  }

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curmaster.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  /* get the first master on each monitor */
  for (m = mons; m; m = m->next) {
    c = nexttiled(m->clients);
    if (c) {
      fprintf(file, "%d %lud\n", c->mon->num, c->win);
    }
  }
  fclose(file);
}

void writecurtag(void)
{
  FILE *file;
  Monitor *m;
  char filepath[256];
  char *user = getenv("USER");
  char *tmpdir;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curtag.txt", tmpdir, user);
  file = fopen(filepath, "w+");
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    fprintf(file, "%d %d\n", m->num, m->tagset[m->seltags]);
  }
  fclose(file);
}

void writecurwin(void)
{
  FILE *file;
  char filepath[256];
  char *user = getenv("USER");
  char *tmpdir;

  if (!selmon->sel) {
    return;
  }

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curwin.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  fprintf(file, "%d %lud\n", selmon->num, selmon->sel->win);
  fclose(file);
}

void writemfact(void)
{
  FILE *file;
  char filepath[256];
  char *user = getenv("USER");
  char *tmpdir;
  Monitor *m;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  sprintf(filepath, "%s/%s-dwm-curmfact.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    fprintf(file, "%d %.2f\n", m->num, m->mfact);
  }
  fclose(file);
}

void zoomfirst(const Arg *arg)
{
  Client *c;

  if (!selmon->sel || !ismaster(selmon->sel) || selmon->sel->isfloating) {
    return;
  }

  for (c = selmon->sel; c->next; c = c->next) {
    ;
  }
  detach(selmon->sel);
  selmon->sel->next = NULL;
  c->next = selmon->sel;
  focusmaster(arg);
  arrange(selmon);
}

void zoomlast(const Arg *arg)
{
  Client *c;
  Client *cl;

  if (!selmon->sel || !ismaster(selmon->sel) || selmon->sel->isfloating) {
    return;
  }

  cl = selmon->sel;
  for (c = selmon->clients; c; c = c->next) {
    if (ISVISIBLE(c) && !c->isfloating) {
      cl = c;
    }
  }
  if (cl) {
    pop(cl);
  }
}

/* zoom/cycles monitors */
void zoommon(const Arg *arg)
{
  Monitor *m;
  Monitor *m1 = NULL;
  Monitor *m2 = NULL;
  Client *c;
  int ac[256] = {0};
  int i = 0;
  unsigned int pctags;
  unsigned int m1_tagset;
  unsigned int m2_tagset;

  /* no next monitor */
  if (!mons->next) {
    return;
  }

  m = mons;
  m1 = m;
  m2 = m->next;

  if (!m1 || !m2) {
    return;
  }

  /* get current tagset */
  m1_tagset = m1->tagset[m1->seltags];
  m2_tagset = m2->tagset[m2->seltags];
  for (i = 0, m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next, i++) {
      ac[i] = c->win;
    }
  }
  for (i = (sizeof(ac) / sizeof(int)) - 1; i >= 0; i--) {
    if (ac[i]) {
      for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
          if (c->win == ac[i]) {
            pctags = c->tags;
            if (c->mon == m1) {
              sendmon(c, m2);
            } else if (c->mon == m2) {
              sendmon(c, m1);
            }
            ac[i] = '\0';
            c->tags = pctags;
            arrange(c->mon);
          }
        }
      }
    }
  }
  /* swap previous tagset */
  m1->tagset[m1->seltags] = m2_tagset;
  m2->tagset[m2->seltags] = m1_tagset;
  arrange(m1);
  arrange(m2);
  focusmaster(arg);
}
