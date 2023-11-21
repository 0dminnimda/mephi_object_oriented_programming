#include "game.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "vector_operations.hpp"


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

void Weapon::use(Actor &target) {
    attack(target.position());
}

void Weapon::attack(sf::Vector2f position) {
    
}

Tile &Tile::set_building(std::unique_ptr<Chest> building) {
    this->building = building.get();
    return *this;
}

long RangeOfValues::get_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dis(min, max);
    return dis(gen);
}

void InventoryCanvas::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    
}

void LevelUpCanvas::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    
}

#ifdef SFML_SYSTEM_IOS
    static const std::string path_to_resources = "";
#else
    static const std::string path_to_resources = "resources/";
#endif

static const char * const logo_name = "rock_eyebrow_meme.png";

void center_text(sf::Text &text) {
    // const sf::String string = text.getString();

    // float max_height = 0;
    // const sf::Font &font = *text.getFont();
    // for (size_t characterIndex = 0; characterIndex < string.getSize(); ++characterIndex)
    // {
    //     auto bounds = font.getGlyph(string[characterIndex], text.getCharacterSize(), false).bounds;
    //     max_height = std::max(max_height, bounds.height);
    // }

    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin(text_rect.left + text_rect.width / 2.0f, text_rect.top  + text_rect.height / 2.0f);
}

int Game::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), "Epic Rock Game",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize | sf::Style::Fullscreen
    );
    window.setVerticalSyncEnabled(true);

    if (!logo_texture.loadFromFile(path_to_resources + logo_name)) return EXIT_FAILURE;
    logo.setTexture(logo_texture);
    float scale = min(sf::Vector2f(window.getSize()) / sf::Vector2f(logo_texture.getSize()) / 2.0f);
    logo.setScale({scale, scale});
    logo.setOrigin(sf::Vector2f(logo_texture.getSize()) / 2.0f);
    logo.setPosition({window.getSize().x / 2.0f, window.getSize().y * 2.0f / 3.0f});

    if (!font.loadFromFile(path_to_resources + "tuffy.ttf")) return EXIT_FAILURE;
    menu_message.setFont(font);
    menu_message.setCharacterSize(40);
    menu_message.setFillColor(sf::Color::White);
#ifdef SFML_SYSTEM_IOS
    menu_message.setString("Welcome to Epic Lab3 Game!\nTouch the screen to start the game.");
#else
    menu_message.setString("Welcome to Epic Lab3 Game!\n\nPress space to start the game.");
#endif
    center_text(menu_message);
    menu_message.setPosition({window.getSize().x / 2.0f, window.getSize().y / 6.0f});

    return EXIT_SUCCESS;
}

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
        handle_events();

        window.clear(sf::Color(50, 50, 50));
        draw(window);
        window.display();
    }

    return EXIT_SUCCESS;
}

void Game::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (is_playing) {
        // target.draw(leftPaddle);
        // target.draw(rightPaddle);
        // target.draw(ball);
    } else {
        target.draw(logo, states);
        target.draw(menu_message, states);
    }
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
