#pragma once

#include <map>
#include <iostream>
#include <string>
#include "components/simple_scene.h"
#include "components/transform.h"
#include "lab_m1/lab5/lab_camera.h"
#define Z_FAR		(100.f)
#define Z_NEAR		(.01f)

namespace m1
{
    class Lab5 : public gfxc::SimpleScene
    {
    public:
        Lab5();
        ~Lab5();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix) override;
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1));
        int GetRandomNumberInRange(const int minInclusive, const int maxInclusive);
        bool CollidesWith2D(float x1, float z1, float x2, float z2, float size1, float size2);

        bool CollidesWithBuilding(glm::vec3 tankCoords, glm::vec3 buildingCoords, glm::vec3 tankSize, glm::vec3 buildingSize);




        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        implemented::Camera* camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
        bool projectionType;

        GLfloat right;
        GLfloat left;
        GLfloat bottom;
        GLfloat top;
        GLfloat fov;
        glm::vec3 myTankPosition;
        float fixedHeight;
        float lastFireTime;
        float fireCooldown;
        float stateTime;
        glm::vec3 lightPosition;
        glm::vec3 lightDirection;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;

        GLint typeOfLight;
        GLfloat angleOX, angleOY;
        GLfloat cutoffAngle;
        glm::vec3 center1;

        // TODO(student): If you need any other class variables, define them here.

        class Building
        {
        public:
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 size;
        };

        std::vector<Building> buildings;

        enum class MovementState
        {
            GoingForward,
            GoingBackward,
            InPlaceRotationLeft,
            InPlaceRotationRight
        };
        
        MovementState GetNextMovementState(const MovementState currentState);
        std::string GetMovementStateName(const MovementState state);

        class Tank
        {
        public:
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 center;
            glm::vec3 size;
            bool isAlive;
            float health;
            bool isAttacking;
            float upperTurretAngle;
            float upperBodyAngle;
            float angle;
            implemented::Camera* camera;
            MovementState movementState;

           

            void Tank::Move(float distance)
            {
                // Update tank position without considering camera adjustments
                glm::vec3 dir = glm::normalize(glm::vec3(position.x, 0, position.z));
                position += dir * distance;
                position.y = size.y / 2;  // Ensure tank stays at ground level
            }
            void Tank::setTankAngle(float angle)
            {
				this->angle = angle;
			}
            void Tank::RotateUpperTurretOY(float angle)
            {
                upperTurretAngle += angle;
            }

            void Tank::RotateUpperBodyOX(float angle)
            {
                upperBodyAngle += angle;
            }
        };

        void RenderTank(Tank& tank, Shader* shader,  const glm::vec3 color) {
            glm:: mat4 modelMatrix = glm::mat4(1);
            // Assuming that 'forward' is the camera's forward vector
            modelMatrix = glm::translate(modelMatrix, tank.position);
            modelMatrix *= glm::rotate(modelMatrix, RADIANS(tank.angle), glm::vec3(0, 1, 0));
            //modelMatrix = glm::scale(modelMatrix, tank.size);
            //modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(0, 1, 0));
            RenderSimpleMesh(meshes["tankbody"], shader, modelMatrix, color);
            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tank.position);
            //modelMatrix = glm::scale(modelMatrix, tank.size);
            modelMatrix *= glm::rotate(modelMatrix, RADIANS(tank.angle), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(-1, 0, 0));
            RenderSimpleMesh(meshes["tankwheels"], shader, modelMatrix, color);
            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tank.position.x, tank.position.y, tank.position.z));
            //modelMatrix = glm::scale(modelMatrix, tank.size);
            modelMatrix *= glm::rotate(modelMatrix, RADIANS(tank.angle), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(-1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(180), glm::vec3(0, 0, 1));
            modelMatrix = glm::rotate(modelMatrix, tank.upperBodyAngle, glm::vec3(0, 0, 1));
            RenderSimpleMesh(meshes["tanktop"], shader, modelMatrix, color);
            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tank.position.x, tank.position.y, tank.position.z));
            //modelMatrix = glm::scale(modelMatrix, tank.size);
            //modelMatrix = glm::translate(modelMatrix, glm::vec3(1, 0, 0));
            modelMatrix *= glm::rotate(modelMatrix, RADIANS(tank.angle), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(-1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(180), glm::vec3(0, 0, 1));
            modelMatrix = glm::rotate(modelMatrix, tank.upperBodyAngle, glm::vec3(0, 0, 1));
            modelMatrix = glm::rotate(modelMatrix, tank.upperTurretAngle, glm::vec3(0, 1, 0));
            //modelMatrix = glm::translate(modelMatrix, glm::vec3(-1, 0, 0));
            RenderSimpleMesh(meshes["tankturret"], shader, modelMatrix, color);
        }

        class Projectile
        {
        public:
            float launchAngle;
            float gravity;
            glm::vec3 position, velocity, direction;
            float startTime;
            float speed;
            float angle;
            float size = 0.5f;
            float verticalSpeed;

            Projectile::Projectile(glm::vec3 position, float launchAngle, float speed, float gravity, float startTime, glm::vec3 direction, float angle)
                : position(position), launchAngle(launchAngle), speed(speed), gravity(gravity), startTime(startTime), direction(direction), angle(angle)
            {
         	    velocity.x = speed * direction.x;
				velocity.y = speed * direction.z - 0.5f * gravity * startTime;
				velocity.z = speed * -direction.y;
                verticalSpeed = speed * direction.z;
            }

        };

        void RenderProjectile(Projectile& projectile, Shader* shader, float deltaTimeSeconds)
        {
            float currentTime = glfwGetTime();
            float elapsedTime = currentTime - projectile.startTime;

            projectile.verticalSpeed -= 0.5f * projectile.gravity * elapsedTime;

        
            projectile.velocity.y = projectile.verticalSpeed;

            projectile.position.x -= projectile.velocity.x * deltaTimeSeconds;
            projectile.position.y += projectile.verticalSpeed * deltaTimeSeconds * deltaTimeSeconds;
            projectile.position.z -= projectile.velocity.z * deltaTimeSeconds;
            //cout << projectile.position.x << " " << projectile.position.y << " " << projectile.position.z << endl;

            glm::mat4 modelMatrix = glm::mat4(1.0f);
            
            modelMatrix = glm::translate(modelMatrix, projectile.position);
            //modelMatrix *= glm::translate(modelMatrix, -projectile.position);
            modelMatrix *= glm::scale(modelMatrix, glm::vec3(0.5f));
            //modelMatrix *= glm::translate(modelMatrix, projectile.position);

            RenderSimpleMesh(meshes["sphere"], shader, modelMatrix, glm::vec3(1.0f, 0.0f, 0.0f)); 
        }


        std::vector<Tank> tanks;
        Tank myTank;
        std::vector<Projectile> projectiles;
        bool ProjectileCollidesWithBuilding(const Projectile& projectile, const Building& building);
        bool ProjectileCollidesWithTank(const Projectile& projectile, const Tank& tank);



    };
}   // namespace m1