#include "MainGame.h"
#include <ryEngine/Errors.h>
#include <ryEngine/ResourceManager.h>

#include <iostream>
#include <string>

MainGame::MainGame() :
    _screenWidth(1024),
    _screenHeight(768),
    _gameState(GameState::PLAY),
    _maxFPS(60.0f),
    _time(0.0f)
{
    _camera.init(_screenWidth, _screenHeight);
}

MainGame::~MainGame()
{

}

void MainGame::run()
{
    initSystems();

    gameLoop();
}

void MainGame::initSystems()
{
    ryEngine::init();

    _window.create("Game Engine", _screenWidth, _screenHeight, 0);

    initShaders();

    _spriteBatch.init();
    _fpsLimiter.init(_maxFPS);
}

void MainGame::initShaders()
{
    _colorProgram.compileShaders("colorShading.vert", "colorShading.frag");
    _colorProgram.addAttribute("vertexPosition");
    _colorProgram.addAttribute("vertexColor");
    _colorProgram.addAttribute("vertexUV");
    _colorProgram.linkShaders();
}

void MainGame::gameLoop()
{
    while ( _gameState != GameState::EXIT) {
        _fpsLimiter.begin();

        processInput();
        _time += 0.01;

        _camera.update();

        for (int i = 0; i < _bullets.size();)
        {
            // Update all bullets
            if (_bullets[i].update() == true)
            {
                _bullets[i] = _bullets.back();
                _bullets.pop_back();
            }
            else
            {
                i++;
            }

        }

        drawGame();

        _fps = _fpsLimiter.end();

        //print only once every 10 frames
        static int frameCounter = 0;
        frameCounter++;
        if(frameCounter == 10000)
        {
            std::cout << _fps << std::endl;
            frameCounter = 0;
        }


    }
}

void MainGame::processInput()
{
    SDL_Event evnt;

    const float CAMERA_SPEED = 2.0f;
    const float SCALE_SPEED = 0.1f;

    while(SDL_PollEvent(&evnt))
    {
        switch (evnt.type)
        {
            case SDL_QUIT:
                _gameState = GameState::EXIT;
                break;
            case SDL_MOUSEMOTION:
                _inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
                break;
            case SDL_KEYDOWN:
                _inputManager.pressKey(evnt.key.keysym.sym);
                break;
            case SDL_KEYUP:
                _inputManager.releaseKey(evnt.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                _inputManager.pressKey(evnt.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                _inputManager.releaseKey(evnt.button.button);
                break;
        }
    }

    if (_inputManager.isKeyPressed(SDLK_w))
            { _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, CAMERA_SPEED)); }
    if (_inputManager.isKeyPressed(SDLK_s))
            { _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, -CAMERA_SPEED)); }
    if (_inputManager.isKeyPressed(SDLK_a))
            { _camera.setPosition(_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f)); }
    if (_inputManager.isKeyPressed(SDLK_d))
            { _camera.setPosition(_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f)); }
    if (_inputManager.isKeyPressed(SDLK_q))
            { _camera.setScale(_camera.getScale() + SCALE_SPEED); }
    if (_inputManager.isKeyPressed(SDLK_e))
            { _camera.setScale(_camera.getScale() - SCALE_SPEED); }

    if (_inputManager.isKeyPressed(SDL_BUTTON_LEFT))
    {
        glm::vec2 mouseCoords = _inputManager.getMouseCoords();
        mouseCoords = _camera.convertScreenToWorld(mouseCoords);
        std::cout << mouseCoords.x << " " << mouseCoords.y << std::endl;

        glm::vec2 playerPosition(0.0f);
        glm::vec2 direction = mouseCoords - playerPosition;
        direction = glm::normalize(direction);

        _bullets.emplace_back(playerPosition, direction, 1.25f, 10000);
    }
    if (_inputManager.isKeyPressed(SDLK_z))
    {
        _window.changeWinBackGround(0.0f, 0.6f, 0.0f, 0.0f);
    }

}

void MainGame::drawGame()
{
    //Set the base depth to 1.0
    glClearDepth(1.0);
    //Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Enable the shader
    _colorProgram.use();

    //Using texture unit 0
    glActiveTexture(GL_TEXTURE0);
    //Get the uniform location
    GLint textureLocation = _colorProgram.getUniformLocation("mySampler");
    //Tell the shader that the texture is in texture unit 0
    glUniform1i(textureLocation, 0);

    //R.I.P. "time" for the shader

    //Set the camera matrix
    GLint pLocation = _colorProgram.getUniformLocation("P");
    glm::mat4 cameraMatrix = _camera.getCameraMatrix();

    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

    _spriteBatch.begin();

    glm::vec4 pos(0.0f, 0.0f, 50.0f, 50.0f);
    glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
    static ryEngine::GLTexture texture = ryEngine::ResourceManager::getTexture("Textures/WaltFace.png");
    ryEngine::Color color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    _spriteBatch.draw(pos, uv, texture.id, 0.0f, color);

    for (int i = 0; i < _bullets.size(); i++)
    {
        _bullets[i].draw(_spriteBatch);
    }

    _spriteBatch.end();

    _spriteBatch.renderBatch();

    glBindTexture(GL_TEXTURE_2D, 0);

    _colorProgram.unuse();

    //Swap our buffer and draw everything to the screen!
    _window.swapBuffer();
}
