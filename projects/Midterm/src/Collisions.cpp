#include "Collisions.h"
#include "Transform.h"
#include "Player.h"
#include "Bullet.h"
#include "ObjLoader.h"

glm::vec2 Collisions::BottomLeft = glm::vec2(0.f);
glm::vec2 Collisions::TopRight = glm::vec2(0.f);
glm::vec2 Collisions::Gravity = glm::vec3(0.f);

Collisions::Collisions(float radius, bool isDynamic, bool isBreakable)
{
    Init(radius, isDynamic, isBreakable);
}

Collisions::Collisions(float w, float h, bool isDynamic, bool isBreakable)
{
    Init(w, h, isDynamic, isBreakable);
}

void Collisions::Init(float radius, bool isDynamic, bool isBreakable)
{
    isCircle = true;
    isStatic = !isDynamic;
    breakable = isBreakable;
    width = radius;
    height = radius;
}

void Collisions::Init(float w, float h, bool isDynamic, bool isBreakable)
{
    isCircle = false;
    isStatic = !isDynamic;
    breakable = isBreakable;
    width = w / 2.f;
    height = h / 2.f;
}

void Collisions::SetWorldBounds(glm::vec2 BL, glm::vec2 TR, glm::vec2 g)
{
    if (BL.x < TR.x && BL.y < TR.y) {
        BottomLeft = BL;
        TopRight = TR;
    }
    Gravity = g;
}

void Collisions::Update(entt::registry* reg, float deltaTime)
{
    if (BottomLeft == TopRight)
        return;

    //bullets collision
    auto bulletView = reg->view<Transform, Collisions, Bullet>();

    //general collisions
    auto view = reg->view<Transform, Collisions>();

    for (auto entity : bulletView) {
        auto & coll = bulletView.get<Collisions>(entity);
        if (coll.dead) {
            reg->remove<Bullet>(entity);
            continue;
        }

        auto& trans = bulletView.get<Transform>(entity);
        Bullet &bull = bulletView.get<Bullet>(entity);

        float radius = coll.width;
        glm::vec3 pos = trans.SetPosition(trans.GetPosition() + bull.getMovement() * deltaTime).GetPosition();
        int bounce = 1;
        if (pos.z < BottomLeft.y + radius) {
            pos.z = BottomLeft.y + radius;
            bounce = bull.FlipZ();
        }
        if (pos.z > TopRight.y - radius) {
            pos.z = TopRight.y - radius;
            bounce = bull.FlipZ();
        }
        if (pos.x < BottomLeft.x + radius) {
            pos.x = BottomLeft.x + radius;
            bounce = bull.FlipX();
        }
        if (pos.x > TopRight.x - radius) {
            pos.x = TopRight.x - radius;
            bounce = bull.FlipX();
        }

        if (bounce <= 0) {
            reg->get<Player>(bull.getShooter()).bulletDead();
            reg->remove<Bullet>(entity);
            coll.dead = true;
            coll.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
            continue;
        }

        glm::vec3 tempPos = pos;
        //calculating with all other objects
        for (auto entity2 : view) {
            if (entity == entity2)  //ignore itself
                continue;

            auto& coll2 = view.get<Collisions>(entity2);

            if (coll2.dead)         //ignore dead things
                continue;

            auto& trans2 = view.get<Transform>(entity2);
            glm::vec3 pos2 = trans2.GetPosition();

            //all bullets are circles so all circle smt test
            if (coll2.isCircle) {   //circle circle
                glm::vec3 diff = pos - pos2;
                float dist = glm::length(diff);
                float distance = radius + coll2.width;
                if (dist < distance) {
                    //did it hit a player?
                    if (reg->has<Player>(entity2)) {
                        reg->get<Player>(entity2).die();
                        bounce = 0;
                        break;
                    }
                    else {
                        glm::vec3 norm = glm::normalize(diff);
                        tempPos = pos2 + norm * distance;
                        bounce = bull.Reflect(norm);
                        if (coll2.breakable) {
                            reg->get<ObjLoader>(entity2).LoadMesh("models/Pillar.obj");
                            coll2.dead = true;
                            coll2.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
                        }
                        break;
                    }
                }
            }
            else {      //box circle, but since all boxes are static, no mid calc
                float radw = coll2.width + radius;
                float radh = coll2.height + radius;

                //all sides of box
                bool hori = false, verti = false;
                if (pos2.x - radw < pos.x && pos2.x + radw > pos.x &&
                    pos2.z - radh < pos.z && pos2.z + radh > pos.z) {
                    //is it aligned on the x?
                    if (pos2.x - coll2.width < pos.x && pos2.x + coll2.width > pos.x) {
                        tempPos.z = pos2.z + (pos.z > pos2.z ? radh : -radh);
                        hori = true;
                    }
                    if (pos2.z - coll2.height < pos.z && pos2.z + coll2.height > pos.z) {
                        tempPos.x = pos2.x + (pos.x > pos2.x ? radw : -radw);
                        verti = true;
                    }
                    //change pos?
                    if (!hori && !verti) {
                        float sum = sqrtf(coll2.width * coll2.width + coll2.height * coll2.height) + radius;
                        glm::vec3 dist = pos - pos2;
                        float distance = glm::length(dist);
                        if (sum > distance) {
                            //just flip on Z because no horizontal walls, less weird stuff
                            /*tempPos.z = pos2.z + (pos.z > pos2.z ? radh : -radh);
                            tempPos.x = pos2.x + (pos.x > pos2.x ? radw : -radw);
                            bounce = bull.FlipZ();
                            bounce = bull.FlipX();*/

                            //more accurate, but more annoying ver
                            glm::vec3 norm = glm::normalize(dist);
                            bounce = bull.Reflect(norm);
                            trans.SetPosition(pos2 + norm * sum * 1.2f);
                            if (coll2.breakable) {
                                reg->get<ObjLoader>(entity2).LoadMesh("models/Cube.obj");
                                coll2.dead = true;
                                coll2.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
                            }
                            break;
                        }

                    }
                    else if (hori && verti) {
                        bounce = 0;
                        if (coll2.breakable) {
                            reg->get<ObjLoader>(entity2).LoadMesh("models/Cube.obj");
                            coll2.dead = true;
                            coll2.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
                        }
                        break;
                    }
                    else if (hori) {
                        bounce = bull.FlipZ();
                        if (coll2.breakable) {
                            reg->get<ObjLoader>(entity2).LoadMesh("models/Cube.obj");
                            coll2.dead = true;
                            coll2.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
                        }
                        break;
                    }
                    else if (verti) {
                        bounce = bull.FlipX();
                        if (coll2.breakable) {
                            reg->get<ObjLoader>(entity2).LoadMesh("models/Cube.obj");
                            coll2.dead = true;
                            coll2.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
                        }
                        break;
                    }
                }
            }
        }

        if (bounce <= 0) {
            reg->get<Player>(bull.getShooter()).bulletDead();
            reg->remove<Bullet>(entity);
            coll.dead = true;
            coll.velo = glm::vec2(rand() % 21 - 10.f, rand() % 11 + 5);
            break;
        }

        if (bounce != 0) {
            trans.SetPosition(tempPos);
        }
    }
    
    for (auto entity : view) {
        if (reg->has<Bullet>(entity))   //dont update bullets again
            continue;
        auto& coll = view.get<Collisions>(entity);
        auto& trans = view.get<Transform>(entity);

        if (coll.dead) {                //dead things do smt different
            glm::vec3 pos1 = trans.GetPosition();
            pos1 = trans.SetPosition(
                glm::vec3((coll.velo.x) * deltaTime + pos1.x, 1.25f, (coll.velo.y) * deltaTime + pos1.z)
            ).GetPosition();
            coll.velo += Gravity * deltaTime;
            if (pos1.z < BottomLeft.y - 5.f) {
                reg->destroy(entity);
            }
            else if (pos1.z > TopRight.y + 5.f) {
                reg->destroy(entity);
            }
            else if (pos1.x < BottomLeft.x - 5.f) {
                reg->destroy(entity);
            }
            else if (pos1.x > TopRight.x + 5.f) {
                reg->destroy(entity);
            }
            continue;
        }

        if (coll.isStatic)              //don't update static objects that aren't dead
            continue;


        // calculating with world walls
        glm::vec3 pos = trans.GetPosition();
        float h = coll.height;
        float w = coll.width;
        if (pos.z < BottomLeft.y + h) {
            pos.z = BottomLeft.y + h;
        }
        if (pos.z > TopRight.y - h) {
            pos.z = TopRight.y - h;
        }
        if (pos.x < BottomLeft.x + w) {
            pos.x = BottomLeft.x + w;
        }
        if (pos.x > TopRight.x - w) {
            pos.x = TopRight.x - w;
        }

        //calculating with all other objects
        glm::vec3 tempPos = pos;
        for (auto entity2 : view) {
            if (entity == entity2 || reg->has<Bullet>(entity2))
                continue;

            auto& coll2 = view.get<Collisions>(entity2);
            if (coll2.dead)
                continue;

            auto& trans2 = view.get<Transform>(entity2);
            glm::vec3 pos2 = trans2.GetPosition();

            //all non static things are circles so all circle smt test
            if (coll2.isCircle) {   //circle circle
                glm::vec3 diff = tempPos - pos2;
                float dist = glm::length(diff);
                float distance = coll.width + coll2.width;
                if (dist < distance) {
                    glm::vec3 norm = glm::normalize(diff);
                    if (!coll2.isStatic) {      //in case i add pillars
                        glm::vec3 mid = pos2 + norm * dist / 2.f;
                        tempPos = mid + norm * coll.width;
                        pos2 = mid - norm * coll2.width;
                        trans2.SetPosition(pos2);
                    }
                    else {
                        tempPos = pos2 + norm * distance;
                    }
                }
            }
            else {      //box circle, but since all boxes are static, no mid calc
                float radw = coll2.width + coll.width;
                float radh = coll2.height + coll.width;

                //all sides of box
                //bool hori = false, verti = false;
                if (pos2.x - radw < tempPos.x && pos2.x + radw > pos.x &&
                    pos2.z - radh < pos.z && pos2.z + radh > pos.z) {
                    glm::vec3 dist = tempPos - pos2;
                    float sum = sqrtf(coll2.width * coll2.width + coll2.height * coll2.height) + coll.width;
                    if (sum > glm::length(dist)) {
                        if (fabsf(dist.x) > fabsf(dist.z))
                            tempPos.x = pos2.x + (tempPos.x > pos2.x ? radw : -radw);
                        else
                            tempPos.z = pos2.z + (tempPos.z > pos2.z ? radh : -radh);
                    }
                }
            }
        }
        trans.SetPosition(tempPos);
    }
}

