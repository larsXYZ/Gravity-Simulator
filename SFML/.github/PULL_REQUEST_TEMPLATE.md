Thanks a lot for making a contribution to SFML! 🙂

Before you create the pull request, we ask you to check the follow boxes. (For small changes not everything needs to ticked, but the more the better!)

* [ ] Has this change been discussed on [the forum](https://en.sfml-dev.org/forums/index.php#c3) or in an issue before?
* [ ] Does the code follow the SFML [Code Style Guide](https://www.sfml-dev.org/style.php)?
* [ ] Have you provided some example/test code for your changes?
* [ ] If you have additional steps which need to be performed list them as tasks!

----

## Description

Please describe your pull request.

This PR is related to the issue #

## Tasks

* [ ] Tested on Linux
* [ ] Tested on Windows
* [ ] Tested on macOS
* [ ] Tested on iOS
* [ ] Tested on Android

## How to test this PR?

Describe how to best test these changes. Please provide a [minimal, complete and verifiable example](https://stackoverflow.com/help/mcve) if possible, you can use the follow template as a start:

```cpp
#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Minimal, complete and verifiable example");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.display();
    }
}
```