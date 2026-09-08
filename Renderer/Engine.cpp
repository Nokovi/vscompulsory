#include "Engine.h"
#include "MainWindow.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "GameObject.h"
#include "Transform.h"
#include "Terrain.h"
#include "Collider.h"
#include "Renderer.h"
#include "Logger.h"
#include <cstdlib>

Engine::Engine()
{}

void Engine::loadScene()
{
    std::srand(8492);

    //Load textures and meshes here!
    mMeshes.push_back(new Mesh(mRenderer));
    mMeshes.push_back(new Mesh(mRenderer, Mesh::MeshType::QUAD)); // Second mesh with an offset


    GameObject* temp;

    for(int i = 0; i < 20; i++){
        temp = new GameObject();
        temp->mTransform = new Transform{  glm::vec3(std::rand() % 20, std::rand() % 20, std::rand() % 20) };
        temp->mTransform->scale = glm::vec3(0.3f, 0.3f, 0.3f);
        temp->mMesh = 1; //Monkey
        temp->mTexture = 0;
        temp->mCollider = new Collider(0.2f, temp);// A collider for our NPC
        mRenderer->mGameObjects.push_back(temp);
        mCollectibles.push_back(mRenderer->mGameObjects.back()); //This is our NPC!
    }


}

void Engine::readTexture(std::string textureName)
{
    // Texture update: - textures must be made before the Descriptor sets!
    Texture* newTexture = new Texture(textureName);
    mRenderer->createTextureImage(newTexture);
    mRenderer->createTextureImageView(newTexture);
    mTextures.push_back(newTexture);
}

//lazy global declarations. who cares.
int engineloopi = 0; //simple timer.

// Main Game Loop function
// called every frame
void Engine::update()
{
    engineloopi++;

    //we are implementing this super lazily.



    // 1. get input from mouse and keyboard
    mMainWindow->handleInput();

    //Hacky input handling in update loop.

    glm::vec3 playerRotation = mPlayer->mTransform->rotation;

    Input control = *mInput;



    float rotX = control.MOUSEY / 10.f;
    float rotY = -control.MOUSEX / 10.f;

    playerRotation.y = rotY + 180.f;


    mPlayer->mTransform->rotation = playerRotation;



    glm::vec3 playerPos = mPlayer->mTransform->position;


    // 2. update game objects
    // physics calculations
    // etc.

	//NPC movement

    /*

	float xMax = 12.f;
	float xMin = 0.f;
	static bool movingRight = true;
	float moveSpeed = 0.01f;
    if (movingRight)
    {
        Enemy->mTransform->position.x += moveSpeed;
        if (Enemy->mTransform->position.x >= xMax)
            movingRight = false;
    }
    else
    {
        Enemy->mTransform->position.x -= moveSpeed;
        if (Enemy->mTransform->position.x <= xMin)
            movingRight = true;
	}

    */

    //for third person camera boom.
    glm::vec3 cameraOffset(0.f, 2.f, 5.f);

    float rad = M_PI / 180.f;

    glm::mat3x3 rotation = {std::cos(-rotY*rad), 0.f, std::sin(-rotY*rad),
                            0.f, 1.f, 0.f,
                            -std::sin(-rotY*rad), 0, std::cos(-rotY*rad) };
    cameraOffset = rotation * cameraOffset;
    glm::vec3 cameraBase = playerPos + cameraOffset;
    mRenderer->mCamera->mPosition = cameraBase;

    //camera angle.
    mRenderer->mCamera->mPitch = -10.0f;
    mRenderer->mCamera->mYaw = rotY;


    //collider


    /*
    for(int i = 0; i < 20; i++){
        if(mPlayer->mCollider->isColliding(*mCollectibles[i]->mCollider)){
            mRenderer->mLogger.logText("Collected!",Logger::LogType::LOG);

            mCollectibles[i]->mTransform->position.y = -1000; //teleport under map for simplicity.
            collected_items++;

            if(collected_items == 20){
                mRenderer->mLogger.logText("YOU WIN!",Logger::LogType::HIGHLIGHT);
            }

        }

    }

    */

    /*
    if(mPlayer->mCollider->isColliding(*Enemy->mCollider)){
        mRenderer->mLogger.logText("YOU LOSE!",Logger::LogType::WARNING);
        playerPos.x = 1000.f; //punish player by teleporting them into the void.
    }
    */


    // 3. call the renderer to draw a frame

    //Update player position.


    if(control.W){
        playerPos.x -= 0.02f * sin(rotY*rad);
        playerPos.z -= 0.02f * cos(rotY*rad);
    }
    if(control.S){
        playerPos.x += 0.02f * sin(rotY*rad);
        playerPos.z += 0.02f * cos(rotY*rad);
    }
    if(control.A){
        playerPos.x -= 0.02f * sin(rotY*rad+90*rad);
        playerPos.z -= 0.02f * cos(rotY*rad+90*rad);
    }
    if(control.D){
        playerPos.x += 0.02f * sin(rotY*rad+90*rad);
        playerPos.z += 0.02f * cos(rotY*rad+90*rad);
    }

    if (mTerrain)
    {
        playerPos.y = mTerrain->heightAtPoint(glm::vec2(playerPos.x, playerPos.z)) + 0.3f; // add 0.3 to put the player slightly above the terrain
        //npcPos.y = mTerrain->heightAtPoint(glm::vec2(npcPos.x, npcPos.z)) + 0.3f; // add 0.3 to put the player slightly above the terrain


    }

    mPlayer->mTransform->position = playerPos;






    mRenderer->update();





}

Engine* Engine::getInstance()
{
    static Engine mInstance;
    return &mInstance;
}

std::vector<Mesh *> Engine::meshes() const
{
    return mMeshes;
}

std::vector<Texture *> Engine::textures() const
{
    return mTextures;
}

void Engine::setRenderer(Renderer *rendererIn)
{
    mRenderer = rendererIn;
}

void Engine::setMainWindow(MainWindow *mainWindowIn)
{
        mMainWindow = mainWindowIn;
}
