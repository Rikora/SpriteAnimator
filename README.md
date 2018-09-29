# Sprite Animator

Sprite animation editor based on the [Thor](https://github.com/Bromeon/Thor) library extension to [SFML](https://github.com/SFML/SFML).

## Features

Sprite Animator is an easy and flexible editor which allows you to create animations in a matter of seconds. The editor
is built upon the animation module from Thor and has support for:

* Sprite sheet selection
* Grid generation for sprite sheets based on tile size
* Export animation data directly to animation files

## Screenshot

![screenshot](https://i.imgur.com/muFHwVh.png)


![screenshot](https://i.imgur.com/xWhEtIl.png)

## How-to

Setting up:

* Build the provided Visual Studio 2017 solution (`sln`) as x86 (Debug)
* Copy the contents of the `dll` folder to your `Debug` folder
* Run the program

## How-to integrate

* Add [json](https://github.com/nlohmann/json) to your project include settings 
* Add `SpriteAnimator.hpp` and `SpriteAnimator.cpp` to your project

## Example code

```c++
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>
#include <loader/SpriteAnimator.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Animation");
    sf::Clock clock;
    sf::Sprite sprite;
    sf::Texture spriteSheet;
    px::SpriteAnimator animator;

    // Prepare sprite
    spriteSheet.loadFromFile("src/res/sprites/orc.png");
    sprite.setTexture(spriteSheet);
    sprite.setPosition(sf::Vector2f(400.f, 300.f));

    // Load animations
    animator.loadAnimation("src/res/data/up_orc.anim");
    animator.loadAnimation("src/res/data/left_orc.anim");
    animator.playAnimation("left_orc", true);

    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
		
        // Update animation system
        animator.update(sprite, clock.restart());

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
```

## Remarks

* The Thor animation module have been slightly modified as crucical functionality was missing for the editor
* These changes can be found in `AnimationMap.hpp` and `AnimationMap.inl`
* If you want to use these files you can also uncomment `removeAnimation(...)` in `SpriteAnimator.hpp`

## Limitations

* Support is limited to one spritesheet

## Dependencies

* [imgui](https://github.com/ocornut/imgui)
* [imgui-sfml](https://github.com/eliasdaler/imgui-sfml)
* [nfd](https://github.com/mlabbe/nativefiledialog)
* [json](https://github.com/nlohmann/json)
* [SFML](https://github.com/SFML/SFML)
* [Thor](https://github.com/Bromeon/Thor)

## License 

This library is licensed under the MIT License, see LICENSE for more information.
