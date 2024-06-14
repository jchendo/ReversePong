#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>;

class Game {

public:
    Game();
    void        run();

private:
    void        processEvents(sf::Clock);
    void        update(sf::Time);
    void        render();
    void        handlePlayerInput(sf::Keyboard::Key, bool);
    void        collisionDetection();
    void        resetGameAttributes();

private:
    sf::RenderWindow   mWindow;
    sf::CircleShape    player;
    sf::RectangleShape leftPaddle, rightPaddle;
    sf::Time           TimePerFrame;
    sf::Text           ScoreText, TitleText, StartText;
    sf::Font           font;
    sf::Mouse          mouse;
    sf::Clock          gameClock;
    bool               isMovingUp, isMovingDown, startGame;
    float              xPlayerSpeed, yPlayerSpeed, lPaddleSpeed, rPaddleSpeed;
    int32_t            score, speedLimit;


};

Game::Game() : mWindow(sf::VideoMode(1280, 720), "Reverse Pong"), player(), leftPaddle(), rightPaddle(), 
               TimePerFrame(), isMovingUp(), isMovingDown(), xPlayerSpeed(), yPlayerSpeed(), 
               startGame(), speedLimit()
{
    speedLimit = 500;
    score = 0;
    TimePerFrame = sf::seconds(1.f / 240.f); // FPS

    // Player parameters
    player.setRadius(10.f);
    player.setFillColor(sf::Color::Green);
    player.setOutlineColor(sf::Color::White);
    player.setOutlineThickness(1);
    player.setPosition(45.f, 360.f);
    xPlayerSpeed = 300.f;
    yPlayerSpeed = 0.5f;

    // Paddle parameters
    leftPaddle.setSize(sf::Vector2f(10.f,100.f));
    leftPaddle.setFillColor(sf::Color::Blue);
    leftPaddle.setPosition(30.f, 360.f);
    lPaddleSpeed = -50.f;

    rightPaddle.setSize(sf::Vector2f(10.f, 100.f));
    rightPaddle.setFillColor(sf::Color::Blue);
    rightPaddle.setPosition(1250.f, 360.f);
    rPaddleSpeed = 50.f;
    
    // Text parameters
    font.loadFromFile("SoulDaisy.otf");
    ScoreText.setFont(font);
    ScoreText.setCharacterSize(50.f);
    ScoreText.setString(std::to_string(score));
    ScoreText.setFillColor(sf::Color::White);
    ScoreText.setPosition(620.f, 30.f);

    TitleText.setFont(font);
    TitleText.setCharacterSize(100.f);
    TitleText.setString("Reverse Pong");
    TitleText.setFillColor(sf::Color::White);
    TitleText.setPosition(300.f, 30.f);

    StartText.setFont(font);
    StartText.setCharacterSize(50.f);
    StartText.setString("START");
    StartText.setFillColor(sf::Color::White);
    StartText.setPosition(100.f, 350.f);
   
}

void Game::run() {

    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (mWindow.isOpen()) {

        while (!startGame) {
            processEvents(clock);
            render();
        }

        processEvents(clock);
        // clock.restart() returns the elapsed time since the clock LAST restarted.
        // Makes it an ideal class for measuring how long each frame takes (what we're doing here!).
        timeSinceLastUpdate += clock.restart();

        while (timeSinceLastUpdate > TimePerFrame) {

            timeSinceLastUpdate -= TimePerFrame;
            update(TimePerFrame);

        }

        ScoreText.setString(std::to_string(score));
        render();

    }

}

void Game::processEvents(sf::Clock clock) {

    sf::Event event;

    while (mWindow.pollEvent(event)) {

        switch (event.type) {

        case sf::Event::Closed:
            mWindow.close();
            break;
        case sf::Event::KeyPressed:
            handlePlayerInput(event.key.code, true);
            break;
        case sf::Event::KeyReleased:
            handlePlayerInput(event.key.code, false);
            break;

        case sf::Event::MouseButtonPressed:
            if    (std::abs(mouse.getPosition(mWindow).x) - 150 <= 50 // If mouse position is within START button & pressed.
                && std::abs(mouse.getPosition(mWindow).y) - 350 <= 50 
                && !startGame) {

                StartText.setFillColor(sf::Color{ 0x4e545e });
            }
            break;

        case sf::Event::MouseButtonReleased:
            if (StartText.getFillColor() == sf::Color{ 0x4e545e }) {

                StartText.setFillColor(sf::Color::White);
                mWindow.draw(StartText);
                Sleep(500); 
                startGame = true;
                resetGameAttributes();
            }
        }
    }
}

void Game::collisionDetection() {
    // Not sure what a FloatRect is.
    sf::FloatRect playerLocation = player.getGlobalBounds();
    sf::FloatRect leftPaddleLocation = leftPaddle.getGlobalBounds();
    sf::FloatRect rightPaddleLocation = rightPaddle.getGlobalBounds();

    // Player - Side Wall collisions.
    // Game over!
    if ((playerLocation.getPosition().x > 1270 || playerLocation.getPosition().x <= 10) && startGame) {
        startGame = false;
        resetGameAttributes();
        std::cout << "Game over!\n";
    }

    // Player - Top/Bottom Wall collisions.
    // Maybe make a border object so .intersects() can be used?
    if (playerLocation.getPosition().y > 700 || playerLocation.getPosition().y <= 0) {
        yPlayerSpeed *= -0.9;
        // Conditional positional change (680 * 1 if at bottom, 680 * 0 if at top).
        player.setPosition(playerLocation.getPosition().x, (playerLocation.getPosition().y > 700) * 680 + 10);
    }

    // Player - Paddle collisions.
    if (playerLocation.intersects(leftPaddleLocation) || playerLocation.intersects(rightPaddleLocation)) {
        // Changing paddle directions after collisions.
        if (xPlayerSpeed > 0) {
            rPaddleSpeed *= -1;
        }
        else {
            lPaddleSpeed *= -1;
        }
        // Changing player speed every collision.
        xPlayerSpeed *= -1.05;
        score += 1;
    }

    /* Paddle - Border collisions
    20 and 600 are edge coordinates (620 = 720-100 (length of rectangle)).
    Distance of 20 is arbitrary for now
    */
    if (leftPaddleLocation.getPosition().y < 20 || leftPaddleLocation.getPosition().y > 600) {
        lPaddleSpeed *= -1;
    }

    if (rightPaddleLocation.getPosition().y < 20 || rightPaddleLocation.getPosition().y > 600) {
        rPaddleSpeed *= -1;
    }


}

void Game::update(sf::Time deltaTime) {

    collisionDetection();
    float factor = 1.07; // For ease of testing values - will replace w/ constant
    sf::Vector2f playerMovement(xPlayerSpeed, yPlayerSpeed);

    // The longer the game goes, the faster you can go.
    if (gameClock.getElapsedTime().asMilliseconds() % 2000 <= 3 && 
        gameClock.getElapsedTime().asMilliseconds() > 2000) {

        speedLimit += 25;
    }

    if (isMovingUp) {
        // If switching directions, just dampen current speed.
        // Avoids rapid directional changes.
        if (yPlayerSpeed > 10) {
            playerMovement.y -= yPlayerSpeed * 0.2;
        }
        else if (yPlayerSpeed > -speedLimit) { // Ensuring always negative (Up is towards 0, i.e. negative speed).
            yPlayerSpeed = std::abs(yPlayerSpeed) * -factor;
            playerMovement.y += yPlayerSpeed;
        }
        else {
            yPlayerSpeed = -speedLimit;
            playerMovement.y += yPlayerSpeed;
        }

    }
    if (isMovingDown) {

        if (yPlayerSpeed < -10) {
            playerMovement.y -= yPlayerSpeed * 0.2;
        }
        else if (yPlayerSpeed < speedLimit) {
            yPlayerSpeed = std::abs(yPlayerSpeed) * factor;
            playerMovement.y += yPlayerSpeed;
        }
        else {
            yPlayerSpeed = speedLimit;
            playerMovement.y += yPlayerSpeed;
        }
  
    }

    // Speed Decay
    if (yPlayerSpeed >= 1 or yPlayerSpeed < -1) {
        yPlayerSpeed /= 1.01;
    }
    
    leftPaddle.move(0.f, lPaddleSpeed * deltaTime.asSeconds());
    rightPaddle.move(0.f, rPaddleSpeed * deltaTime.asSeconds());
    player.move(playerMovement * deltaTime.asSeconds());
    
}

void Game::render() {
    
    if (startGame) {
        mWindow.clear(sf::Color::Black);
        mWindow.draw(player);
        mWindow.draw(leftPaddle);
        mWindow.draw(rightPaddle);
        mWindow.draw(ScoreText);
    }
    else {
        mWindow.clear(sf::Color::Magenta);
        mWindow.draw(TitleText);
        mWindow.draw(StartText);
    }
    mWindow.display();

}

void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed) {
    switch (key) {

    case sf::Keyboard::S:
        isMovingDown = isPressed;
        break;
    case sf::Keyboard::W:
        isMovingUp = isPressed;
        break;
   
    }
}

void Game::resetGameAttributes() {
    // For starting/restarting game
    player.setPosition(45.f, 360.f);
    xPlayerSpeed = 300.f;
    yPlayerSpeed = 0.5f;

    leftPaddle.setPosition(30.f, 360.f);
    lPaddleSpeed = -50.f;

    rightPaddle.setPosition(1250.f, 360.f);
    rPaddleSpeed = 50.f;

    speedLimit = 500;
    score = 0;
    gameClock.restart();
}

int main() {
   
    Game game;
    // We're able to use the run() function as it exists in the public scope of the Game class.
    game.run();
    
    return 0;
}