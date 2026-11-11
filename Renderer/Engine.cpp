#include "Engine.h"
#include "MainWindow.h"
#include "Mesh.h"
#include "Texture.h"

#include "GameObject.h"
#include "Transform.h"
#include "Terrain.h"
#include "Collider.h"
#include "Renderer.h"
#include "Logger.h"
#include <cstdlib>


Engine::Engine() : mLogger(Logger::getInstance())
{
}

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

bool bBallInAir = true;

//Triangle position. Mainly for testing.
glm::vec2 P1 = {0.0f, 0.0f}; float P1h = 0.f;
glm::vec2 P2 = {30.0f, 0.0f}; float P2h = 0.f;
glm::vec2 P3 = {0.0f, 30.0f}; float P3h = 5.f;

float BallRadius = 1.f;
float BallMass = 1.f;
glm::vec3 BallVelocity{0.f,0.f,0.f};


float DeltaTime = 0.001f; //I'll calculate DeltaTime from process time at a later date.
//Right now each frame simulates 1/1000th of a second.

// Main Game Loop function
// called every frame
void Engine::update()
{

    // 1. get input from mouse and keyboard
    mMainWindow->handleInput();

    //Hacky input handling in update loop.

    glm::vec3 playerRotation = mPlayer->mTransform->rotation;

    Input control = *mInput;
    glm::vec3 playerPos = mPlayer->mTransform->position;

    glm::vec3 Bary = Mesh::barycentricCoordinates(P1, P2, P3, glm::vec2(playerPos.x, playerPos.z));

    if(Bary.x >= 0 && Bary.x <= 1 &&
        Bary.y >= 0 && Bary.x <= 1 &&
        Bary.z >= 0 && Bary.z <= 1){


        glm::vec3 p12 = {P1 - P2, P1h - P2h};
        glm::vec3 p13 = {P1 - P3, P1h - P3h};
        glm::vec3 n = glm::cross(glm::vec3(p12), glm::vec3(p13)); // p12 ^ p13;

        n = glm::normalize(n);

        glm::vec3 F = glm::vec3(n.x * n.z * 9.81, (pow(n.z, 2.f) - 1.f)*9.81, n.y * n.z * 9.81);

        float height = 0.0f;
        height += P1h * Bary.x;
        height += P2h * Bary.y;
        height += P3h * Bary.z;


        if(bBallInAir == true){

            if(playerPos.y - BallRadius <= height){
                playerPos.y = height + BallRadius;
                BallVelocity = glm::vec3{0.f,0.f,0.f}; // temp. Make better till next time.
                bBallInAir = false;

                LOGP("Normals: %f, %f, %f.", n.x, n.y, n.z);




            } else {
                F = {0.f, -9.81, 0.f};
            }
        }

        if (bBallInAir == false){

        }
        playerPos += glm::vec3(BallVelocity.x * DeltaTime, BallVelocity.y * DeltaTime, BallVelocity.z * DeltaTime) +
                     glm::vec3(F.x * 0.5 * pow(DeltaTime, 2.f),  F.z * 0.5 * pow(DeltaTime, 2.f),F.y * 0.5 * pow(DeltaTime, 2.f)); // 9.14, swizzled.

        BallVelocity += glm::vec3(F.x * DeltaTime, F.y * DeltaTime, F.z * DeltaTime);

    } else {
        bBallInAir = true;
        glm::vec3 F{0.f, -9.81f, 0.f};

        playerPos += glm::vec3(BallVelocity.x * DeltaTime, BallVelocity.y * DeltaTime, BallVelocity.z * DeltaTime) +
                     glm::vec3(F.x * 0.5 * pow(DeltaTime, 2.f),  F.z * 0.5 * pow(DeltaTime, 2.f),F.y * 0.5 * pow(DeltaTime, 2.f)); // 9.14, swizzled.

        BallVelocity += glm::vec3(F.x * DeltaTime, F.y * DeltaTime, F.z * DeltaTime);
    }





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
