#include "game.hpp"

#include <random>

void CharacteristicsModifier::apply(Characteristics &value) {
    if (max_health) {
        value.max_health = std::visit(
            [&](auto &&v) -> auto { return (long)v.apply(value.max_health); }, *max_health
        );
    }

    if (defence) {
        value.defence =
            std::visit([&](auto &&v) -> auto { return (long)v.apply(value.defence); }, *defence);
    }

    if (speed) {
        value.speed =
            std::visit([&](auto &&v) -> auto { return (long)v.apply(value.speed); }, *speed);
    }
}

void Potion::apply(Actor &target) { modifier.apply(target.characteristics()); }

void Potion::use(Actor &target) { apply(target); }

Tile &Tile::set_building(std::unique_ptr<Chest> building) {
    this->building = std::move(building);
    return *this;
}

long RangeOfValues::get_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dis(min, max);
    return dis(gen);
}

// Game::Game(unsigned int width, unsigned int height)
//     : window(
//           sf::VideoMode(width, height, 32), "Epic Rock Game",
//           sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize | sf::Style::Fullscreen
//       ) {
//     window.setVerticalSyncEnabled(true);

//     if(!gameLogoTexture.loadFromFile(resourcesDir() + "rock.jpeg"))
//         return EXIT_FAILURE;
//     gameLogo.setTexture(gameLogoTexture);
//     gameLogo.setPosition(sf::Vector2f(window.getSize() / 2u - gameLogoTexture.getSize() / 2u));

// }

std::string resourcesDir() {
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources/";
#endif
}

int Game::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), "Epic Rock Game",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize | sf::Style::Fullscreen
    );
    window.setVerticalSyncEnabled(true);

    if (!logo_texture.loadFromFile(resourcesDir() + "rock_eyebrow_meme.png")) return EXIT_FAILURE;
    logo.setTexture(logo_texture);
    logo.setPosition(sf::Vector2f(window.getSize() / 2u - logo_texture.getSize() / 2u));

    return EXIT_SUCCESS;
}

// void Game::draw(sf::RenderTarget& target, sf::RenderStates states) {
//     target.draw(logo, states);
// }

void Game::start_playing() {
    if (!is_playing) {
        is_playing = true;
        clock.restart();

        // Reset the position of the paddles and ball
        // leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
        // rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
        // ball.setPosition(gameWidth / 2.f, gameHeight / 2.f);
    }
}

void Game::handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if ((event.type == sf::Event::Closed) ||
            ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
        {
            window.close();
            break;
        }

        if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
            (event.type == sf::Event::TouchBegan))
        {
            start_playing();
        }

        // if (event.type == sf::Event::Resized) {
        //     sf::View view;
        //     view.setSize(gameWidth, gameHeight);
        //     view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
        //     window.setView(view);
        // }
    }
}

int Game::run() {
    while (window.isOpen()) {
        window.clear(sf::Color(50, 50, 50));

        if (is_playing) {
            // window.draw(leftPaddle);
            // window.draw(rightPaddle);
            // window.draw(ball);
        } else {
            window.draw(logo);
            // window.draw(pauseMessage);
        }

        window.display();
    }

    return EXIT_SUCCESS;
}

void Game::add_level(DungeonLevel &level) { all_levels.push_back(level); }

void Game::load_level(size_t index) { current_level_index = index; }

void Game::unload_current_level() { current_level_index = -1; }

DungeonLevel *Game::get_current_level() {
    if (current_level_index < 0 || current_level_index >= all_levels.size()) {
        return nullptr;
    }
    return &all_levels[current_level_index];
}
