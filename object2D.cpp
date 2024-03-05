#include "object2D.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"
#include <math.h>


Mesh* object2D::CreateSquare(
    const std::string &name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner, color),
        VertexFormat(corner + glm::vec3(length, 0, 0), color),
        VertexFormat(corner + glm::vec3(length, length, 0), color),
        VertexFormat(corner + glm::vec3(0, length, 0), color)
    };

    Mesh* square = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        square->SetDrawMode(GL_LINE_LOOP);
    } else {
        // Draw 2 triangles. Add the remaining 2 indices
        indices.push_back(0);
        indices.push_back(2);
    }

    square->InitFromData(vertices, indices);
    return square;
}

// hai sa fac tureta si hexagonul si steaua tot aici.
// de rearanjat tureta asta un pic la indices.
// de verificat daca centrul este ok.

Mesh* object2D::CreateTurret(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner + glm::vec3(0, length / 2, 0), color),
        VertexFormat(corner + glm::vec3(length / 4, 0, 0), color),
        VertexFormat(corner + glm::vec3(length / 2, length / 2, 0), color),
        VertexFormat(corner + glm::vec3(length / 4, length, 0), color),
        VertexFormat(corner + glm::vec3(length / 4, length / 2 + length / 10, 0), color),
        VertexFormat(corner + glm::vec3(length / 4, length / 2 - length / 10, 0), color),
        VertexFormat(corner + glm::vec3(3 * length / 4, length / 2 - length / 10, 0), color),
        VertexFormat(corner + glm::vec3(3 * length / 4, length / 2 + length / 10, 0), color)

    };

    Mesh* rhombus = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3};

    if (!fill) {
        rhombus->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(4);
        indices.push_back(5);
        indices.push_back(6);
        indices.push_back(7);
        indices.push_back(4);
        indices.push_back(6);
    }

    rhombus->InitFromData(vertices, indices);
    return rhombus;


}
// hexagonus maximus
Mesh* object2D::CreateHexagon(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color1,
    glm::vec3 color2,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;
    float x = length * cos(0);
    float y = length * sin(0);
    double pi = 2 * acos(0.0);
    float inner_length = 2 * length / 3;
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner + glm::vec3(length * cos(0), length * sin(0), 1), color1),
        VertexFormat(corner + glm::vec3(length * cos(pi /3 ), length * sin(pi/3), 1), color1),
        VertexFormat(corner + glm::vec3(length * cos(2*pi /3), length * sin(2*pi/3), 1), color1),
        VertexFormat(corner + glm::vec3(length * cos(pi), length * sin(pi), 1), color1),
        VertexFormat(corner + glm::vec3(length * cos(4*pi/3), length * sin(4*pi/3), 1), color1),
        VertexFormat(corner + glm::vec3(length * cos(5*pi/3), length * sin(5*pi/3), 1), color1),
        VertexFormat(corner + glm::vec3(inner_length * cos(0), inner_length * sin(0), 2), color2),
        VertexFormat(corner + glm::vec3(inner_length * cos(pi / 3), inner_length * sin(pi / 3), 2), color2),
        VertexFormat(corner + glm::vec3(inner_length * cos(2 * pi / 3), inner_length * sin(2 * pi / 3), 2), color2),
        VertexFormat(corner + glm::vec3(inner_length * cos(pi), inner_length * sin(pi), 2), color2),
        VertexFormat(corner + glm::vec3(inner_length * cos(4 * pi / 3), inner_length * sin(4 * pi / 3), 2), color2),
        VertexFormat(corner + glm::vec3(inner_length * cos(5 * pi / 3), inner_length * sin(5 * pi / 3), 2), color2),
    };

    Mesh* hexagon = new Mesh(name);
    std::vector<unsigned int> indices = {0,1,3 ,1,2,4, 2,3,5, 3,4,0, 4,5,1, 5,0,2, 
        6,7,9 ,7,8,10, 8,9,11, 9,10,6, 10,11,7, 11, 6, 8 };


    if (!fill) {
        hexagon->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        // Draw 2 triangles. Add the remaining 2 indices
        //indices.push_back(0);
        //indices.push_back(2);
    }

    hexagon->InitFromData(vertices, indices);
    return hexagon;
}

Mesh* object2D::CreateStar(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color,
    bool fill)
{

    glm::vec3 corner = leftBottomCorner;
    float degtorad = 2 * acos(0.0) / 180;
    std::vector<VertexFormat> vertices =
    {

        VertexFormat(corner + glm::vec3(length/3 * cos(0), length/3 * sin(0), 0), color),
        VertexFormat(corner + glm::vec3(length * cos(72 * degtorad), length * sin(72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length/3 * cos(2 * 72 * degtorad), length/3 *sin(2 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length * cos(3 * 72 * degtorad), length * sin(3 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length/3 * cos(4 * 72 * degtorad), length/3 * sin(4 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length * cos(5 * 72 * degtorad), length * sin(5 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length/3 * cos(6 * 72 * degtorad), length/3 * sin(6 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length * cos(7 * 72 * degtorad), length * sin(7 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length/3 * cos(8 * 72 * degtorad), length/3 * sin(8 * 72 * degtorad), 0), color),
        VertexFormat(corner + glm::vec3(length * cos(9 * 72 * degtorad), length * sin(9 * 72 * degtorad), 0), color),
    };

    Mesh* star = new Mesh(name);
    // put the indices so that it can draw a star
    std::vector<unsigned int> indices = {3,7,0, 5, 8, 1, 2,6,9};

    if (!fill) {
        star->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        
    }

    star->InitFromData(vertices, indices);
    return star;
}