#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>

using std::min, std::max;

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

#include "game.hpp"
#include "vector_operations.hpp"

std::string resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources/";
#endif
}

float signed_distance_to_axis_aligned_rect(const sf::Vector2f &point, const sf::Vector2f &top_left, const sf::Vector2f &bottom_right)
{
    sf::Vector2f d = max(top_left-point, point-bottom_right);
    return length(max(sf::Vector2f(0, 0), d)) + min(0.0f, max(d.x, d.y));
}


float signed_distance_from_rect_to_circle(const sf::RectangleShape &rect, const sf::CircleShape &circle) {
    return signed_distance_to_axis_aligned_rect(circle.getPosition(), rect.getPosition() - rect.getSize() / 2.0f, rect.getPosition() + rect.getSize() / 2.0f) - circle.getRadius();
}

int s_main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const float pi = 3.14159f;
    const float gameWidth = 800;
    const float gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 100.f;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight), 32), "Epic Rock Game",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile(resourcesDir() + "vine-boom.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);

    sf::SoundBuffer animeAhhSoundBuffer;
    if (!animeAhhSoundBuffer.loadFromFile(resourcesDir() + "anime-ahh.mp3"))
        return EXIT_FAILURE;
    sf::Sound animeAhhSound(animeAhhSoundBuffer);

    sf::SoundBuffer tromboneWahSoundBuffer;
    if (!tromboneWahSoundBuffer.loadFromFile(resourcesDir() + "fail-wah-wah-wah-trombone.mp3"))
        return EXIT_FAILURE;
    sf::Sound tromboneWahSound(tromboneWahSoundBuffer);

    // Create the Game logo texture:
    sf::Texture gameLogoTexture;
    if(!gameLogoTexture.loadFromFile(resourcesDir() + "rock.jpeg"))
        return EXIT_FAILURE;
    sf::Sprite gameLogo;
    gameLogo.setTexture(gameLogoTexture);
    gameLogo.setPosition(sf::Vector2f(window.getSize() / 2u - gameLogoTexture.getSize() / 2u));

    // Create the left paddle
    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize);
    leftPaddle.setOutlineThickness(3);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize / 2.f);

    // Create the right paddle
    sf::RectangleShape rightPaddle;
    rightPaddle.setSize(paddleSize);
    rightPaddle.setOutlineThickness(3);
    rightPaddle.setOutlineColor(sf::Color::Black);
    rightPaddle.setFillColor(sf::Color(200, 100, 100));
    rightPaddle.setOrigin(paddleSize / 2.f);

    // Create the ball
    sf::CircleShape ball;
    ball.setRadius(ballRadius);
    ball.setOutlineThickness(2);
    ball.setOutlineColor(sf::Color::Black);
    ball.setFillColor(sf::Color::White);
    ball.setOrigin(ballRadius, ballRadius);

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile(resourcesDir() + "tuffy.ttf"))
        return EXIT_FAILURE;

    // Initialize the pause message
    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 200.f);
    pauseMessage.setFillColor(sf::Color::White);

    #ifdef SFML_SYSTEM_IOS
    pauseMessage.setString("Welcome to Epic Rock Game!\nTouch the screen to start the game.");
    #else
    pauseMessage.setString("Welcome to Epic Rock Game!\n\nPress space to start the game.");
    #endif

    // Define the paddles properties
    sf::Clock AITimer;
    const sf::Time AITime   = sf::seconds(0.1f);
    const float paddleSpeed = 400.f;
    float rightPaddleSpeed  = 0.f;
    // const float ballSpeed   = 20.f;
    float ballSpeed   = 300.f;
    float ballAngle         = 0.f; // to be changed later

    sf::Clock clock;
    bool isPlaying = false;
    while (window.isOpen())
    {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Window closed or escape key pressed: exit
            if ((event.type == sf::Event::Closed) ||
               ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }

            // Space key pressed: play
            if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
                (event.type == sf::Event::TouchBegan))
            {
                if (!isPlaying)
                {
                    // (re)start the game
                    isPlaying = true;
                    clock.restart();

                    // Reset the position of the paddles and ball
                    leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
                    rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
                    ball.setPosition(gameWidth / 2.f, gameHeight / 2.f);

                    // Reset the ball angle
                    do
                    {
                        // Make sure the ball initial angle is not too much vertical
                        ballAngle = static_cast<float>(std::rand() % 360) * 2.f * pi / 360.f;
                    }
                    while (std::abs(std::cos(ballAngle)) < 0.7f);
                }
            }

            // Window size changed, adjust view appropriately
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(gameWidth, gameHeight);
                view.setCenter(gameWidth / 2.f, gameHeight  /2.f);
                window.setView(view);
            }
        }

        if (isPlaying)
        {
            float deltaTime = clock.restart().asSeconds();

            // Move the player's paddle
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
               (leftPaddle.getPosition().y - paddleSize.y / 2 > 5.f))
            {
                leftPaddle.move(0.f, -paddleSpeed * deltaTime);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
               (leftPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f))
            {
                leftPaddle.move(0.f, paddleSpeed * deltaTime);
            }

            if (sf::Touch::isDown(0))
            {
                sf::Vector2i pos = sf::Touch::getPosition(0);
                sf::Vector2f mappedPos = window.mapPixelToCoords(pos);
                leftPaddle.setPosition(leftPaddle.getPosition().x, mappedPos.y);
            }

            // Move the computer's paddle
            if (((rightPaddleSpeed < 0.f) && (rightPaddle.getPosition().y - paddleSize.y / 2 > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rightPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f)))
            {
                rightPaddle.move(0.f, rightPaddleSpeed * deltaTime);
            }

            // Update the computer's paddle direction according to the ball position
            if (AITimer.getElapsedTime() > AITime)
            {
                AITimer.restart();
                if (ball.getPosition().y + ballRadius > rightPaddle.getPosition().y + paddleSize.y / 2)
                    rightPaddleSpeed = paddleSpeed;
                else if (ball.getPosition().y - ballRadius < rightPaddle.getPosition().y - paddleSize.y / 2)
                    rightPaddleSpeed = -paddleSpeed;
                else
                    rightPaddleSpeed = 0.f;
            }

            // Move the ball
            float factor = ballSpeed * deltaTime;
            ball.move(std::cos(ballAngle) * factor, std::sin(ballAngle) * factor);

            #ifdef SFML_SYSTEM_IOS
            const std::string inputString = "Touch the screen to restart.";
            #else
            const std::string inputString = "Press space to restart or\nescape to exit.";
            #endif

            // Check collisions between the ball and the screen
            if (ball.getPosition().x - ballRadius < 0.f)
            {   
                tromboneWahSound.play();
                isPlaying = false;
                pauseMessage.setString("You Lost!\n\n" + inputString);
            }
            if (ball.getPosition().x + ballRadius > gameWidth)
            {
                animeAhhSound.play();
                isPlaying = false;
                pauseMessage.setString("You Won!\n\n" + inputString);
            }
            if (ball.getPosition().y - ballRadius < 0.f)
            {
                ballSound.play();
                ballAngle = -ballAngle;
                ball.setPosition(ball.getPosition().x, ballRadius + 0.1f);
            }
            if (ball.getPosition().y + ballRadius > gameHeight)
            {
                ballSound.play();
                ballAngle = -ballAngle;
                ball.setPosition(ball.getPosition().x, gameHeight - ballRadius - 0.1f);
            }

            // Check the collisions between the ball and the paddles
            // Left Paddle
            if (signed_distance_from_rect_to_circle(leftPaddle, ball) <= 0)
            {
                if (ball.getPosition().y > leftPaddle.getPosition().y)
                    ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                else
                    ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                ballSpeed *= 1.1;
                // ballRadius *= 0.9;

                ballSound.play();
                ball.setPosition(leftPaddle.getPosition().x + ballRadius + paddleSize.x / 2 + 0.1f, ball.getPosition().y);
            }

            // Right Paddle
            if (signed_distance_from_rect_to_circle(rightPaddle, ball) < 0)
            {
                if (ball.getPosition().y > rightPaddle.getPosition().y)
                    ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                else
                    ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                ballSpeed *= 1.1;
                // ballRadius *= 0.9;

                ballSound.play();
                ball.setPosition(rightPaddle.getPosition().x - ballRadius - paddleSize.x / 2 - 0.1f, ball.getPosition().y);
            }
        }

        // Clear the window
        window.clear(sf::Color(50, 50, 50));

        if (isPlaying)
        {
            // Draw the paddles and the ball
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            window.draw(ball);
        }
        else
        {
            // Draw the pause message
            window.draw(gameLogo);
            window.draw(pauseMessage);
        }

        // Display things on screen
        window.display();
    }

    return EXIT_SUCCESS;
}

int main() {
    Game game;

    Matrix<Tile> tiles(10, 10);
    tiles[4][4].kind = Tile::OpenDor;
    tiles[4][5].kind = Tile::ClosedDor;
    game.add_level(DungeonLevel(tiles));

    int result = game.init(800, 600);
    if (result != EXIT_SUCCESS) return result;
    return game.run();


    return EXIT_SUCCESS;

    // return s_main();
}

