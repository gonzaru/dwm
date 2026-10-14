/*
 * by Gonzaru
 * Distributed under the terms of the GNU General Public License v3
 */

/* enable multimedia key buttons */
/* #include <X11/XF86keysym.h> */

/* some custom colors & themes */
#if 1
#include "themes.h"
#endif

/* appearance */
static const unsigned int borderpx = 1; /* border pixel of windows */
static const unsigned int snap = 32; /* snap pixel */
static const int showbar = 1; /* False means no bar */
static const int topbar = 1; /* False means bottom bar */
static const char *fonts[] = { "DejaVu Sans Mono:pixelsize=16:antialias=true:autohint=true" };
static const char dmenufont[] = "DejaVu Sans Mono:pixelsize=16:antialias=true:autohint=true";
static const char normbgcolor[] = BLACK;
static const char normbordercolor[] = "#444444";
static const char normfgcolor[] = WHITE;
static const char selfgcolor[] = WHITE;
static const char selbordercolor[] = PLAN9_BLUE_SELECTED;
static const char selbgcolor[] = BLUE_ANGELS;
static const char *colors[][3] = {
  /*               fg         bg         border   */
  [SchemeNorm] = {normfgcolor, normbgcolor, normbordercolor},
  [SchemeSel] = {selfgcolor, selbgcolor, selbordercolor},
};

/* browser for maps like focus/reload */
static const char defbrowser[] = "firefox";

/* default scratch applications */
static const char *defscratchappbrowser[] = { "firefox", "firefox", NULL };
static const char *defscratchappdyn[] = { NULL, "Scratchpaddyn", NULL };
static const char *defscratchapp1[] = { "xterm -u8 -class 'Scratchpad1'", "Scratchpad1", NULL };
static const char *defscratchapp2[] = { "emacs", "Emacs", NULL };
static const char *defscratchapp3[] = { "gvim", "Gvim", NULL };

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
  /* xprop(1):
  *  WM_CLASS(STRING) = instance, class
  *  WM_NAME(STRING) = title
  */
  /* class              instance    title       tags mask     isfloating   monitor */
  { "Firefox",          NULL,       NULL,       1 << 8,       0,           -1 },
  { "Google-chrome",    NULL,       NULL,       1 << 8,       0,           -1 },
  { "Chromium-browser", NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.65;    /* factor of master area size [0.05..0.95] */
static const float defmfact  = mfact;   /* default mfact */
static const int nmaster     = 1;       /* number of clients in master area */
static const int defnmaster  = nmaster; /* default nmaster */
static const int resizehints = 0;       /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1;    /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;     /* refresh rate (per second) for client move/resize */

#define WITH_XTEST 0                    /* set to 1 to use XTest support, 0 is disabled (used by sendkey) */
#if WITH_XTEST
static const int modkeycode = 0;        /* 0 means dynamically guess Super_L, else use exact keycode (e.g. 133) */
#endif

static void deck(Monitor *m);
static void deckright(Monitor *m);    /* master on right */
static void tileright(Monitor *m);    /* master on right */
static void tilecol(Monitor *m);
static void tilecolright(Monitor *m); /* master on right */
static const Layout layouts[] = {
  /* symbol     arrange function */
  { "[]=",      tile },         /* first entry is default */
  { "><>",      NULL },         /* no layout function means floating behavior */
  { "[M]",      monocle },
  { "=[]",      tileright },    /* master on right */
  { "[]|",      tilecol },
  { "|[]",      tilecolright }, /* master on right */
  { "[]:",      deck },
  { ":[]",      deckright },    /* master on right */
  { NULL,       NULL },
};

/* custom functions */
#include "functions.c"

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
{ MODKEY,                       KEY,      view,             {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask,           KEY,      toggleview,       {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             KEY,      tag,              {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,        {.ui = 1 << TAG} }, \

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, topbar ? NULL : "-b", NULL };
static const char *termcmd[]  = { "xterm", NULL };

static Key keys[] = {
  /* modifier                     key        function        argument */

  /* DEFAULTS */
  { MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
  { MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
  { MODKEY,                       XK_b,      togglebar,      {0} },
  { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
  { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
  { MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
  { MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
  /*
  { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
  { MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
  */
  { MODKEY,                       XK_h,      setmfactwrp,    {.f = -0.05} },
  { MODKEY,                       XK_l,      setmfactwrp,    {.f = +0.05} },
  { MODKEY,                       XK_Return, zoom,           {0} },
  { MODKEY,                       XK_Tab,    view,           {0} },
  { MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
  { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
  { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
  { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
  { MODKEY,                       XK_space,  setlayout,      {0} },
  { MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
  { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
  { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
  { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
  { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
  { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
  { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
  TAGKEYS(                        XK_1,                      0)
  TAGKEYS(                        XK_2,                      1)
  TAGKEYS(                        XK_3,                      2)
  TAGKEYS(                        XK_4,                      3)
  TAGKEYS(                        XK_5,                      4)
  TAGKEYS(                        XK_6,                      5)
  TAGKEYS(                        XK_7,                      6)
  TAGKEYS(                        XK_8,                      7)
  TAGKEYS(                        XK_9,                      8)
  { MODKEY|ShiftMask,             XK_q,      quit,           {0} },
  /* END DEFAULTS */

  /* MY MODKEYS */
  { MODKEY|ShiftMask,               XK_t,            setlayout,          {.v = &layouts[3]} }, /* tile layout (master right) */
  { MODKEY,                         XK_bracketleft,  zoomfirstwrp,       {0} },
  { MODKEY|ShiftMask,               XK_bracketleft,  stackup,            {0} },
  { MODKEY,                         XK_bracketright, zoomlastwrp,        {0} },
  { MODKEY|ShiftMask,               XK_bracketright, stackdown,          {0} },
  { MODKEY|ShiftMask,               XK_j,            focusstackfull,     {.i = +1 } },
  { MODKEY|ControlMask,             XK_j,            focusstackmon,      {.i = +1 } },
  { MODKEY|ShiftMask,               XK_k,            focusstackfull,     {.i = -1 } },
  { MODKEY|ControlMask,             XK_k,            focusstackmon,      {.i = -1 } },
  { MODKEY|ControlMask,             XK_Tab,          focusclass,         {.i = +1 } },
  { MODKEY|ControlMask|ShiftMask,   XK_Tab,          focusclass,         {.i = -1 } },
  { MODKEY|ControlMask|ShiftMask,   XK_c,            killunsel,          {0} },
  { MODKEY|ControlMask,             XK_Escape,       spawn,              SHCMD("xkill") },
  { MODKEY|ControlMask|ShiftMask,   XK_Escape,       spawn,              SHCMD("wmkill") },
  { MODKEY|ShiftMask,               XK_p,            spawn,              SHCMD("gmrun") },
  { MODKEY|ControlMask|ShiftMask,   XK_p,            spawn,              SHCMD("wmmenu") },
  { MODKEY,                         XK_w,            scratchpadmon,      {.v = defscratchappbrowser} },
  { MODKEY|ControlMask,             XK_Return,       scratchpadmon,      {.v = defscratchapp1} },
  { MODKEY,                         XK_e,            scratchpadmon,      {.v = defscratchapp2} },
  { MODKEY|ShiftMask,               XK_e,            scratchpadmon,      {.v = defscratchapp3} },
  { MODKEY|ControlMask,             XK_i,            setscratchpad,      {.v = defscratchappdyn} },
  { MODKEY|ControlMask|ShiftMask,   XK_i,            unsetscratchpad,    {.v = defscratchappdyn} },
  { MODKEY|ControlMask,             XK_o,            scratchpadmon,      {.v = defscratchappdyn} },
  { MODKEY|ControlMask|ShiftMask,   XK_Return,       zoommon,            {0} },
  { MODKEY,                         XK_a,            spawn,              SHCMD("primary2clipboard") },
  { MODKEY|ShiftMask,               XK_a,            spawn,              SHCMD("clipboard2primary") },
  { MODKEY|ControlMask,             XK_a,            spawn,              SHCMD("xclip -o | dmenu -l 25") },
  { MODKEY|ControlMask|ShiftMask,   XK_a,            spawn,              SHCMD("xclip -o -selection clipboard | dmenu -l 25") },
  { MODKEY,                         XK_F10,          spawn,              SHCMD("volume toggle && wmbarupdate") },
  { MODKEY|ShiftMask,               XK_F10,          spawn,              SHCMD("volume toggleapp && wmbarupdate") },
  { MODKEY,                         XK_F11,          spawn,              SHCMD("volume down && wmbarupdate") },
  { MODKEY|ShiftMask,               XK_F11,          spawn,              SHCMD("volume downapp && wmbarupdate") },
  { MODKEY,                         XK_F12,          spawn,              SHCMD("volume up && wmbarupdate") },
  { MODKEY|ShiftMask,               XK_F12,          spawn,              SHCMD("volume upapp && wmbarupdate") },
  { MODKEY,                         XK_s,            spawn,              SHCMD("sleep 0.2 && slock") },
  { MODKEY|ShiftMask,               XK_s,            savesession,        {0} },
  { MODKEY|ControlMask,             XK_s,            restoresession,     {0} },
  { MODKEY|ShiftMask,               XK_b,            togglebarpos,       {0} },
  { MODKEY|ControlMask,             XK_b,            spawn,              SHCMD("wmbarupdate") },
  { MODKEY,                         XK_Print,        spawn,              SHCMD("sleep 0.2 && wmscreenshot full") },
  { MODKEY|ShiftMask,               XK_Print,        spawn,              SHCMD("sleep 0.2 && wmscreenshot select") },
  { MODKEY,                         XK_Home,         focusclient,        {.v = defbrowser} },
  { MODKEY,                         XK_End,          reloadbrowser,      {.v = defbrowser} },
  { MODKEY,                         XK_u,            showurgent,         {0} },
  { MODKEY|ShiftMask,               XK_u,            showapps,           {.v = "focus"} },
  { MODKEY|ControlMask,             XK_u,            showapps,           {.v = "master"} },
  { MODKEY|ControlMask|ShiftMask,   XK_u,            showapps,           {.v = "kill"} },
  { MODKEY,                         XK_g,            toggleborder,       {0}},
  { MODKEY|ShiftMask,               XK_m,            togglefullscr,      {0} },
  { MODKEY,                         XK_r,            togglemousecursor,  {0} },
  { MODKEY,                         XK_n,            focusmaster,        {0} },
  { MODKEY|ShiftMask,               XK_n,            focuslast,          {0} },
  { MODKEY,                         XK_Right,        nexttag,            {0} },
  { MODKEY,                         XK_Left,         prevtag,            {0} },
  { MODKEY,                         XK_Prior,        focusmonmaster,     {.i = -1} },
  { MODKEY,                         XK_Next,         focusmonmaster,     {.i = +1} },
  { MODKEY|ShiftMask,               XK_Prior,        tagmon,             {.i = -1} },
  { MODKEY|ShiftMask,               XK_Next,         tagmon,             {.i = +1} },
  { MODKEY,                         XK_F4,           spawn,              SHCMD("setxkbsw -n && wmbarupdate") },
  { MODKEY,                         XK_z,            nextlayout,         {0} },
  { MODKEY|ShiftMask,               XK_z,            prevlayout,         {0} },
  { MODKEY,                         XK_q,            reloaddwm,          {0} },
  { MODKEY,                         XK_equal,        setdefmfact,        {.f = defmfact} },
  { MODKEY|ShiftMask,               XK_equal,        setdefnmaster,      {0} },

  /* example of sendkey for h,j,k,l style */
  /*
  #if WITH_XTEST
  { MODKEY|ALTKEY,                  XK_h,            sendkey,            {.ui = XK_Left} },
  { MODKEY|ALTKEY,                  XK_j,            sendkey,            {.ui = XK_Down} },
  { MODKEY|ALTKEY,                  XK_k,            sendkey,            {.ui = XK_Up} },
  { MODKEY|ALTKEY,                  XK_l,            sendkey,            {.ui = XK_Right} },
  #endif
  */

  /* example of focusstacknum */
  /*
  { MODKEY|ALTKEY,                  XK_1,            focusstacknum,      {.i = 0} },
  { MODKEY|ALTKEY,                  XK_2,            focusstacknum,      {.i = 1} },
  { MODKEY|ALTKEY,                  XK_3,            focusstacknum,      {.i = 2} },
  { MODKEY|ALTKEY,                  XK_4,            focusstacknum,      {.i = 3} },
  { MODKEY|ALTKEY,                  XK_5,            focusstacknum,      {.i = 4} },
  { MODKEY|ALTKEY,                  XK_6,            focusstacknum,      {.i = 5} },
  { MODKEY|ALTKEY,                  XK_7,            focusstacknum,      {.i = 6} },
  { MODKEY|ALTKEY,                  XK_8,            focusstacknum,      {.i = 7} },
  { MODKEY|ALTKEY,                  XK_9,            focusstacknum,      {.i = 8} },
  */

};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
  /* click                event mask      button          function        argument */
  { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
  { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
  { ClkWinTitle,          0,              Button2,        zoom,           {0} },
  { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
  { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
  { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
  { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
  { ClkTagBar,            0,              Button1,        view,           {0} },
  { ClkTagBar,            0,              Button3,        toggleview,     {0} },
  { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
  { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
