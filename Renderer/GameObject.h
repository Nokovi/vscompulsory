#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

class GameObject
{
public:
    GameObject() {};

    struct Transform* mTransform{nullptr};
    int mMesh{0};
    int mTexture{0};
	class Collider* mCollider{ nullptr };
};

#endif // GAMEOBJECT_H
