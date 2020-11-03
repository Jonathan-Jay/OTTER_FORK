#include "Player.h"
#include "ECS.h"
#include "ObjLoader.h"
#include "Bullet.h"

bool Player::Shoot(float bulletSpeed, glm::quat rotation, glm::vec3 pos)
{
    if (bulletId != 0)
        return false;
    
    bulletId = ECS::CreateEntity();
    ECS::AttachComponent<ObjLoader>(bulletId).LoadMesh("models/Ball.obj");
    ECS::AttachComponent<Bullet>(bulletId).Init(rotation, bulletSpeed, 3, playerId);
    ECS::GetComponent<Transform>(bulletId).SetPosition(pos + glm::rotate(rotation, glm::vec3(0, 0, 0.75f)));
    ECS::AttachComponent<Collisions>(bulletId).Init(0.25f, true, false);

    return true;
}
