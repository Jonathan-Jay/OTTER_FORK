#include "Rendering.h"
#include "Bullet.h"

namespace Rendering {

    void Update(entt::registry* reg)
    {
        glClearColor(BackColour.x, BackColour.y, BackColour.z, BackColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto objView = reg->view<ObjLoader, Transform>();
        auto cameraView = reg->view<Camera, Transform>();

        for (auto cam : cameraView)
        {
            cameraView.get<Camera>(cam).SetPosition(cameraView.get<Transform>(cam).GetPosition());
            cameraView.get<Camera>(cam).SetRotation(cameraView.get<Transform>(cam).GetRotation());

            Camera& camCam = cameraView.get<Camera>(cam);

            //draw all the objs
            for (auto entity : objView)
            {
                Transform& trans = objView.get<Transform>(entity);

                //colour bullets according to owner
                if (reg->has<Bullet>(entity))
                    DefaultColour = (reg->get<Player>(reg->get<Bullet>(entity).getShooter()).
                        isPlayer1() ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1));
                else if (reg->has<Player>(entity))
                    DefaultColour = (reg->get<Player>(entity).isPlayer1() ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1));
                else if (DefaultColour != glm::vec3(1.f))
                    if (reg->has<Collisions>(entity))
                        if (reg->get<Collisions>(entity).isDead())
                            DefaultColour = glm::vec3(0.5f);

                objView.get<ObjLoader>(entity).Draw(camCam, trans.GetModel(), trans.GetRotationM3(),
                    DefaultColour, LightPos1, LightPos2, LightColour, 1, 128, 0.25f);
            }
        }
    }

    glm::vec4 BackColour = { 0.2f, 0.2f, 0.2f, 1.f };
    glm::vec3 LightColour = glm::vec3(3.f);
    glm::vec3 LightPos1 = glm::vec3(0.f);
    glm::vec3 LightPos2 = glm::vec3(0.f);
    glm::vec3 DefaultColour = glm::vec3(1.f);

}