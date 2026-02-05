/*
 * by Gonzaru
 * Distributed under the terms of the GNU General Public License v3
 */

/* My customized functions to add dwm functionality without patching dwm.c */

/* standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

/* global macros */
#define FILE_SIZE 256
#define FIRST_TAG 1 << 0
#define LAST_TAG 1 << (LENGTH(tags) - 1)
#define LINE_SIZE 256
#define SCRATCH_TAG 1 << 8

/* default nmaster */
#define DEFAULT_NMASTER defnmaster

/* default temporary directory path */
#define DEFAULT_TMPDIR "/tmp"

/* function declarations */
void debug(const char *fmt, ...);
void fakepresskey(Window win, char *typemask, char *strkey);
void focusclient(const Arg *arg);
void focuslast(const Arg *arg);
void focusmaster(const Arg *arg);
void focusmonmaster(const Arg *arg);
void focusstackfull(const Arg *arg);
char *gettmpdir(void);
char *getwindow(Window w, char *type);
int getidfromclass(const Arg *arg);
Client *getclientfromid(int winid);
int ismaster(Client *c);
int istileright(void);
void killunsel(const Arg *arg);
int layoutidx(const char *sym);
void nexttag(const Arg *arg);
void nextlayout(const Arg *arg);
void pressmousecursor(const Arg *arg);
void prevlayout(const Arg *arg);
void prevtag(const Arg *arg);
void putcurbar(void);
void putcurlayout(void);
void putcurmaster(void);
void putcurmon(void);
void putcurtag(void);
void putcurwin(void);
void putfilemfact(const Arg *arg);
void putkeeptags(void);
void reloadbrowser(const Arg *arg);
void reloaddwm(const Arg *arg);
void restoresession(const Arg *arg);
void savesession(const Arg *arg);
void scratchpadmon(const Arg *arg);
void setscratchpad(const Arg *arg);
void unsetscratchpad(const Arg *arg);
void setasmaster(Client *c);
void setasmastermon(Client *c);
void setdefmfact(const Arg *arg);
void setdefnmaster(const Arg *arg);
void setlayoutmon(const Arg *arg, Monitor *m);
void setmfactwrp(const Arg *arg);
int setwindownameclass(Window w, char *sn, char *sc);
void showapps(const Arg *arg);
void showurgent(const Arg *arg);
void spawnsh(const char *cmd);
void stackdown(const Arg *arg);
void stackup(const Arg *arg);
void tileright(Monitor *m);
void toggleborder(const Arg *arg);
void togglefullscr(const Arg *arg);
void togglemousecursor(const Arg *arg);
void togglebarpos(const Arg *arg);
void writecurbar(void);
void writecurlayout(void);
void writecurmaster(void);
void writecurmon(void);
void writecurtag(void);
void writecurwin(void);
void writecurmfact(void);
void writeclassname(Client *c);
void zoomfirst(const Arg *arg);
void zoomfirstwrp(const Arg *arg);
void zoomlast(const Arg *arg);
void zoomlastwrp(const Arg *arg);
void zoommon(const Arg *arg);

/* debug info */
void debug(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
}

/* simulate a key press */
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

/* focus the client from the window class or window id */
void focusclient(const Arg *arg)
{
  Arg a;
  Monitor *m;
  Client *c;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (strcmp(getwindow(c->win, "class"), arg->v) == 0 || c->win == atoi(arg->v)) {
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

/* focus the last client */
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

/* focus the master client */
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

/* focus the master client (multi monitor) */
void focusmonmaster(const Arg *arg)
{
  focusmon(arg);
  focusmaster(arg);
}

/* focus stack on the fullcreen window */
void focusstackfull(const Arg *arg)
{
  if (selmon->sel && selmon->sel->isfullscreen) {
    setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
  }
  focusstack(arg);
}

/* get the tmpdir */
char *gettmpdir(void)
{
  char *tmpdir;

  if (!(tmpdir = getenv("TMPDIR"))) {
    tmpdir = DEFAULT_TMPDIR;
  }
  return tmpdir;
}

/* get the client window (class, name) */
char *getwindow(Window w, char *type)
{
  XClassHint ch = { NULL, NULL };
  char *ret = NULL;

  if (XGetClassHint(dpy, w, &ch)) {
    if (strcmp(type, "class") == 0 && ch.res_class) {
      ret = strdup(ch.res_class);
    } else if (strcmp(type, "name") == 0 && ch.res_name) {
      ret = strdup(ch.res_name);
    }
    if (ch.res_class) {
      XFree(ch.res_class);
    }
    if (ch.res_name) {
      XFree(ch.res_name);
    }
  }
  return ret;
}

/* get the client window id from the window class */
int getidfromclass(const Arg *arg)
{
  Monitor *m;
  Client *c;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (strcmp(getwindow(c->win, "class"), arg->v) == 0) {
        return c->win;
      }
    }
  }
  return 0;
}

/* get the client from the window id */
Client *getclientfromid(int winid)
{
  Monitor *m;
  Client *c;

  if (!winid) {
    return NULL;
  }

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (c->win == winid) {
         return c;
      }
    }
  }
  return NULL;
}

/* check if the client is the master */
int ismaster(Client *cm)
{
  Client *c;

  if (!cm || !cm->mon->nmaster || cm->isfloating) {
    return 0;
  }

  c = nexttiled(cm->mon->clients);
  return (c == cm) ? 1 : 0;
}

/* check if the tile is on the right position (default left) */
int istileright(void)
{
  /* return strcmp(selmon->ltsymbol, "=[]") == 0; */
  return selmon->lt[selmon->sellt]->arrange == tileright;
}

/* kill all clients except the selected one (current tag) */
void killunsel(const Arg *arg)
{
  Client *c;

  if (!selmon->sel) {
    return;
  }

  for (c = selmon->clients; c; c = c->next) {
    if (ISVISIBLE(c) && c != selmon->sel) {
      if (!sendevent(c, wmatom[WMDelete])) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, c->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
      }
    }
  }
}

/* return the layout index from symbol */
int layoutidx(const char *sym)
{
  int i;

  /* layout like [%d] */
  int count;
  char trailing;

  if (!sym) {
    return -1;
  }

  for (i = 0; layouts[i].symbol; i++) {
    /* layout symbol */
    if (strcmp(layouts[i].symbol, sym) == 0) {
      return i;
    }
    /* monocle [%d] */
    if (sscanf(sym, "[%d]%c", &count, &trailing) == 1) {
      if (layouts[i].arrange == monocle) {
        return i;
      }
    }
  }
  return -1;
}

/* go to the next tag */
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

/* go to the next layout */
void nextlayout(const Arg *arg)
{
  Arg a;
  Layout *l;

  for (l = (Layout *)layouts; l->symbol && l != selmon->lt[selmon->sellt]; l++) {
    ;
  }

  /* fallback */
  if (!l->symbol) {
    l = (Layout *)layouts;
  }

  if ((l + 1)->symbol) {
    a.v = (l + 1);
    setlayout(&a);
  } else {
    a.v = &layouts[0];
    setlayout(&a);
  }
}

/* simulate a press mouse */
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

/* go to the previous layout */
void prevlayout(const Arg *arg)
{
  Arg a;
  Layout *l;
  Layout *p;

  for (l = (Layout *)layouts; l->symbol && l != selmon->lt[selmon->sellt]; l++) {
    ;
  }

  /* fallback */
  if (!l->symbol) {
    l = (Layout *)layouts;
  }

  for (p = (Layout *)layouts; (p + 1)->symbol; p++) {
    ;
  }

  if (l == (Layout *)layouts) {
    a.v = p;
    setlayout(&a);
  } else {
    a.v = (l - 1);
    setlayout(&a);
  }
}

/* go to the previous tag */
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

/* set bar */
void putcurbar(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Monitor *m;
  int fmon, fshowbar, ftopbar;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curbar.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  while (!feof(file)) {
    if (fgets(line, sizeof line - 1, file)) {
      sscanf(line, "%d %d %d", &fmon, &fshowbar, &ftopbar);
      for (m = mons; m; m = m->next) {
        if (m->num == fmon) {
          m->showbar = fshowbar;
          m->topbar = ftopbar;
          updatebarpos(m);
          XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
          arrange(m);
        }
      }
    }
  }
  fclose(file);
}

/* set layout */
void putcurlayout(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Monitor *m;
  int idx;
  int fmon;
  char fltsymbol[16];

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curlayout.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  while (!feof(file)) {
    if (fgets(line, sizeof line - 1, file)) {
      sscanf(line, "%d %s", &fmon, fltsymbol);
      for (m = mons; m; m = m->next) {
        if (m->num == fmon) {
          idx = layoutidx(fltsymbol);
          if (idx >= 0) {
            setlayoutmon(&((Arg) { .v = &layouts[idx] }), m);
          }
        }
      }
    }
  }
  fclose(file);
}

/* set as master to the latest current master window */
void putcurmaster(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Monitor *m;
  Client *c;
  int fmon;
  long unsigned int fwin;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curmaster.txt", tmpdir, user);
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

/* go to the latest monitor */
void putcurmon(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Monitor *m;
  int fmon;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curmon.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  if (fgets(line, sizeof line - 1, file)) {
    sscanf(line, "%d", &fmon);
    for (m = mons; m; m = m->next) {
      if (m->num == fmon) {
        if (m != selmon) {
          unfocus(selmon->sel, True);
          selmon = m;
          focus(NULL);
        }
        arrange(m);
        break;
      }
    }
  }
  fclose(file);
}

/* go to the latest monitor current tag */
void putcurtag(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Monitor *m;
  int fmon;
  int ftag;
  Arg a;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curtag.txt", tmpdir, user);
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

/* focus to the latest current window */
void putcurwin(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Arg a;
  Monitor *m;
  Client *c;
  int fmon;
  long unsigned int fwin;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curwin.txt", tmpdir, user);
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

/* set the fmfact for all monitors */
void putfilemfact(const Arg *arg)
{
  Monitor *m;
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  int fmon;
  float fmfact;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curmfact.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  while (!feof(file)) {
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
  }
  fclose(file);
}

/* put the tags after reload */
void putkeeptags(void)
{
  Arg a;
  Monitor *m;
  Client *c;
  FILE *file;
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  int fmon;
  int ftag;
  int fmaster;
  int fsel;
  int ffloating;
  long unsigned int fwin;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-state-tags.txt", tmpdir, user);
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

/* reload the default browser */
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
      if (strcmp(getwindow(c->win, "class"), arg->v) == 0 || (strcmp(defbrowser, arg->v) == 0)) {
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

/* reload dwm */
void reloaddwm(const Arg *arg)
{
  savesession(arg);
  spawnsh("${HOME}/bin/wmreload");
}

/* save the session */
void savesession(const Arg *arg)
{
  Monitor *m;
  Client *c;
  FILE *file = NULL;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  writecurmon();
  writecurlayout();
  writecurbar();
  writecurmfact();
  writecurtag();
  writecurmaster();
  writecurwin();

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-session.lock", tmpdir, user);
  remove(filepath);

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-state-tags.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      fprintf(file, "%d %d %d %d %d %lud %s %s\n", c->mon->num,
        c->tags, (ismaster(c) == 1) ? 1 : 0, (c == selmon->sel) ? 1 : 0,
        (c->isfloating) ? 1 : 0, c->win, getwindow(c->win, "class"), c->name);
    }
  }
  fclose(file);
}

/* restore the session */
void restoresession(const Arg *arg)
{
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  FILE *lock;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-session.lock", tmpdir, user);
  if ((lock = fopen(filepath, "r"))) {
    fclose(lock);
    spawnsh("echo 'dwm session already executed!' | dmenu");
    return;
  }
  if (!(lock = fopen(filepath, "w"))) {
    debug("can't create lock %s: %s", filepath, strerror(errno));
    return;
  }
  fclose(lock);

  /* set bar */
  putcurbar();

  /* set layout */
  putcurlayout();

  /* put tags after reload */
  putkeeptags();

  /* set fmfact for all monitors */
  putfilemfact(arg);

  /* set as master to latest current master window */
  putcurmaster();

  /* focus to latest current window */
  putcurwin();

  /* go to latest monitor current tag */
  putcurtag();

  /* go to lastest monitor */
  putcurmon();

  /* remove lock */
  unlink(filepath);
}

/* set the scratchpad */
void setscratchpad(const Arg *arg) {
  Monitor *m;
  Client *c;
  char *scratchclass = ((char **)arg->v)[1];

  if (!selmon->sel) {
    return;
  }

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (strcmp(scratchclass, getwindow(c->win, "class")) == 0) {
        spawnsh("echo 'cannot create a new dynamic scratch, already exists!' | dmenu");
        return;
      }
    }
  }
  writeclassname(selmon->sel);
  setwindownameclass(selmon->sel->win, scratchclass, scratchclass);
}

/* unset the scratchpad */
void unsetscratchpad(const Arg *arg) {
  FILE *file;
  char *scratchclass = ((char **)arg->v)[1];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  char filepath[FILE_SIZE];
  char line[LINE_SIZE];
  char prevscratchclass[FILE_SIZE];
  char prevscratchname[FILE_SIZE];

  if (!selmon->sel) {
    return;
  }

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-nameclass.txt", tmpdir, user);
  if (!(file = fopen(filepath, "r"))) {
    debug("can't open file %s", filepath);
    return;
  }

  if (strcmp(scratchclass, getwindow(selmon->sel->win, "class")) == 0) {
    if (fgets(line, sizeof line - 1, file)) {
      sscanf(line, "%s %s", prevscratchname, prevscratchclass);
      setwindownameclass(selmon->sel->win, prevscratchname, prevscratchclass);
    }
  } else {
    spawnsh("echo 'this is not a dynamic scratch!' | dmenu");
  }
  fclose(file);
}

/* scratchpad (multi monitor) */
void scratchpadmon(const Arg *arg)
{
  Monitor *m;
  Client *c;
  const char *scratchcmd = ((char **)arg->v)[0];
  const char *scratchclass = ((char **)arg->v)[1];
  int match = 0;

  for (m = mons; m && !match; m = m->next) {
    for (c = m->clients; c && !match; c = c->next) {
      if (strcmp(scratchclass, getwindow(c->win, "class")) == 0) {
        match = 1;
        if (c->mon != selmon) {
          sendmon(c, selmon);
          arrange(c->mon);
          setasmaster(c);
        } else if (!ISVISIBLE(c)) {
          c->tags = selmon->tagset[selmon->seltags];
          arrange(c->mon);
          setasmaster(c);
        } else if (c == selmon->sel && c->tags != SCRATCH_TAG) {
          c->tags = SCRATCH_TAG;
          arrange(c->mon);
          focus(NULL);
        } else {
          setasmaster(c);
        }
      }
    }
  }
  /* run it if not present */
  if (!match && scratchcmd) {
    spawnsh(scratchcmd);
  }
}

/* put the client in master area */
void setasmaster(Client *cm)
{
  Client *c;
  int nc = 0;

  if (!cm || cm->isfloating) {
    return;
  }

  /* check if has more than 1 client */
  for (c = selmon->clients; c && nc < 2; c = c->next) {
      if (ISVISIBLE(c) && !c->isfloating) {
        nc++;
      }
  }
  if (nc >= 2) {
    pop(cm);
  }
}

/* put the client in the master area (multi monitor) */
void setasmastermon(Client *cm)
{
  Monitor *m;
  Client *c;

  if (!cm || cm->isfloating) {
    return;
  }

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (c == cm) {
        if (c->mon != selmon) {
          sendmon(c, selmon);
          arrange(c->mon);
          setasmaster(c);
        } else if (!ISVISIBLE(c)) {
          c->tags = selmon->tagset[selmon->seltags];
          arrange(c->mon);
          setasmaster(c);
        } else {
          setasmaster(c);
        }
        return;
      }
    }
  }
}

/* set the default mfact */
void setdefmfact(const Arg *arg)
{
  if (!arg || !selmon->lt[selmon->sellt]->arrange) {
    return;
  }

  if (arg->f < 0.05 || arg->f > 0.95) {
    return;
  }

  /* TODO: multi monitor? */
  selmon->mfact = arg->f;
  arrange(selmon);
}

/* set the layout by monitor */
void setlayoutmon(const Arg *arg, Monitor *m)
{
  if (!arg || !arg->v || arg->v != m->lt[m->sellt]) {
    m->sellt ^= 1;
  }
  if (arg && arg->v) {
    m->lt[m->sellt] = (Layout *)arg->v;
  }
  strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
  if (m->sel) {
    arrange(m);
  } else {
    drawbar(m);
  }
}

/* set the default nmaster */
void setdefnmaster(const Arg *arg)
{
  selmon->nmaster = DEFAULT_NMASTER;
  arrange(selmon);
}

/* set mfact wrapper */
void setmfactwrp(const Arg *arg)
{
  Arg a = *arg;

  if (istileright()) {
    a.f = -a.f;
  }
  setmfact(&a);
}

/* set the client window class */
int setwindownameclass(Window w, char *sn, char *sc)
{
  XClassHint ch = { NULL, NULL };

  ch.res_name = sn;
  ch.res_class = sc;
  return XSetClassHint(dpy, w, &ch);
}

/* show the apps and focus/kill/master the selected one */
void showapps(const Arg *arg)
{
  Arg a;
  Monitor *m;
  Client *c;
  FILE *file = NULL;
  char *tmpdir = gettmpdir();
  char *user = getenv("USER");
  char filepath1[FILE_SIZE];
  char filepath2[FILE_SIZE];
  char line[LINE_SIZE];
  char cmd[sizeof filepath1 + sizeof filepath2 + 37]; /* 37 dmenu|sort|cut */
  int nclients = 0;
  int winid = 0;

  snprintf(filepath1, sizeof filepath1, "%s/%s-dwm-showapps-list.txt", tmpdir, user);
  if (!(file = fopen(filepath1, "w+"))) {
    debug("can't create file %s", filepath1);
    return;
  }
  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      fprintf(file, "%s %lud\n", getwindow(c->win, "class"), c->win);
      nclients++;
    }
  }
  fclose(file);

  if (!nclients) {
    spawnsh("echo 'no apps to show!' | dmenu");
    return;
  }

  snprintf(filepath2, sizeof filepath2, "%s/%s-dwm-showapps-select.txt", tmpdir, user);
  snprintf(cmd, sizeof cmd, "sort %s | dmenu -l %d | cut -d ' ' -f2 > %s", filepath1, nclients, filepath2);
  if (system(cmd) && (file = fopen(filepath2, "r"))) {
    if (fgets(line, sizeof line - 1, file)) {
      /* remove '\n' from fgets */
      line[strcspn(line, "\n")] = '\0';
      a.v = line;
      if (strcmp(arg->v, "focus") == 0) {
        focusclient(&a);
      } else if (strcmp(arg->v, "kill") == 0) {
        winid = atoi(a.v);
        if (winid) {
          XKillClient(dpy, winid);
        }
      } else if (strcmp(arg->v, "master") == 0) {
        setasmastermon(getclientfromid(atoi(a.v)));
      }
    }
    fclose(file);
  }
}

/* focus the urgent window */
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

/* move the client down from the stack */
void stackdown(const Arg *arg) {
  Client *c;

  if (!selmon->sel || selmon->sel->isfloating || ismaster(selmon->sel)) {
    return;
  }

  c = nexttiled(selmon->sel->next);
  if (c) {
    detach(selmon->sel);
    selmon->sel->next = c->next;
    c->next = selmon->sel;
    focus(selmon->sel);
    arrange(selmon);
  }
}

/* move the client up from the stack */
void stackup(const Arg *arg) {
  Client *c, *p;

  if (!selmon->sel || selmon->sel->isfloating || ismaster(selmon->sel)) {
    return;
  }

  c = NULL;
  for (p = selmon->clients; p && p != selmon->sel; p = p->next) {
    if (!p->isfloating && ISVISIBLE(p)) {
      c = p;
    }
  }
  if (c && !ismaster(c)) {
    detach(selmon->sel);
    selmon->sel->next = c;
    for (c = selmon->clients; c->next != selmon->sel->next; c = c->next) {
      ;
    }
    c->next = selmon->sel;
    focus(selmon->sel);
    arrange(selmon);
  }
}

/* tile right (layout) */
void tileright(Monitor *m)
{
  unsigned int i, n, h, mw, my, ty;
  Client *c;

  for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++) {
    ;
  }
  if (n == 0) {
    return;
  }

  if (n > m->nmaster) {
    mw = m->nmaster ? m->ww * m->mfact : 0;
  } else {
    mw = m->ww;
  }

  for (i = my = ty = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
    if (i < m->nmaster) {
      h = (m->wh - my) / (MIN(n, m->nmaster) - i);
      resize(c, m->wx + (m->ww - mw), m->wy + my, mw - (2*c->bw), h - (2*c->bw), 0);
      if (my + HEIGHT(c) < m->wh) {
        my += HEIGHT(c);
      }
   } else {
      h = (m->wh - ty) / (n - i);
      resize(c, m->wx, m->wy + ty, m->ww - mw - (2*c->bw), h - (2*c->bw), 0);
      if (ty + HEIGHT(c) < m->wh) {
        ty += HEIGHT(c);
      }
    }
  }
}

/* toggle the window border */
void toggleborder(const Arg *arg)
{
  if (!selmon->sel) {
    return;
  }
  if (selmon->sel->bw) {
    selmon->sel->oldbw = selmon->sel->bw;
    selmon->sel->bw = 0;
  } else {
    selmon->sel->bw = selmon->sel->oldbw;
  }
  arrange(selmon);
}

/* toggle to fullscreen */
void togglefullscr(const Arg *arg)
{
  if (selmon->sel) {
    setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
  }
}

/* toggle the mouse cursor */
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

/* toggle bar position between top and bottom */
void togglebarpos(const Arg *arg)
{
  if (selmon->showbar) {
    selmon->topbar = !selmon->topbar;
    updatebarpos(selmon);
    XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bh);
    arrange(selmon);
  }
}

/* write the current bar */
void writecurbar(void)
{
  FILE *file;
  Monitor *m;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curbar.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    fprintf(file, "%d %d %d\n", m->num, m->showbar, m->topbar);
  }
  fclose(file);
}

/* write the current layout */
void writecurlayout(void)
{
  FILE *file;
  Monitor *m;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curlayout.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    fprintf(file, "%d %s\n", m->num, m->ltsymbol);
  }
  fclose(file);
}

/* write the current master window */
void writecurmaster(void)
{
  FILE *file;
  Monitor *m;
  Client *c;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  if (!selmon->sel || selmon->sel->isfloating) {
    return;
  }

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curmaster.txt", tmpdir, user);
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

/* write the current monitor */
void writecurmon(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  if (!selmon) {
    return;
  }

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curmon.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }
  fprintf(file, "%d\n", selmon->num);
  fclose(file);
}

/* write the current tag */
void writecurtag(void)
{
  FILE *file;
  Monitor *m;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curtag.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    fprintf(file, "%d %d\n", m->num, m->tagset[m->seltags]);
  }
  fclose(file);
}

/* write the current window */
void writecurwin(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();

  if (!selmon->sel) {
    return;
  }

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curwin.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }
  fprintf(file, "%d %lud\n", selmon->num, selmon->sel->win);
  fclose(file);
}

/* write the current mfact */
void writecurmfact(void)
{
  FILE *file;
  char filepath[FILE_SIZE];
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  Monitor *m;

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-curmfact.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  for (m = mons; m; m = m->next) {
    fprintf(file, "%d %.2f\n", m->num, m->mfact);
  }
  fclose(file);
}

/* write the client window class name */
void writeclassname(Client *c)
{
  FILE *file;
  char *user = getenv("USER");
  char *tmpdir = gettmpdir();
  char filepath[FILE_SIZE];

  if (!c) {
    return;
  }

  snprintf(filepath, sizeof filepath, "%s/%s-dwm-classname.txt", tmpdir, user);
  if (!(file = fopen(filepath, "w+"))) {
    debug("can't create file %s", filepath);
    return;
  }

  fprintf(file, "%s %s\n", getwindow(c->win, "name"), getwindow(c->win, "class"));
  fclose(file);
}

/* zoom the first window */
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

/* zoom first wrapper */
void zoomfirstwrp(const Arg *arg)
{
  if (istileright()) {
    zoomlast(arg);
  } else {
    zoomfirst(arg);
  }
}

/* zoom the last window */
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

/* zoom last wrapper */
void zoomlastwrp(const Arg *arg)
{
  if (istileright()) {
    zoomfirst(arg);
  } else {
    zoomlast(arg);
  }
}

/* zoom/cycles the monitors */
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
    if (!ac[i]) {
      continue;
    }
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
  /* swap previous tagset */
  m1->tagset[m1->seltags] = m2_tagset;
  m2->tagset[m2->seltags] = m1_tagset;
  arrange(m1);
  arrange(m2);
  focusmaster(arg);
}
