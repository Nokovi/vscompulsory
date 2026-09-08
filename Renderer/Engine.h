#ifndef ENGINE_H
#define ENGINE_H

#include "Renderer.h"
#include "Input.h"
#include <vector>

class Terrain;

// Holds the meshes and textures for the scene
// Also holds the main Game Loop
class Engine
{
Input* mInput;
public:

    // Getting the instance of this class. This class is a singleton!
    static Engine* getInstance();

    // Main Game Loop update function
    void update();

    void loadScene();

    //Getters
    std::vector<Mesh *> meshes() const;
    std::vector<Texture *> textures() const;

    void setRenderer(Renderer *rendererIn);
    void setMainWindow(MainWindow *mainWindowIn);
    void setInput(Input *inputIn){mInput = inputIn;};


    Terrain* mTerrain{ nullptr };

    GameObject* mPlayer{ nullptr };

    std::vector<GameObject*> mCollectibles;
    uint8_t collected_items = 0;

    GameObject* Enemy;


private:
    Engine();
    ~Engine() = default;




    // Delete copy constructor and assignment operator
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void readTexture(std::string textureName);

    Renderer* mRenderer{ nullptr };
    MainWindow* mMainWindow { nullptr };



    std::vector<class Mesh*> mMeshes;
    std::vector<class Texture*> mTextures;
};

#endif // ENGINE_H
