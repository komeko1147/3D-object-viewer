﻿#include "headers.h"
#include "trianglemesh.h"
#include "camera.h"
#include "shaderprog.h"
#include "light.h"
#include "imagetexture.h"
#include "skybox.h"


// Global variables.
int screenWidth = 600;
int screenHeight = 600;
// Triangle mesh.
TriangleMesh* mesh = nullptr;
// Lights.
DirectionalLight* dirLight = nullptr;
PointLight* pointLight = nullptr;
SpotLight* spotLight = nullptr;
glm::vec3 dirLightDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 dirLightRadiance = glm::vec3(0.9f, 0.9f, 0.9f);
glm::vec3 pointLightPosition = glm::vec3(0.8f, 0.0f, 0.8f);
glm::vec3 pointLightIntensity = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 spotLightPosition = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightIntensity = glm::vec3(0.25f, 0.25f, 0.1f);
float spotLightCutoffStartInDegree = 30.0f;
float spotLightTotalWidthInDegree = 45.0f;
glm::vec3 ambientLight = glm::vec3(0.2f, 0.2f, 0.2f);
// Camera.
Camera* camera = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fovy = 30.0f;
float zNear = 0.1f;
float zFar = 1000.0f;
// Shader.
FillColorShaderProg* fillColorShader = nullptr;
PhongShadingDemoShaderProg* phongShadingShader = nullptr;
SkyboxShaderProg* skyboxShader = nullptr;
// UI.
const float lightMoveSpeed = 0.2f;
// Skybox.
Skybox* skybox = nullptr;

// SceneObject.
struct SceneObject
{
    SceneObject() {
        mesh = nullptr;
        worldMatrix = glm::mat4x4(1.0f);
    }
    TriangleMesh* mesh;
    glm::mat4x4 worldMatrix;
};
SceneObject sceneObj;

// ScenePointLight (for visualization of a point light).
struct ScenePointLight
{
    ScenePointLight() {
        light = nullptr;
        worldMatrix = glm::mat4x4(1.0f);
        visColor = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    PointLight* light;
    glm::mat4x4 worldMatrix;
    glm::vec3 visColor;
};
ScenePointLight pointLightObj;
ScenePointLight spotLightObj;

// Function prototypes.
void ReleaseResources();
// Callback functions.
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);
void SetupRenderState();
void LoadObjects(const std::string&);
void CreateCamera();
void CreateSkybox(const std::string);
void CreateShaderLib();



void ReleaseResources()
{
    // Delete scene objects and lights.
    if (mesh != nullptr) {
        delete mesh;
        mesh = nullptr;
    }
    if (pointLight != nullptr) {
        delete pointLight;
        pointLight = nullptr;
    }
    if (dirLight != nullptr) {
        delete dirLight;
        dirLight = nullptr;
    }
    if (spotLight != nullptr) {
        delete spotLight;
        spotLight = nullptr;
    }
    // Delete camera.
    if (camera != nullptr) {
        delete camera;
        camera = nullptr;
    }
    // Delete shaders.
    if (fillColorShader != nullptr) {
        delete fillColorShader;
        fillColorShader = nullptr;
    }
    if (phongShadingShader != nullptr) {
        delete phongShadingShader;
        phongShadingShader = nullptr;
    }
    if (skyboxShader != nullptr) {
        delete skyboxShader;
        skyboxShader = nullptr;
    }
}

static float curObjRotationY = 30.0f;
static float skyboxRotationY = 30.0f;
const float rotStep = 0.002f;
bool objRotate = false;
bool skyboxRotate = false;
void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    TriangleMesh* pMesh = sceneObj.mesh;
    if (pMesh != nullptr) {
        // Update transform.
        if (objRotate) {
            curObjRotationY += 50 * rotStep;
        }
        glm::mat4x4 S = glm::scale(glm::mat4x4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
        glm::mat4x4 R = glm::rotate(glm::mat4x4(1.0f), glm::radians(curObjRotationY), glm::vec3(0, 1, 0));
        sceneObj.worldMatrix = S * R;
        // -------------------------------------------------------
		// Note: if you want to compute lighting in the View Space, 
        //       you might need to change the code below.
		// -------------------------------------------------------
        glm::mat4x4 normalMatrix = glm::transpose(glm::inverse(sceneObj.worldMatrix));
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * sceneObj.worldMatrix;
        
        // -------------------------------------------------------
		// Add your rendering code here.

        phongShadingShader->Bind();

        // Transformation matrix.
        glUniformMatrix4fv(phongShadingShader->GetLocM(), 1, GL_FALSE, glm::value_ptr(sceneObj.worldMatrix));
        glUniformMatrix4fv(phongShadingShader->GetLocNM(), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniformMatrix4fv(phongShadingShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(phongShadingShader->GetLocCameraPos(), 1, glm::value_ptr(camera->GetCameraPos()));
        // Light data.
        if (dirLight != nullptr) {
            glUniform3fv(phongShadingShader->GetLocDirLightDir(), 1, glm::value_ptr(dirLight->GetDirection()));
            glUniform3fv(phongShadingShader->GetLocDirLightRadiance(), 1, glm::value_ptr(dirLight->GetRadiance()));
        }
        if (pointLight != nullptr) {
            glUniform3fv(phongShadingShader->GetLocPointLightPos(), 1, glm::value_ptr(pointLight->GetPosition()));
            glUniform3fv(phongShadingShader->GetLocPointLightIntensity(), 1, glm::value_ptr(pointLight->GetIntensity()));
        }
        if (spotLight != nullptr) {
            glUniform3fv(phongShadingShader->GetLocSpotLightPos(), 1, glm::value_ptr(spotLight->GetPosition()));
            glUniform3fv(phongShadingShader->GetLocSpotLightIntensity(), 1, glm::value_ptr(spotLight->GetIntensity()));
            glUniform3fv(phongShadingShader->GetLocSpotLightDirection(), 1, glm::value_ptr(spotLightDirection));
            glUniform1f(phongShadingShader->GetLocCutoffPos(), spotLight->GetSpotCutoff());
            glUniform1f(phongShadingShader->GetLocTotalwidthPos(), spotLight->GetSpotTotalwidth());
        }
        glUniform3fv(phongShadingShader->GetLocAmbientLight(), 1, glm::value_ptr(ambientLight));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->Get_vbo());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)24);
        for (SubMesh& sm : mesh->GetsubMeshes()) {
            glUniform3fv(phongShadingShader->GetLocKa(), 1, glm::value_ptr(sm.material->GetKa()));
            if (sm.material->GetMapKd() != nullptr) {
                sm.material->GetMapKd()->Bind(GL_TEXTURE0);
                //glUniform1i(phongShadingShader->GetLocHas(), 1); //use a variable to check if there s MapKd,but now we use another method
                glUniform1i(phongShadingShader->GetLocMapKd(), 0);  
                glm::vec3 identity = { 1, 1, 1 };
                glUniform3fv(phongShadingShader->GetLocKd(), 1, glm::value_ptr(identity));
            }
            else{
                ImageTexture* white = new ImageTexture(); //create a 1x1 white texture
                white->Bind(GL_TEXTURE0);
                //glUniform1i(phongShadingShader->GetLocHas(), 0); //use a variable to check if there s MapKd,but now we use another method
                glUniform3fv(phongShadingShader->GetLocKd(), 1, glm::value_ptr(sm.material->GetKd()));
            }
            glUniform3fv(phongShadingShader->GetLocKs(), 1, glm::value_ptr(sm.material->GetKs()));
            glUniform1f(phongShadingShader->GetLocNs(), sm.material->GetNs());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm.iboId);
            glDrawElements(GL_TRIANGLES, sm.vertexIndices.size(), GL_UNSIGNED_INT, 0);
        }
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        phongShadingShader->UnBind();
		// -------------------------------------------------------
    }
    // -------------------------------------------------------------------------------------------

    // Visualize the light with fill color. ------------------------------------------------------
    PointLight* pointLight = pointLightObj.light;
    if (pointLight != nullptr) {
        glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), pointLight->GetPosition());
        pointLightObj.worldMatrix = T;
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * pointLightObj.worldMatrix;
        fillColorShader->Bind();
        glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(fillColorShader->GetLocFillColor(), 1, glm::value_ptr(pointLightObj.visColor));
        // Render the point light.
        pointLight->Draw();
        fillColorShader->UnBind();
    }
    SpotLight* spotLight = (SpotLight*)(spotLightObj.light);
    if (spotLight != nullptr) {
        glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), spotLight->GetPosition());
        spotLightObj.worldMatrix = T;
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * spotLightObj.worldMatrix;
        fillColorShader->Bind();
        glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(fillColorShader->GetLocFillColor(), 1, glm::value_ptr(spotLightObj.visColor));
        // Render the spot light.
        spotLight->Draw();
        fillColorShader->UnBind();
    }
    // -------------------------------------------------------------------------------------------

    // Render skybox. ----------------------------------------------------------------------------
    if (skybox != nullptr) {
        // -------------------------------------------------------
	    // Add your code to rotate the skybox.
        if (skyboxRotate) {
            skyboxRotationY += rotStep;
        }
        skybox->SetRotation(skyboxRotationY);
        // -------------------------------------------------------
        skybox->Render(camera, skyboxShader);
    }
    // -------------------------------------------------------------------------------------------

    glutSwapBuffers();
}

void ReshapeCB(int w, int h)
{
    // Update viewport.
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, screenWidth, screenHeight);
    // Adjust camera and projection.
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void ProcessSpecialKeysCB(int key, int x, int y)
{
    // Handle special (functional) keyboard inputs such as F1, spacebar, page up, etc. 
    switch (key) {
    // Rendering mode.
    case GLUT_KEY_F1:
        // Render with point mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case GLUT_KEY_F2:
        // Render with line mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case GLUT_KEY_F3:
        // Render with fill mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    
    // Light control.
    case GLUT_KEY_LEFT:
        if (pointLight != nullptr)
            pointLight->MoveLeft(lightMoveSpeed);
        break;
    case GLUT_KEY_RIGHT:
        if (pointLight != nullptr)
            pointLight->MoveRight(lightMoveSpeed);
        break;
    case GLUT_KEY_UP:
        if (pointLight != nullptr)
            pointLight->MoveUp(lightMoveSpeed);
        break;
    case GLUT_KEY_DOWN:
        if (pointLight != nullptr)
            pointLight->MoveDown(lightMoveSpeed);
        break;
    default:
        break;
    }
}

void ProcessKeysCB(unsigned char key, int x, int y)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    
    // Handle other keyboard inputs those are not defined as special keys.
    if (key == 27) {
        // Release memory allocation if needed.
        ReleaseResources();
        exit(0);
    }
    // Spot light control.
    if (spotLight != nullptr) {
        if (key == 'a')
            spotLight->MoveLeft(lightMoveSpeed);
        if (key == 'd')
            spotLight->MoveRight(lightMoveSpeed);
        if (key == 'w')
            spotLight->MoveUp(lightMoveSpeed);
        if (key == 's')
            spotLight->MoveDown(lightMoveSpeed);
    }
    // Directional light control.
    if (dirLight != nullptr) {
        if (key == 'j')
            dirLight->MoveLeft(lightMoveSpeed);
        if (key == 'l')
            dirLight->MoveRight(lightMoveSpeed);
        if (key == 'i')
            dirLight->MoveUp(lightMoveSpeed);
        if (key == 'k')
            dirLight->MoveDown(lightMoveSpeed);
    }

    // press "r" to change if skybox should rotate
    if (key == 'r' && skybox != nullptr) {
        skyboxRotate = !skyboxRotate;
    }
    // press "e" to change if object should rotate
    if (key == 'e' && sceneObj.mesh != nullptr) {
        objRotate = !objRotate;
    }
    // press "`"(~) to delete skybox
    if (key == '`') {
        delete skybox;
        skybox = nullptr;
    }
}

void SetupRenderState()
{
    glEnable(GL_DEPTH_TEST);

    glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
    glClearColor(
        (GLclampf)(clearColor.r), 
        (GLclampf)(clearColor.g), 
        (GLclampf)(clearColor.b), 
        (GLclampf)(clearColor.a)
    );
}

void LoadObjects(const std::string& modelPath)
{
    // -------------------------------------------------------
	// Note: you can change the code below if you want to load
    //       the model dynamically.
	// -------------------------------------------------------

    mesh = new TriangleMesh();
    mesh->LoadFromFile(modelPath, true);
    mesh->ShowInfo();
    sceneObj.mesh = mesh;    
}

void CreateLights()
{
    // Create a directional light.
    dirLight = new DirectionalLight(dirLightDirection, dirLightRadiance);
    // Create a point light.
    pointLight = new PointLight(pointLightPosition, pointLightIntensity);
    pointLightObj.light = pointLight;
    pointLightObj.visColor = glm::normalize((pointLightObj.light)->GetIntensity());
    // Create a spot light.
    spotLight = new SpotLight(spotLightPosition, spotLightIntensity, spotLightDirection, 
            spotLightCutoffStartInDegree, spotLightTotalWidthInDegree);
    spotLightObj.light = spotLight;
    spotLightObj.visColor = glm::normalize((spotLightObj.light)->GetIntensity());
}

void CreateCamera()
{
    // Create a camera and update view and proj matrices.
    camera = new Camera((float)screenWidth / (float)screenHeight);
    camera->UpdateView(cameraPos, cameraTarget, cameraUp);
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void CreateSkybox(const std::string texFilePath)
{
    // -------------------------------------------------------
	// Note: you can change the code below if you want to change
    //       the skybox texture dynamically.
	// -------------------------------------------------------

    const int numSlices = 36;
    const int numStacks = 18;
    const float radius = 50.0f;
    skybox = new Skybox(texFilePath, numSlices, numStacks, radius);
}

void CreateShaderLib()
{
    fillColorShader = new FillColorShaderProg();
    if (!fillColorShader->LoadFromFiles("shaders/fixed_color.vs", "shaders/fixed_color.fs"))
        exit(1);

    phongShadingShader = new PhongShadingDemoShaderProg();
    if (!phongShadingShader->LoadFromFiles("shaders/phong_shading_demo.vs", "shaders/phong_shading_demo.fs"))
        exit(1);

    skyboxShader = new SkyboxShaderProg();
    if (!skyboxShader->LoadFromFiles("shaders/skybox.vs", "shaders/skybox.fs"))
        exit(1);
}
// method related to careate pop-up menu
void resetResourse()
{
    // Release memory if needed.
    // Add your code here.
    if (mesh != NULL) {
        delete mesh;
        mesh = new TriangleMesh();
    }
}

void processMenuEvents(int option) {

    if (option == 1) {
        OPENFILENAMEW ofn;
        wchar_t szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.lpstrFile = szFile;
        ofn.hwndOwner = NULL;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"OBJ Files\0*.obj\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameW(&ofn)) {
            // User selected a file, load it.
            char filePath[300];
            WideCharToMultiByte(CP_UTF8, 0, szFile, -1, filePath, 300, NULL, NULL);
            std::string file(filePath);
            resetResourse();
            LoadObjects(file);
        }
    }
    if (option == 2) {
        OPENFILENAMEW ofn;
        wchar_t szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.lpstrFile = szFile;
        ofn.hwndOwner = NULL;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"PNG Files\0*.png\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameW(&ofn)) {
            // User selected a file, load it.
            char filePath[300];
            WideCharToMultiByte(CP_UTF8, 0, szFile, -1, filePath, 300, NULL, NULL);
            std::string file(filePath);
            delete skybox;
            CreateSkybox(file);
        }
    }
}

void createGLUTMenus() {

    int menu;

    // create the menu and
    // tell glut that "processMenuEvents" will
    // handle the events
    menu = glutCreateMenu(processMenuEvents);

    //add entries to our menu
    glutAddMenuEntry("Load Model", 1);
    glutAddMenuEntry("Load Skybox", 2);

    // attach the menu to the right button
    glutAttachMenu(GLUT_RIGHT_BUTTON);

}
//--------------------------------------------------------------


int main(int argc, char** argv)
{
    // Setting window properties.
    glutInit(&argc, argv);
    glutSetOption(GLUT_MULTISAMPLE, 4);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Texture Mapping");

    // Initialize GLEW.
    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cerr << "GLEW initialization error: " 
                  << glewGetErrorString(res) << std::endl;
        return 1;
    }

    // Initialization.
    SetupRenderState();
    LoadObjects("..\\TestModels_HW3\\Koffing\\Koffing.obj");
    LoadObjects("..\\TestModels_HW3\\Gengar\\Gengar.obj");
    CreateLights();
    CreateCamera();
    CreateSkybox("textures/photostudio_02_2k.png");
    CreateShaderLib();

    // Register callback functions.
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
    glutReshapeFunc(ReshapeCB);
    glutSpecialFunc(ProcessSpecialKeysCB);
    glutKeyboardFunc(ProcessKeysCB);

    // Create Pop-up menu.
    createGLUTMenus();
    // Start rendering loop.
    glutMainLoop();

    return 0;
}
