# Gravity-Simulator-v1.0-SOURCE

This is the source code for my gravity simulation project. DISCLAIMER: Keep in
mind it started out as a simple test. So the code is horribly organized. I am
also Norwegian, so some stuff might be in Norwegian. I've tried to make it
easier to understand. https://www.youtube.com/channel/UCslIyA4_1rciR3loFI09pZQ


# Compiling for Debian

Gravity-Simulator uses the graphics library [SFML](http://www.sfml-dev.org/)
and the user interface library [TGUI](https://www.tgui.eu/).

## Dependencies

```bash
# >> /etc/apt/sources.list <<< "deb http://ppa.launchpad.net/texus/tgui/ubuntu xenial main"
# apt update
# apt install libsfml-dev libtgui-dev
```

# Compile

```bash
$ make
```
