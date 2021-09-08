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
static const unsigned int borderpx = 3; /* border pixel of windows */
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
static const char defbrowser[] = "Chromium-browser";

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
  /* xprop(1):
  *  WM_CLASS(STRING) = instance, class
  *  WM_NAME(STRING) = title
  */
  /* class              instance    title       tags mask     isfloating   monitor */
  { "Firefox",          NULL,       NULL,       1 << 8,       0,           -1 },
  { "Opera",            NULL,       NULL,       1 << 8,       0,           -1 },
  { "Google-chrome",    NULL,       NULL,       1 << 8,       0,           -1 },
  { "Chrome",           NULL,       NULL,       1 << 8,       0,           -1 },
  { "Chromium-browser", NULL,       NULL,       1 << 8,       0,           -1 },
  { "chromium-browser", NULL,       NULL,       1 << 8,       0,           -1 },
};


/* layout(s) */
static const float mfact     = 0.60; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

/* custom functions */
#include "functions.c"

static const Layout layouts[] = {
  /* symbol     arrange function */
  { "[]=",      tile },    /* first entry is default */
  { "><>",      NULL },    /* no layout function means floating behavior */
  { "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
{ MODKEY,                       KEY,      view,             {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask,           KEY,      toggleview,       {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             KEY,      tag,              {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,        {.ui = 1 << TAG} }, \

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
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
  { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
  { MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
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
  { MODKEY,                         XK_bracketleft,  zoomlast,           {0} },
  { MODKEY,                         XK_bracketright, zoomfirst,          {0} },
  { MODKEY|ShiftMask,               XK_Escape,       spawn,              SHCMD("xkill") },
  { MODKEY|ShiftMask|ControlMask,   XK_Escape,       spawn,              SHCMD("wmkill") },
  { MODKEY|ShiftMask,               XK_p,            spawn,              SHCMD("gmrun") },
  { MODKEY|ShiftMask|ControlMask,   XK_p,            spawn,              SHCMD("wmmenu") },
  { MODKEY|ControlMask,             XK_Return,       scratchpadmon,      {0} },
  { MODKEY|ShiftMask|ControlMask,   XK_Return,       zoommon,            {0} },
  { MODKEY|ControlMask,             XK_bracketleft,  spawn,              SHCMD("primary2clipboard") },
  { MODKEY|ControlMask,             XK_bracketright, spawn,              SHCMD("clipboard2primary") },
  { MODKEY|ShiftMask|ControlMask,   XK_bracketleft,  spawn,              SHCMD("xclip -o | dmenu -l 25") },
  { MODKEY|ShiftMask|ControlMask,   XK_bracketright, spawn,              SHCMD("xclip -o -selection clipboard | dmenu -l 25") },
  { MODKEY,                         XK_F9,           spawn,              SHCMD("volume toggle && wmbarupdate") },
  { MODKEY,                         XK_F11,          spawn,              SHCMD("volume down && wmbarupdate") },
  { MODKEY,                         XK_F12,          spawn,              SHCMD("volume up && wmbarupdate") },
  { MODKEY|ControlMask,             XK_s,            spawn,              SHCMD("slock") },
  { MODKEY|ControlMask,             XK_b,            spawn,              SHCMD("wmbarupdate") },
  { MODKEY,                         XK_Print,        spawn,              SHCMD("wmscreenshot full") },
  { MODKEY|ShiftMask,               XK_Print,        spawn,              SHCMD("wmscreenshot select") },
  { MODKEY,                         XK_Home,         focusclient,        {.v = defbrowser} },
  { MODKEY,                         XK_End,          reloadbrowser,      {.v = defbrowser} },
  { MODKEY,                         XK_u,            showurgent,         {0} },
  { MODKEY,                         XK_g,            toggleborder,       {0}},
  { MODKEY|ShiftMask,               XK_m,            togglefullscr,      {0} },
  { MODKEY,                         XK_r,            togglemousecursor,  {0} },
  { MODKEY,                         XK_n,            focusmaster,        {0} },
  { MODKEY|ShiftMask,               XK_n,            focuslast,          {0} },
  { MODKEY,                         XK_Right,        nexttag,            {0} },
  { MODKEY,                         XK_Left,         prevtag,            {0} },
  { MODKEY,                         XK_Prior,        focusmonmaster,     {0} },
  { MODKEY,                         XK_Next,         focusmonmaster,     {0} },
  { MODKEY|ShiftMask,               XK_Prior,        tagmon,             {.i = -1} },
  { MODKEY|ShiftMask,               XK_Next,         tagmon,             {.i = +1} },
  { MODKEY,                         XK_q,            reloaddwm,          {0} },
  { MODKEY,                         XK_s,            savekeeptags,       {0} },
  { MODKEY,                         XK_F4,           spawn,              SHCMD("keyboard-toggle && wmbarupdate") },
  { MODKEY,                         XK_o,            organize,           {0} },
  { MODKEY|ShiftMask,               XK_o,            putfilemfact,       {0} },
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
