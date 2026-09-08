#include "Terrain.h"
#include "Renderer.h"
#include <random>
#include <glm/glm.hpp>
#include <cmath>

Terrain::Terrain(Renderer* render, int gridSize, int scale) : Mesh(render), mScale(scale)
{
    // ensure at least one square
    if (gridSize < 1) gridSize = 1;

    static std::mt19937 gen{ 3 };
    std::uniform_real_distribution<float> heightDist(0.0f, 1.f);

    // Clear any data initialized by the base Mesh constructor
    mVertices.clear();
    mIndices.clear();

    // number of vertices per axis is gridSize + 1 (gridSize squares -> gridSize+1 vertices)
    int vertsPerAxis = gridSize + 1;
    float texDen = static_cast<float>(gridSize); // divide by gridSize to get texture coords in [0,1]

    // create vertices
    for (int z = 0; z < vertsPerAxis; ++z)
    {
        for (int x = 0; x < vertsPerAxis; ++x)
        {
            // random height between 0 and heightDist.upper bound
            float h = heightDist(gen);
            Vertex v;
            v.position = {(float)x * mScale, h, (float)z * mScale };
            v.color = {0.0f, 1.0f, 0.0f};
            v.textureCoordinate = {(texDen > 0.0f) ? (float)x / texDen : 0.0f, (texDen > 0.0f) ? (float)z / texDen : 0.0f};
            mVertices.push_back(v);
        }
    }

    // create indices: two triangles per square
    for (int z = 0; z < gridSize; ++z)
    {
        for (int x = 0; x < gridSize; ++x)
        {
            uint32_t topLeft = z * vertsPerAxis + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (z + 1) * vertsPerAxis + x;
            uint32_t bottomRight = bottomLeft + 1;

            // triangle 1
            mIndices.push_back(topLeft);
            mIndices.push_back(bottomLeft);
            mIndices.push_back(topRight);

            // triangle 2
            mIndices.push_back(topRight);
            mIndices.push_back(bottomLeft);
            mIndices.push_back(bottomRight);
        }
    }

    createBuffers();
}

float Terrain::heightAtPoint(const glm::vec2& point)
{
    if (mVertices.empty())
        return 0.0f;

    // Compute which square the world point lies in. Use floor to handle positions correctly.
    int xSquare = std::floor(point.x / mScale);
    int zSquare = std::floor(point.y / mScale);

    // Compute verts per axis from vertex count
    int vertsPerAxis = static_cast<int>(std::sqrt(mVertices.size()));
    if (vertsPerAxis < 2)
        return 0.0f;

    // If point is outside the grid, return 0
    if (xSquare < 0 || xSquare >= vertsPerAxis - 1 || zSquare < 0 || zSquare >= vertsPerAxis - 1)
        return 0.0f;

    // Indices of the square's vertices
    uint32_t topLeft = zSquare * vertsPerAxis + xSquare;
    uint32_t topRight = topLeft + 1;
    uint32_t bottomLeft = (zSquare + 1) * vertsPerAxis + xSquare;
    uint32_t bottomRight = bottomLeft + 1;

    // Compute local coordinates inside the square (origin at top-left corner of the square)
    glm::vec2 squareOrigin = glm::vec2(xSquare * mScale, zSquare * mScale);
    glm::vec2 localPoint = point - squareOrigin;

    // Build triangle vertex positions in the same local coordinate space (x,z)
    glm::vec2 pTL = glm::vec2(mVertices[topLeft].position.x, mVertices[topLeft].position.z) - squareOrigin;
    glm::vec2 pBL = glm::vec2(mVertices[bottomLeft].position.x, mVertices[bottomLeft].position.z) - squareOrigin;
    glm::vec2 pTR = glm::vec2(mVertices[topRight].position.x, mVertices[topRight].position.z) - squareOrigin;
    glm::vec2 pBR = glm::vec2(mVertices[bottomRight].position.x, mVertices[bottomRight].position.z) - squareOrigin;

    // First triangle: topLeft, bottomLeft, topRight
    glm::vec3 baryc = barycentricCoordinates(pTL, pBL, pTR, localPoint);
    if (baryc.x >= 0.0f && baryc.y >= 0.0f && baryc.z >= 0.0f)
    {
        return baryc.x * mVertices[topLeft].position.y + baryc.y * mVertices[bottomLeft].position.y + baryc.z * mVertices[topRight].position.y;
    }

    // Second triangle: bottomLeft, bottomRight, topRight
    baryc = barycentricCoordinates(pBL, pBR, pTR, localPoint);
    if (baryc.x >= 0.0f && baryc.y >= 0.0f && baryc.z >= 0.0f)
    {
        return baryc.x * mVertices[bottomLeft].position.y + baryc.y * mVertices[bottomRight].position.y + baryc.z * mVertices[topRight].position.y;
    }

    // Not inside either triangle (shouldn't happen if point inside grid), return 0
    return 0.0f;
}

glm::vec3 Terrain::barycentricCoordinates(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, 
    glm::vec2 testPoint)
{
	// The testPoint in is the point we want to find the barycentric coordinates of, 
    // with respect to the triangle defined by p1, p2, and p3.

	// Compute the area of the whole triangle (p1, p2, p3) using the cross product method
    glm::vec2 p12 = p2 - p1;
    glm::vec2 p13 = p3 - p1;
    glm::vec3 n = glm::cross(glm::vec3(p12, 0.0f), glm::vec3(p13, 0.0f)); // p12 ^ p13;
    float areal_123 = n.z; //  Need to keep the sign here - lenght gives absolute value - n.length(); // double area

    glm::vec3 baryc; // for return. Remember
    
	// Compute the area of the sub-triangles formed by the testPoint and each edge of the triangle,
    // u
    glm::vec2 p = p2 - testPoint; // *this;
    glm::vec2 q = p3 - testPoint; // *this;
    n = glm::cross(glm::vec3(p, 0.0f), glm::vec3(q, 0.0f)); //p ^ q;
    baryc.x = n.z / areal_123;
    // v
    p = p3 - testPoint; // *this;
    q = p1 - testPoint; //*this;
    n = glm::cross(glm::vec3(p, 0.0f), glm::vec3(q, 0.0f)); // p ^ q;
    baryc.y = n.z / areal_123;
    // w
    p = p1 - testPoint; //*this;
    q = p2 - testPoint; //*this;
    n = glm::cross(glm::vec3(p, 0.0f), glm::vec3(q, 0.0f));  //p ^ q;
    baryc.z = n.z / areal_123;
    return baryc;
}

