#include "lab_m1/lab5/lab5.h"
#include "lab_m1/lab3/object2D.h"
#include <iostream>
#include <random>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <string>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab5::Lab5()
{
}


Lab5::~Lab5()
{
}

int sgn(float x)
{
	if (x < 0)
		return -1;
	else
		return 1;
} 

int Lab5::GetRandomNumberInRange(const int minInclusive, const int maxInclusive)
{
    static random_device randomDevice;
    static default_random_engine randomEngine(randomDevice());

    uniform_int_distribution<int> uniformDist(minInclusive, maxInclusive);

    return uniformDist(randomEngine);
}

// one type of collision check (used for tanks especially since we treat their hitboxes as circles)
bool Lab5::CollidesWith2D(float x1, float z1, float x2, float z2, float size1, float size2)
{

    float center1x = x1 + size1 / 2;
    float center2x = x2 + size2 / 2;
    float center1z = z1 + size1 / 2;
    float center2z = z2 + size2 / 2;
    float length1 = size1 / 2;
    float length2 = size2 / 2;
    float distance = glm::distance(glm::vec2(center1x,center1z), glm::vec2(center2x,center2z));
    return distance < length1 + length2;

}

// state machine for enemy tanks movement
m1::Lab5::MovementState Lab5::GetNextMovementState(const MovementState currentState)
{
    int randomChange = GetRandomNumberInRange(0, 1);
    MovementState nextState = currentState;

    switch (currentState)
    {
    case MovementState::GoingForward:
    case MovementState::GoingBackward:
        nextState = (randomChange == 1)
            ? MovementState::InPlaceRotationLeft
            : MovementState::InPlaceRotationRight;
        break;

    case MovementState::InPlaceRotationLeft:
    case MovementState::InPlaceRotationRight:
        nextState = (randomChange == 1)
            ? MovementState::GoingForward
            : MovementState::GoingBackward;
        break;

    default:
        break;
    }

    return nextState;
}

string Lab5::GetMovementStateName(const MovementState state)
{
    static const map<MovementState, string> kStateNames
    {
        { MovementState::GoingForward, "GoingForward" },
        { MovementState::GoingBackward, "GoingBackward" },
        { MovementState::InPlaceRotationLeft, "InPlaceRotationLeft" },
        { MovementState::InPlaceRotationRight, "InPlaceRotationRight" },
    };

    string s = "";

    if (kStateNames.find(state) != kStateNames.end())
    {
        s = kStateNames.at(state);
    }

    return s;
}

// other specific collision checks
bool Lab5::CollidesWithBuilding(glm::vec3 tankCoords, glm::vec3 buildingCoords, glm::vec3 tankSize, glm::vec3 buildingSize)
{
    // Calculate the half extents along x, y, and z axes
    float tankHalfX = tankSize.x / 2.0f;
    float tankHalfZ = tankSize.z / 2.0f;
    float buildingHalfX = buildingSize.x / 2.0f;
    float buildingHalfZ = buildingSize.z / 2.0f;

    // Calculate the ranges along x and z axes
    float tankMinX = tankCoords.x - tankHalfX;
    float tankMaxX = tankCoords.x + tankHalfX;
    float tankMinZ = tankCoords.z - tankHalfZ;
    float tankMaxZ = tankCoords.z + tankHalfZ;

    float buildingMinX = buildingCoords.x - buildingHalfX;
    float buildingMaxX = buildingCoords.x + buildingHalfX;
    float buildingMinZ = buildingCoords.z - buildingHalfZ;
    float buildingMaxZ = buildingCoords.z + buildingHalfZ;

    // Check for overlap along x and z axes
    bool overlapX = (tankMaxX >= buildingMinX) && (tankMinX <= buildingMaxX);
    bool overlapZ = (tankMaxZ >= buildingMinZ) && (tankMinZ <= buildingMaxZ);

    // Collision occurs only if there is overlap along both axes
    return overlapX && overlapZ;
}

bool Lab5::ProjectileCollidesWithBuilding(const Projectile& projectile, const Building& building)
{
    // Assuming the building position is at the center of its base
    glm::vec3 buildingMin = building.position - 0.5f * building.size;
    glm::vec3 buildingMax = building.position + 0.5f * building.size;

    // Check for collision in the X and Z axes
    return (projectile.position.x + projectile.size > buildingMin.x && projectile.position.x - projectile.size < buildingMax.x) &&
        (projectile.position.z + projectile.size > buildingMin.z && projectile.position.z - projectile.size < buildingMax.z);
}

bool Lab5::ProjectileCollidesWithTank(const Projectile& projectile, const Tank& tank)
{
    float distance = glm::distance(tank.position, projectile.position);

    // Check for collision using the sum of radii
    float sumOfRadii = tank.size.y*1.75f + 0.5f;
    if (distance <= sumOfRadii) {
        return true;  // Collision detected based on the centers
    }

    // Check for collision using the distance from the surface of the tank
    float tankRadius = tank.size.y * 1.75f;
    float distanceFromSurface = distance - tankRadius;
    return distanceFromSurface <= 0.5f;  
}




void Lab5::Init()
{
    renderCameraTarget = true;
    projectionType = true;

    right = 10.f;
    left = .01f;
    bottom = .01f;
    top = 10.f;
    fov = 40.f;
   

    camera = new implemented::Camera();
    camera->Set(glm::vec3(10, 6, 0), glm::vec3(0, -1, 0), glm::vec3(0, 1, 0));
    camera->RotateFirstPerson_OX(RADIANS(30));

    // Rendering the tank by its core elements. They are separated as they may move differently based on
    // user input or other factors
    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("tankbody");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "corp.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh * mesh = new Mesh("tankwheels");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "roti.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("tanktop");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "top.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("tankturret");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "cannon.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        vector<glm::vec3> vertices
        {
            glm::vec3(0.5f,   0.5f, 0.0f),    // top right
            glm::vec3(0.5f,  -0.5f, 0.0f),    // bottom right
            glm::vec3(-0.5f, -0.5f, 0.0f),    // bottom left
            glm::vec3(-0.5f,  0.5f, 0.0f),    // top left
        };

        vector<glm::vec3> normals
        {
            glm::vec3(0, 1, 1),
            glm::vec3(1, 0, 1),
            glm::vec3(1, 0, 0),
            glm::vec3(0, 1, 0)
        };

        vector<glm::vec2> textureCoords
        {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)

        };

        vector<unsigned int> indices =
        {
            0, 1, 3,
            1, 2, 3
        };

        Mesh* mesh = new Mesh("square");
        mesh->InitFromData(vertices, normals, textureCoords, indices);
        meshes[mesh->GetMeshID()] = mesh;
    }

    lastFireTime = 0.0f;
    fireCooldown = 1.0f;

        // make random number of buildings of varying sizes.
        int randomNumberOfBuildings = GetRandomNumberInRange(10, 15);

        for (int i = 0; i < randomNumberOfBuildings; i++)
        {
			Building building;
			building.color = glm::vec3(0.5f, 0.5f, 0.5f);
            building.size = glm::vec3(GetRandomNumberInRange(5, 20), GetRandomNumberInRange(20, 40), GetRandomNumberInRange(5, 20));
            building.position = glm::vec3(GetRandomNumberInRange(-50, 50), 0, GetRandomNumberInRange(-50, 50));
			buildings.push_back(building);
		}

        int randomNumberOfTanks = GetRandomNumberInRange(5, 10);

        for (int i = 0; i < randomNumberOfTanks; i++)
        {
			Tank tank;
			tank.color = glm::vec3(float(GetRandomNumberInRange(0, 256))/256, float(GetRandomNumberInRange(0, 256)) / 256, float(GetRandomNumberInRange(0, 256)) / 256);
            tank.center = glm::vec2(1 / 2, 1 / 2);
            tank.size = glm::vec3(3.f, 2.f, 2.f);
            tank.position = glm::vec3(GetRandomNumberInRange(-50, 50), 0, GetRandomNumberInRange(-50, 50));
            tank.isAlive = true;
            tank.health = 3;
            tank.isAttacking = false;
            tank.upperTurretAngle = 0;
            tank.upperBodyAngle = 0;
            tank.movementState = GetNextMovementState(MovementState::GoingForward);
            tanks.push_back(tank);
		}

    myTank.center = glm::vec2(1 / 2, 1 / 2);
    myTank.color = glm::vec3(0.70f, 0.22f, 0.14f);
    myTank.size = glm::vec3(3.f, 2.f, 2.f);
    myTank.position = glm::vec3(0, 0, 0);
    myTank.isAlive = true;
    myTank.health = 3;
    myTank.isAttacking = false;
    myTank.upperTurretAngle = 0;
    myTank.upperBodyAngle = 0;
    myTank.camera = new implemented::Camera();
    myTank.camera->Set(myTank.position, glm::vec3(myTank.position.x + 1, myTank.position.y + 1, myTank.position.z + 1), glm::vec3(0, 1, 0));

    stateTime = 0;

    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, Z_NEAR, Z_FAR);
    fixedHeight = 1.25f;
    
    // Create a shader program for drawing face polygon with the color of the normal
    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab8", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab8", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Light & material properties
    {
        lightPosition = glm::vec3(0, 100, 0);
        lightDirection = glm::vec3(0, -1, 0);
        materialShininess = 30;
        materialKd = 0.8;
        materialKs = 0.8;

        typeOfLight = 0;
        angleOX = 0.f;
        angleOY = 0.f;
        cutoffAngle = 20.f;
    }
}


void Lab5::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1.0f);  // Replace with your desired ground color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab5::Update(float deltaTimeSeconds)
{

    // main scene
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0,0,0));
    modelMatrix *= glm::scale(modelMatrix, glm::vec3(100));
    modelMatrix *= glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
    RenderSimpleMesh(meshes["square"], shaders["LabShader"], modelMatrix, glm::vec3(0.65f, 0.89f, 0.59f));

    //building rendering and collision check
    for (Building& building : buildings)
    {
		modelMatrix = glm::mat4(1);
		modelMatrix *= glm::translate(modelMatrix, building.position);
		modelMatrix *= glm::scale(modelMatrix, building.size);
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, building.color);
        if (CollidesWithBuilding(myTank.position, building.position ,myTank.size, building.size * 0.5f))
        {
            cout << building.position << " " << myTank.position;
            float p = myTank.size.x/2 +building.size.x/2 + glm::distance(myTank.position, building.position);
            glm::vec3 collisionNormal = glm::normalize(myTank.position - building.position);
            glm::vec3 pvec = abs(p) * glm::normalize(collisionNormal);
            myTank.position.x += 0.3f * pvec.x;
            float pz = myTank.size.z / 2 + building.size.z / 2 + glm::distance(myTank.position, building.position);
            glm::vec3 pvecz = abs(pz) * glm::normalize(collisionNormal);
            myTank.position.z += 0.3f * pvecz.z;
            myTank.camera->SetPosition(glm::vec3(myTank.position.x, 0, myTank.position.z));
            camera->SetPosition(glm::vec3(camera->getPosition().x + 0.6f * pvec.x, camera->getPosition().y, camera->getPosition().z + 0.6f * pvecz.z));
            cout << "collision" << endl;
		}
	}
    // tank related activities. will iterate through each tank to check/change same things
    for (Tank &tank : tanks)
    {
        // collision checks
        if (CollidesWith2D(myTank.position.x, myTank.position.z, tank.position.x, tank.position.z, myTank.size.x, tank.size.x))
        {
            float p = myTank.size.x + glm::distance(myTank.position, tank.position);
            glm::vec3 collisionNormal = glm::normalize(myTank.position - tank.position);
            glm::vec3 pvec = abs(p) * glm::normalize(collisionNormal);

            tank.position.x -= 0.3f * pvec.x;
            tank.position.z -= 0.3f * pvec.z;

            myTank.position.x += 0.3f * pvec.x;
            myTank.position.z += 0.3f * pvec.z;

            camera->SetPosition(glm::vec3(camera->getPosition().x + 0.6f * pvec.x, camera->getPosition().y, camera->getPosition().z + 0.6f * pvec.z));
            cout << "collision" << endl;
        }
        // state machine for the enemy tanks
        if(glfwGetTime() - stateTime > 5)
        {
            cout << deltaTimeSeconds << endl;
            tank.movementState = GetNextMovementState(tank.movementState);
            stateTime = glfwGetTime();
        }
        cout << GetMovementStateName(tank.movementState) << "\n";
        if (GetMovementStateName(tank.movementState) == "GoingForward")
        {
            tank.position.x += deltaTimeSeconds * 10;
		}
        else if (GetMovementStateName(tank.movementState) == "GoingBackward")
        {
            tank.position.x -= deltaTimeSeconds * 10;
        }
        else if (GetMovementStateName(tank.movementState) == "InPlaceRotationLeft")
        {
            tank.angle += deltaTimeSeconds * 2;
		}
        else if (GetMovementStateName(tank.movementState) == "InPlaceRotationRight")
        {
			tank.angle -= deltaTimeSeconds / 10;
		}

        float value = tank.health / 3.f;
        tank.color = glm::vec3(tank.color.x * value, tank.color.y * value, tank.color.z * value);
        RenderTank(tank, shaders["LabShader"], tank.color);
    }
    // main tank rendering and camera view
    modelMatrix = glm::mat4(1);
    myTank.setTankAngle(glm::degrees(atan2(-camera->getForward().z, camera->getForward().x)));
    RenderTank(myTank, shaders["LabShader"], myTank.color);
    // might be funny? i keep track of a list of all projectiles that exist on the map at a given time and
    // move them accordingly. they are removed off the list if they a) collide with anything b) go out of bounds
    auto projectile = projectiles.begin();
    while (projectile != projectiles.end()) {
        RenderProjectile(*projectile, shaders["LabShader"], deltaTimeSeconds);

        if (projectile->position.y < 0) {
            projectile = projectiles.erase(projectile);
            continue;
        }

        bool collisionDetected = false;
        for (const Building& building : buildings) {
            if (ProjectileCollidesWithBuilding(*projectile, building)) {
                collisionDetected = true;
                cout << "collision" << endl;
                break;
            }
        }

        for (Tank& tank : tanks)
        {
            if (ProjectileCollidesWithTank(*projectile, tank))
            {
                cout << "collision with projectile " << tank.health << endl;
				collisionDetected = true;
                tank.health--;
                if (tank.health <= 0)
                {
					tank.isAlive = false;
				}

				break;
			}
		}
        

        if (collisionDetected) {
            projectile = projectiles.erase(projectile);
        }
        else {
            ++projectile;
        }
    }


}


void Lab5::FrameEnd()
{
    DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Lab5::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

void Lab5::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Set shader uniforms for light properties
    GLint light_position = glGetUniformLocation(shader->program, "light_position");
    glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

    GLint light_direction = glGetUniformLocation(shader->program, "light_direction");
    glUniform3f(light_direction, lightDirection.x, lightDirection.y, lightDirection.z);

    // Set eye position (camera position) uniform
    glm::vec3 eyePosition = camera->GetTargetPosition();
    GLint eye_position = glGetUniformLocation(shader->program, "eye_position");
    glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

    // Set material property uniforms (shininess, kd, ks, object color) 
    GLint material_shininess = glGetUniformLocation(shader->program, "material_shininess");
    glUniform1i(material_shininess, materialShininess);

    GLint material_kd = glGetUniformLocation(shader->program, "material_kd");
    glUniform1f(material_kd, materialKd);

    GLint material_ks = glGetUniformLocation(shader->program, "material_ks");
    glUniform1f(material_ks, materialKs);

    GLint object_color = glGetUniformLocation(shader->program, "object_color");
    glUniform3f(object_color, color.r, color.g, color.b);

    // TODO(student): Set any other shader uniforms that you need

    // Bind model matrix
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    GLint type = glGetUniformLocation(shader->program, "type_of_light");
    glUniform1i(type, typeOfLight);

    GLint cut_off_angle = glGetUniformLocation(shader->program, "cut_off_angle");
    glUniform1f(cut_off_angle, cutoffAngle);

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab5::OnInputUpdate(float deltaTime, int mods)
{
    // move the camera only if MOUSE_RIGHT button is pressed
        float cameraSpeed = 15.0f;

        if (window->KeyHold(GLFW_KEY_W))
        {
            // Translate the camera forward
            camera->MoveForward(deltaTime * cameraSpeed);
            //myTank.camera->MoveForward(deltaTime * cameraSpeed);
            myTank.position += deltaTime * cameraSpeed/2 * glm::vec3(camera->forward.x, 0, camera->forward.z);

        }
        if (window->KeyHold(GLFW_KEY_A))
        {
            // Translate the camera to the left
            camera->RotateThirdPerson_OY(deltaTime);
            myTank.camera->RotateFirstPerson_OY(deltaTime);
            //camera->TranslateForward(deltaTime * cameraSpeed);

        }
        if (window->KeyHold(GLFW_KEY_S))
        {
            // Translate the camera backwards
            //camera->TranslateForward(deltaTime * cameraSpeed);
            camera->MoveForward(-deltaTime * cameraSpeed);
            myTank.position -= deltaTime * cameraSpeed/2 * glm::vec3(camera->forward.x, 0, camera->forward.z);

        }
        if (window->KeyHold(GLFW_KEY_D))
        {
            // Translate the camera to the right
            camera->RotateThirdPerson_OY(-deltaTime);
            myTank.camera->RotateFirstPerson_OY(-deltaTime);
            //camera->TranslateForward(deltaTime * cameraSpeed);

        }
        if (window->KeyHold(GLFW_KEY_Q))
        {
            // Translate the camera down
            camera->TranslateUpward(-deltaTime * cameraSpeed);
        }
        if (window->KeyHold(GLFW_KEY_E))
        {
            // Translate the camera up
            camera->TranslateUpward(deltaTime * cameraSpeed);
        }
       
        myTank.camera->SetPosition(glm::vec3(myTank.position.x, 0, myTank.position.z));

        if (window->KeyHold(GLFW_KEY_F))
        {
            fov += deltaTime * cameraSpeed;

            if (projectionType)
            {
                projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, Z_NEAR, Z_FAR);
            }
        }
        if (window->KeyHold(GLFW_KEY_G))
        {
            fov -= deltaTime * cameraSpeed;

            if (projectionType)
            {
                projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, Z_NEAR, Z_FAR);
            }
        }
    

    // TODO(student): Change projection parameters. Declare any extra
    // variables you might need in the class header. Inspect this file
    // for any hardcoded projection arguments (can you find any?) and
    // replace them with those extra variables.

}


void Lab5::OnKeyPress(int key, int mods)
{
    // Add key press event
    // TODO(student): Switch projections
    if (key == GLFW_KEY_T)
    {
        renderCameraTarget = !renderCameraTarget;
    }
    if (key == GLFW_KEY_O)
    {
        projectionType = false;
        projectionMatrix = glm::ortho(left, right, bottom, top, Z_NEAR, Z_FAR);
    }
    if (key == GLFW_KEY_P)
    {
        projectionType = true;
        projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, Z_NEAR, Z_FAR);
    }
    

}


void Lab5::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
    float sensivityOX = 0.001f;
    float sensivityOY = 0.001f;
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (window->GetSpecialKeyState() == 0) {
            renderCameraTarget = true;
            // TODO(student): Rotate the camera in first-person mode around
            // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
            // variables for setting up the rotation speed.
            camera->RotateFirstPerson_OX(sensivityOX * -deltaY);
            camera->RotateFirstPerson_OY(sensivityOY * -deltaX);

        }

        if (window->GetSpecialKeyState() & GLFW_MOD_CONTROL) {
            renderCameraTarget = false;
            // TODO(student): Rotate the camera in third-person mode around
            // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
            // variables for setting up the rotation speed.
            camera->RotateThirdPerson_OX(sensivityOX * -deltaY);
            camera->RotateThirdPerson_OY(sensivityOY * -deltaX);

        }
    }
    myTank.RotateUpperBodyOX(sensivityOX * -deltaX);
    myTank.RotateUpperTurretOY(sensivityOY * -deltaY);

}


void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
    if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT))
    {
        if (glfwGetTime() - lastFireTime > fireCooldown) {
            myTank.isAttacking = true;
            float upperTurretAngle = myTank.upperTurretAngle;
            float lowerTurretAngle = myTank.upperBodyAngle;
            float angle = RADIANS(myTank.angle + 180);
            glm::vec3 offset = glm::vec3(0.0f, -myTank.size.y, 0.0f);
            cout << angle << endl;
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), lowerTurretAngle, glm::vec3(0, 1, 0));
            rotationMatrix = glm::rotate(rotationMatrix, upperTurretAngle, glm::vec3(0, 0, 1));
            offset = glm::vec3(rotationMatrix * glm::vec4(offset, 1.0f));
            cout << offset.x << " " << offset.y << " " << offset.z << endl;
            glm::vec3 direction = glm::normalize(glm::vec3(
                    cos(lowerTurretAngle + angle) * cos(upperTurretAngle),
                    sin(lowerTurretAngle + angle),
                    cos(lowerTurretAngle + angle) * sin(upperTurretAngle)));
            cout << lowerTurretAngle + angle << endl;
            float initialSpeed = 20.f;
            glm::vec3 initialPosition = myTank.position - offset;
            initialPosition.y -= 0.5f;
            //initialPosition.x -= 0.5f;
            cout << initialPosition.x << " " << initialPosition.y << " " << initialPosition.z << endl;
            projectiles.push_back(Projectile(initialPosition, lowerTurretAngle, initialSpeed, 10.f, glfwGetTime(), direction, angle));
            cout << "fire " << glfwGetTime() << endl;

            lastFireTime = glfwGetTime();
        }
	}
}


void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab5::OnWindowResize(int width, int height)
{
}