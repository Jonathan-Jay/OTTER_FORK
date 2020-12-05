#include "Rendering.h"
#include "Components/Player.h"
#include "Components/Bullet.h"
#include "Components/Body.h"

namespace Rendering {

    void Update(entt::registry* reg, int numOfCams, float dt)
    {
        glClearColor(BackColour.x, BackColour.y, BackColour.z, BackColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto& objView = reg->view<ObjLoader, Transform>();
        auto& morphView = reg->view<ObjMorphLoader, Transform>();
        auto& cameraView = reg->view<Camera, Transform>();

        int height = BackEnd::GetHalfHeight();
        int width = BackEnd::GetHalfWidth();
        short count = 1;
        for (auto cam : cameraView)
        {
            //glViewport((count % 2 == 0 ? width : 0), ((count < 3) && (numOfCams > 2) ? height : 0),
            //    width * (numOfCams == 1 ? 2 : 1), height * (numOfCams > 2 ? 1 : 2));

            if (numOfCams > 2)
                glViewport((count % 2 == 1 ? 0 : width), (count < 3 ? height : 0), width, height);
            else if (numOfCams == 2)
                glViewport((count == 1 ? 0 : width), 0, width, height * 2);
            else
                glViewport(0, 0, width * 2, height * 2);

            Transform& camTrans = cameraView.get<Transform>(cam);
            glm::mat4 view = glm::inverse(camTrans.GetModel());

            Camera& camCam = cameraView.get<Camera>(cam);

            if (massDraw)
                ObjLoader::BeginDraw();

            //draw all the objs
            for (auto entity : objView)
            {
                Transform& trans = objView.get<Transform>(entity);

                if (!massDraw) {
                    if (reg->has<Bullet>(entity))
                        DefaultColour = (reg->get<Player>(reg->get<Bullet>(entity).getShooter()).
                            isPlayer1() ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1));
                    else if (reg->has<Player>(entity))
                        DefaultColour = (reg->get<Player>(entity).isPlayer1() ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1));
                    else if (DefaultColour != glm::vec3(1.f))
                        if (reg->has<Collisions>(entity))
                            if (reg->get<Collisions>(entity).isDead())
                                DefaultColour = glm::vec3(0.75f);

                    objView.get<ObjLoader>(entity).DrawSkipQueue(trans.GetModel(), view, camCam, DefaultColour,
                        LightPos1, LightPos2, LightColour, 1, 4, 0.25f);
                }
                else {
                    if (reg->has<Body>(entity)) {
                        reg->get<Body>(entity).Draw();
                        objView.get<ObjLoader>(entity).DrawSkipQueue(trans.GetModel(), view, camCam, glm::vec3(1.f, 0.5f, 0.5f),
                            LightPos1, LightPos2, LightColour, 1, 2, 0.75f);
                    }
                    else
                        objView.get<ObjLoader>(entity).Draw(trans.GetModel());
                }
            }

            if (massDraw)
                ObjLoader::PerformDraw(view, camCam, DefaultColour, LightPos1, LightPos2, LightColour, 1, 2, 0.75f);

            ObjMorphLoader::BeginDraw();

            //draw all the objs
            for (auto entity : morphView)
            {
                Transform& trans = morphView.get<Transform>(entity);

                if (count == 1)
                    morphView.get<ObjMorphLoader>(entity).Update(dt);
                morphView.get<ObjMorphLoader>(entity).Draw(trans.GetModel());
            }

            ObjMorphLoader::PerformDraw(view, camCam, DefaultColour, LightPos1, LightPos2, LightColour, 1, 4, 0.75f);

            ++count;
            //exit even if some cams haven't been checked, because only the amount specified should render
            if (count > numOfCams)
                break;
        }
    }

    glm::vec4 BackColour = { 0.2f, 0.2f, 0.2f, 1.f };
    glm::vec3 LightColour = glm::vec3(3.f);
    glm::vec3 LightPos1 = glm::vec3(0.f);
    glm::vec3 LightPos2 = glm::vec3(0.f);
    glm::vec3 DefaultColour = glm::vec3(1.f);

    bool massDraw = false;
}