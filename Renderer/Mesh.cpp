#include "Mesh.h"
#include "sstream"
#include <fstream>
#include "Utilities.h"
#include "Logger.h"
#include "Renderer.h"

Mesh::Mesh(Renderer* render) : mRenderer{render}, mLogger(Logger::getInstance())
{
    //makeTriangle();
    //createBuffers();
}

Mesh::Mesh(Renderer* render, MeshType meshTypeIn, std::string fileNameIn) : mRenderer{render},
            mLogger(Logger::getInstance()), mFileName(fileNameIn)
{
    switch (meshTypeIn)
    {
    case MeshType::TRIANGLE :
        makeTriangle();
        break;
    case MeshType::QUAD :
        makeQuad();
        break;
    case MeshType::OBJ :
        makeObj();
        break;
    case MeshType::VertexData :
        makefromTXT();
        break;
    default:
        makeTriangle();
    }

    createBuffers();
}

Mesh::~Mesh()
{
    LOG("Mesh Destructor called");
    // Destroy only if handles are valid - prevents calls on uninitialized / already freed handles
    if (mVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mRenderer->device, mVertexBuffer, nullptr);
        mVertexBuffer = VK_NULL_HANDLE;
    }
    if (mVertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mRenderer->device, mVertexBufferMemory, nullptr);
        mVertexBufferMemory = VK_NULL_HANDLE;
    }
    if (mIndexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mRenderer->device, mIndexBuffer, nullptr);
        mIndexBuffer = VK_NULL_HANDLE;
    }
    if (mIndexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mRenderer->device, mIndexBufferMemory, nullptr);
        mIndexBufferMemory = VK_NULL_HANDLE;
    }
}

void Mesh::createBuffers()
{
    mRenderer->createVertexBuffer(this);

    // Only create index buffer if we have indices
    if (mIndices.size() > 0)
        mRenderer->createIndexBuffer(this);
}

void Mesh::makeTriangle()
{
    mVertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.f, 1.f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}}
    };
}

void Mesh::makeQuad()
{
    // Second parameter is now the surface normal!
    mVertices = {
        {{-0.5f, -0.5f, 0.f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
    };

    mIndices = {
        0, 1, 2, 2, 3, 0
    };
}

void Mesh::TriangulateFromPoints(float GridResolution){
    double xMin, xMax;
    double yMin, yMax;

    //set to first. most useful.

    //NOTE: In setting up the mesh previously, I swap Y and Z from the source file. The lazy solution here is just reading all information for Y from the Z. Apologies
    //if it makes the code less readable.

    xMin =  mVertices[0].position.z; xMax = mVertices[0].position.z;
    yMin = mVertices[0].position.z; yMax =  mVertices[0].position.z;

    //Find max values with basic search.

    for(Vertex i : mVertices){
        if(i.position.x < xMin) {
            xMin = i.position.x;
        }
        if(i.position.x > xMax) {
            xMax = i.position.x;
        }
        if(i.position.y < yMin) {
            yMin = i.position.z;
        }
        if(i.position.y > yMax) {
            yMax = i.position.z;
        }
    }

    //find number of grids. might be a slow way to go about it, but I don't want to deal with rounding errors.
    int ix = 1, iy = 1; //at least 1, I think.
    {
        float x = xMin;
        while(x < xMax){
            ix++;
            x += GridResolution;
        }
    }
    {
        float y = yMin;
        while(y < yMax){
            iy++;
            y += GridResolution;
        }
    }

    // for grid values. 3D vector. Apparently dynamic regular array sizes are not supported by the compiler by default? Seems like an oversight.
    std::vector<std::vector<std::vector<glm::vec3>>> VertexGroupings;

     //Initialize to max length. Basically, I'm doing this because I don't want to deal with index arithmetic later.
    for(int i = 0; i < ix; i++){
        VertexGroupings.push_back(std::vector<std::vector<glm::vec3>>());
        for(int j = 0; j < iy; j++){
            VertexGroupings[i].push_back(std::vector<glm::vec3>());
        }
    }



     //Organizing is effectively O(n^2)... Please keep the distance between points to 3 or above or it will take minutes.
    int counter = 0;
    for(Vertex iVertex : mVertices){
        if(counter % 20000 == 1) LOGP("Processing. %f done.", (float)counter/(float)mVertices.size() * 100.f);
        for(int i = 0; i < ix; i++){
            for(int j = 0; j < iy; j++){ //får får ikke får, får får lam.
                if(
                    iVertex.position.x > xMin + GridResolution * i
                    && iVertex.position.x < xMin + GridResolution * (i+1)
                    && iVertex.position.z > yMin + GridResolution * j
                    && iVertex.position.z < yMin + GridResolution * (j+1)
                ){
                    VertexGroupings[i][j].push_back(glm::vec3(iVertex.position.x, iVertex.position.y, iVertex.position.z));
                    break;
                }
            }
        }
        counter++;
    }

    //make averages and and push final vector.

    std::vector<glm::vec3> tempVertices;
    glm::vec2 tempUV = {0.f, 0.f};
    glm::vec3 tempNormal = {0.f,0.f,0.f};


    float cumulative_y = 0;
    int sum = 0;

    for(int i = 0; i < ix; i++){
        for(int j = 0; j < iy; j++){
            cumulative_y = 0;
            sum = 0;
            for(glm::vec3 V : VertexGroupings[i][j]){
                cumulative_y += V.y;
                sum++;
            }
            tempVertices.push_back(
                {
                 ((float)i+0.5) * GridResolution,
                    cumulative_y / (float)sum,
                    ((float)j + 0.5) * GridResolution
                }
            );
             LOGP("Created point in %i of %i, %i of %i. cumulative y = %f. %i points.", i, ix, j, iy, cumulative_y, sum);
        }
    }

    mVertices.clear();


    //Push vertex data.
    for(glm::vec3 v : tempVertices){
        mVertices.push_back({v, tempNormal, tempUV});
    }

    //Create index data.
    int tempIndex = 0;
     for(int i = 0; i < ix-1; i++){
         for(int j = 0; j < iy-1; j++){

             mIndices.push_back(tempIndex);
             mIndices.push_back(tempIndex+1);
             mIndices.push_back(tempIndex+iy);

             mIndices.push_back(tempIndex+1);
             mIndices.push_back(tempIndex+iy+1);
             mIndices.push_back(tempIndex+iy);

             tempIndex++;
         }
     }
}

void Mesh::makeObj()
{
    std::ifstream fileIn;
    fileIn.open( PATH + "Assets/" + mFileName, std::ifstream::in);
    if(!fileIn)
    {
        LOGE("ERROR: Could not load mesh!");
        LOGP("     Could not open file for reading: %s", mFileName.c_str());
        LOGW("     Making default triangle instead");
        makeTriangle();
    }

    std::string oneLine;
    std::string oneWord;
    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec3> tempNormals;
    std::vector<glm::vec2> tempUVs;

    unsigned int temp_index = 0;
    while(std::getline(fileIn,oneLine))
    {
        std::stringstream sStream;
        sStream << oneLine;
        oneWord = "";
        sStream >> oneWord;
        if(oneWord == "#")  // comment
            continue;

        if (oneWord == "")  // empty line
            continue;

        if(oneWord == "v")  // vertex
        {
            glm::vec3 tempVertex;
            sStream >> oneWord;
            tempVertex.x = std::stof(oneWord);
            sStream >>oneWord;
            tempVertex.y = std::stof(oneWord);
            sStream >>oneWord;
            tempVertex.z = std::stof(oneWord);
            tempVertices.push_back(tempVertex);

            continue;
        }
        if(oneWord == "vt") // uv coordinate
        {
            glm::vec2 tempUV;
            sStream >> oneWord;
            tempUV.x = std::stof(oneWord);
            sStream >> oneWord;
            tempUV.y = std::stof(oneWord);
            tempUVs.push_back(tempUV);

            continue;
        }
        if(oneWord == "vn") // normal
        {
            glm::vec3 tempNormal;
            sStream >> oneWord;
            tempNormal.x = std::stof(oneWord);
            sStream >> oneWord;
            tempNormal.y = std::stof(oneWord);
            sStream >> oneWord;
            tempNormal.z = std::stof(oneWord);
            tempNormals.push_back(tempNormal);

            continue;
        }
        if(oneWord == "f")  // face
        {
            int index, normal, uv;
            for(int i = 0; i<3; i++)
            {
                sStream >> oneWord;
                std::stringstream tempWord(oneWord);
                std::string segment;
                std::vector<std::string> segmentArray;

                while(std::getline(tempWord, segment, '/'))
                    segmentArray.push_back(segment);

                index =std::stoi(segmentArray[0]);

                if(segmentArray[1] !="")
                    uv = std::stoi(segmentArray[1]);
                else
                    uv = 0;

                normal = std::stoi(segmentArray[2]);

                // obj indexes starts at 1, C++ starts at 0
                index--;
                uv--;
                normal--;

                if(uv > -1)
                {
                    Vertex tempVertex{tempVertices[index],tempNormals[normal] ,tempUVs[uv]};
                    mVertices.push_back(tempVertex);
                }
                else
                {
                    Vertex tempVertex{tempVertices[index],tempNormals[normal] ,glm::vec2{0,0}};
                    mVertices.push_back(tempVertex);
                }

                mIndices.push_back(temp_index++);
            }
            continue;
        }
    }
    fileIn.close();

    LOGP("obj file %s made into a mesh!", mFileName.c_str());
}

void Mesh::makefromTXT()
{
    std::ifstream fileIn;
    fileIn.open( PATH + "../data/VertexData.txt", std::ifstream::in);
    if(!fileIn)
    {
        LOGE("ERROR: Could not load mesh!");
        LOGP("     Could not open file for reading: %s", mFileName.c_str());
        LOGW("     Making default triangle instead");
        makeTriangle();
    }


    std::string oneLine;
    std::string oneWord;
    std::string x;
    std::string y;
    std::string z;
    std::vector<glm::vec3> tempVertices;
    glm::vec3 tempNormals = glm::vec3{0.f,0.f,0.f};
    glm::vec2 tempUVs = glm::vec2{0.f,0.f};

    std::getline(fileIn, oneLine);
    int index = std::stoi(oneLine);

    LOGP("  Opening text. %i lines.", index);

    mVertices.resize(index);

    float offset_x = 0.f;
    float offset_y = 0.f;
    float offset_z = 0.f;

    unsigned int temp_index = 0;
    while(std::getline(fileIn,oneLine)){
        std::stringstream sStream;

        sStream << oneLine;
        oneWord = "";
        sStream >> x;

        float xf = std::stof(x, nullptr);

        sStream >> y;

        float yf = std::stof(y, nullptr);

        sStream >> z;

        float zf = std::stof(z, nullptr);

        if(temp_index == 0){
            offset_x = xf;
            offset_y = yf;
        }




        tempVertices.push_back(glm::vec3{xf - offset_x, zf, yf - offset_y});

        if (temp_index % 100000 == 1){
            LOGP("point text. %i lines. %f, %f, %f %s", temp_index, xf, yf, zf, oneWord.c_str());
        }

        mVertices.push_back({tempVertices[temp_index],tempNormals ,tempUVs});

        temp_index++;


    }

    TriangulateFromPoints(3.f); //Point resolution here (meters between points). please do not go beneath 3 unless you want the process to take several minutes.

    fileIn.close();

    LOGP("text file %s made into a mesh!", mFileName.c_str());
}


