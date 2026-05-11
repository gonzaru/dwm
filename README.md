# Modular build

This build is easy to maintain. It keeps the `dwm.c` code separate from new features.

## No Core Patches

The main goal is to avoid patching directly `dwm.c`. This approach has these benefits:

- **Easy Updates**: It is easy to add new dwm versions because the core code stays original.
- **Organized**: All new features stay in `functions.c`. This makes the code simple to follow.

## Main Features

This method adds these features without changing the core:

- **Better Workflow**: Improved window focus, browser tools, and navigation.
- **New Layouts**: Includes new window layouts.
- **Save Sessions**: Remembers window layouts and states when restarting.
- **Scratchpads**: Quick access to apps through a special scratchpad system.

## More Features

The `functions.c` file has more features for daily work, etc.

## Installation

#### 1. Clone

    $ git clone https://github.com/gonzaru/dwm.git

#### 2. Copy

    # copy the files to your dwm directory
    $ cp {config.h,functions.c,themes.h} /path/to/dwm

#### 3. Build

    $ cd /path/to/dwm
    $ make clean
    $ make

#### 4. Install (optional)

    # copy the binary file to any searchable shell $PATH, for example:
    $ sudo cp dwm /usr/local/bin/

* Or

```
$ make install (use sudo if necessary)
```

## Configuration

Customize the build using `config.h` and `themes.h`. The functions in `functions.c` work just like standard dwm functions. Use them in the keys and buttons arrays in `config.h`.

---
Maintained by Gonzaru | Distributed under the GPL-3.0 License
