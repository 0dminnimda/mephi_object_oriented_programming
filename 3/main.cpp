
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

std::string resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources/";
#endif
}


float dot(const sf::Vector2f &a, const sf::Vector2f &b) {
    return a.x * b.x + a.y * b.y;
}

float length(const sf::Vector2f &a) {
    return std::sqrt(dot(a, a));
}

sf::Vector2f max(const sf::Vector2f &a, const sf::Vector2f &b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y)};
}

sf::Vector2f proj(const sf::Vector2f &a, const sf::Vector2f &b) {
    float k = dot(a, b) / dot(b, b);
    return {k * b.x, k * b.y};
}


/**
 * Returns the distance from line segment AB to point C
 */
float distance_from_segment_to_point(const sf::Vector2f &A, const sf::Vector2f &B, const sf::Vector2f &C) {
    // Compute vectors AC and AB
    sf::Vector2f AC = C - A;
    sf::Vector2f AB = B - A;

    // Get point D by taking the projection of AC onto AB then adding the offset of A
    sf::Vector2f D = proj(AC, AB) + A;

    sf::Vector2f AD = D - A;
    // D might not be on AB so calculate k of D down AB (aka solve AD = k * AB)
    // We can use either component, but choose larger value to reduce the chance of dividing by zero
    float k = std::abs(AB.x) > std::abs(AB.y) ? AD.x / AB.x : AD.y / AB.y;

    // Check if D is off either end of the line segment
    if (k <= 0.0) {
        return length(C - A);
    } else if (k >= 1.0) {
        return length(C - B);
    }

    return length(C - D);
}


float distance_from_rect_edge_to_circle(const sf::RectangleShape &rect, size_t point1, size_t point2, const sf::CircleShape &circle) {
    return distance_from_segment_to_point(
        rect.getPosition() + rect.getPoint(point1),
        rect.getPosition() + rect.getPoint(point2),
        circle.getPosition()
    ) - circle.getRadius();
}


// float distance_from_rect_to_circle(const sf::RectangleShape &rect, const sf::CircleShape &circle) {
//     return std::min(
//         std::min(
//             distance_from_rect_edge_to_circle(rect, 0, 1, circle),
//             distance_from_rect_edge_to_circle(rect, 1, 2, circle)
//         ),
//         std::min(
//             distance_from_rect_edge_to_circle(rect, 2, 3, circle),
//             distance_from_rect_edge_to_circle(rect, 3, 0, circle)
//         )
//     );
// }



float signed_distance_to_axis_aligned_rect(const sf::Vector2f &uv, const sf::Vector2f &tl, const sf::Vector2f &br)
{
    sf::Vector2f d = max(tl-uv, uv-br);
    return length(max(sf::Vector2f(0, 0), d)) + std::min(0.0f, std::max(d.x, d.y));
}


float distance_from_rect_to_circle(const sf::RectangleShape &rect, const sf::CircleShape &circle) {
    return signed_distance_to_axis_aligned_rect(circle.getPosition(), rect.getPosition() - rect.getSize() / 2.0f, rect.getPosition() + rect.getSize() / 2.0f) - circle.getRadius();
}


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const float pi = 3.14159f;
    const float gameWidth = 800;
    const float gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 100.f;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight), 32), "SFML Tennis",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile(resourcesDir() + "vine-boom.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);

    // Create the SFML logo texture:
    sf::Texture sfmlLogoTexture;
    if(!sfmlLogoTexture.loadFromFile(resourcesDir() + "rock.jpeg"))
        return EXIT_FAILURE;
    sf::Sprite sfmlLogo;
    sfmlLogo.setTexture(sfmlLogoTexture);
    sfmlLogo.setPosition(170, 50);

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
    pauseMessage.setString("Welcome to SFML Tennis!\nTouch the screen to start the game.");
    #else
    pauseMessage.setString("Welcome to SFML Tennis!\n\nPress space to start the game.");
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
                isPlaying = false;
                pauseMessage.setString("You Lost!\n\n" + inputString);
            }
            if (ball.getPosition().x + ballRadius > gameWidth)
            {
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

            // float d = distance_from_rectagle_to_circle(leftPaddle, ball);
            // std::cout << "d = " << d << std::endl;
            // std::cout << "d = " << distance_from_rect_to_circle(leftPaddle, ball) << std::endl;

            // Check the collisions between the ball and the paddles
            // Left Paddle
            // if (ball.getPosition().x - ballRadius < leftPaddle.getPosition().x + paddleSize.x / 2 &&
            //     ball.getPosition().x - ballRadius > leftPaddle.getPosition().x &&
            //     ball.getPosition().y + ballRadius >= leftPaddle.getPosition().y - paddleSize.y / 2 &&
            //     ball.getPosition().y - ballRadius <= leftPaddle.getPosition().y + paddleSize.y / 2)
            if (distance_from_rect_to_circle(leftPaddle, ball) < 0)
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


            // d = distance_from_rectagle_to_circle(rightPaddle, ball);
            // std::cout << "d = " << d << std::endl;

            // Right Paddle
            // if (ball.getPosition().x + ballRadius > rightPaddle.getPosition().x - paddleSize.x / 2 &&
            //     ball.getPosition().x + ballRadius < rightPaddle.getPosition().x &&
            //     ball.getPosition().y + ballRadius >= rightPaddle.getPosition().y - paddleSize.y / 2 &&
            //     ball.getPosition().y - ballRadius <= rightPaddle.getPosition().y + paddleSize.y / 2)
            if (distance_from_rect_to_circle(rightPaddle, ball) < 0)
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
            window.draw(pauseMessage);
            window.draw(sfmlLogo);
        }

        // Display things on screen
        window.display();
    }

    return EXIT_SUCCESS;
}
