
#include <GLFW/glfw3.h>
#include <engine/Billboard.h>
#include <engine/CollisionBox.h>
#include <engine/Objectives.h>
#include <engine/Particles.h>
#include <engine/Plane.h>
#include <engine/QuadTexture.h>
#include <engine/RigidModel.h>
#include <engine/Terrain.h>
#include <engine/functions.h>
#include <engine/shader_m.h>
#include <engine/skybox.h>
#include <engine/textrenderer.h>
#include <glad/glad.h>
#include <iostream>
#include <Windows.h>

#define TIMER 100

int main()
{
    //:::: INICIALIZAMOS GLFW CON LA VERSIÓN 3.3 :::://
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //:::: CREAMOS LA VENTANA:::://
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Graficas Web 1 - PIA", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //:::: OBTENEMOS INFORMACIÓN DE TODOS LOS EVENTOS DE LA VENTANA:::://
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetJoystickCallback(joystick_callback);

    //:::: DESHABILITAMOS EL CURSOR VISUALMENTE EN LA PANTALLA :::://
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //:::: INICIALIZAMOS GLAD CON LAS CARACTERÍSTICAS DE OPENGL ESCENCIALES :::://
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //INICIALIZAMOS LA ESCENA
    Shader ourShader("shaders/multiple_lighting.vs", "shaders/multiple_lighting.fs");
    Shader selectShader("shaders/selectedShader.vs", "shaders/lighting_maps.fs");
    initScene(ourShader);
    Terrain terrain("textures/heightmap.jpg", texturePaths);
    SkyBox sky(1.0f, "1");
    cb = isCollBoxModel ? &models[indexCollBox].collbox : &collboxes.at(indexCollBox).second;

    //:::: RENDER:::://
    while (!glfwWindowShouldClose(window))
    {

        //::::TIMING:::://Ayuda a crear animaciones fluidas
        float currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;
        respawnCount += 0.1;

        //::::ENTRADA CONTROL:::://
        processInput(window);
        //:::: LIMPIAMOS BUFFERS:::://
        glClearColor(0.933f, 0.811f, 0.647f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //:::: PASAMOS INFORMACIÓN AL SHADER:::://
        ourShader.use();

        //:::: DEFINICIÓN DE MATRICES::::// La multiplicaciónd e model*view*projection crea nuestro entorno 3D
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //:::: RENDER DE MODELOS:::://
        drawModels(&ourShader, view, projection);
        //:::: SKYBOX Y TERRENO:::://
        loadEnviroment(&terrain, &sky, view, projection);
        

        //=======WATER=======//
        if (animWaterY <= 4.0f && animWaterX <= 4.0f && !is_water_out) {
            animWaterX += 0.001f;
            animWaterY += 0.001f;
        }
        else {
            is_water_out = true;
            if (animWaterY > 0.0f && animWaterX > 0.0f && is_water_out) {
                animWaterX -= 0.0000001f;
                animWaterY -= 0.0000001f;
            }
            else {
                is_water_out = false;
            }
        }
        plane.draw(animWaterX, animWaterY, view, projection);
        //:::: COLISIONES :::://
        collisions();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    //:::: LIBERACIÓN DE MEMORIA:::://   
    delete[] texturePaths;
    sky.Release();
    terrain.Release();
    for (int i = 0; i < lightcubes.size(); i++)
        lightcubes[i].second.Release();
    for (int i = 0; i < collboxes.size(); i++)
        collboxes[i].second.Release();
    for (int i = 0; i < models.size(); i++)
        models[i].Release();
    for (int i = 0; i < rigidModels.size(); i++)
    {
        physicsEnviroment.Unregister(rigidModels[i].getRigidbox());
    }
    physicsEnviroment.Unregister(&piso);
    physicsEnviroment.Unregister(&pared);
    glfwTerminate();


    //======WATER PLANE=========//
    plane.Release();
    glfwTerminate();
    KillTimer(NULL, TIMER);

    return 0;
}

void initScene(Shader ourShader)
{

    //AGUA
    //:::: DEFINIMOS LAS TEXTURAS DE LA MULTITEXTURA DEL TERRENO :::://
    texturePaths = new const char *[4];
    texturePaths[0] = "textures/multitexture_colors.png";
    texturePaths[1] = "textures/terrain1.png";
    texturePaths[2] = "textures/terrain2.png";
    texturePaths[3] = "textures/terrain3.png";

    //:::: POSICIONES DE TODAS LAS LUCES :::://
    pointLightPositions.push_back(glm::vec3(2.3f, 5.2f, 2.0f));
    pointLightPositions.push_back(glm::vec3(2.3f, 10.3f, -4.0f));
    pointLightPositions.push_back(glm::vec3(1.0f, 9.3f, -7.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 10.0f, -3.0f));

    //:::: POSICIONES DE TODOS LOS OBJETOS CON FISICAS :::://
    physicsObjectsPositions.push_back(glm::vec3(0.0, 10.0, 0.0));
    physicsObjectsPositions.push_back(glm::vec3(2.0, 10.0, 0.0));
    physicsObjectsPositions.push_back(glm::vec3(1.0, 10.0, 0.0));
    physicsObjectsPositions.push_back(glm::vec3(-2.0, 10.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(-2.0, 10.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(15.0, 1.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(15.0, 1.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(25.0, 10.0, -2.0));


    //:::: ESTADO GLOBAL DE OPENGL :::://
    glEnable(GL_DEPTH_TEST);

    //Definimos los collbox de la camara
    camera.setCollBox();

    //:::: CARGAMOS LOS SHADERS :::://
    ourShader.use();
       

    //:::: INICIALIZAMOS NUESTROS MODELOS :::://    
    models.push_back(Model("carroazul", "models/CarroAzul.obj", glm::vec3(5.3, 0.5, -4.3), glm::vec3(0, 90, 0), 0.0f, initScale));
    models.push_back(Model("closet", "models/closet.obj", glm::vec3(-9.6, 0.7, -2), glm::vec3(0, 0, 0), 0.0f, initScale));
    models.push_back(Model("box", "models/box.obj", glm::vec3(12, 0.8, -4.5), glm::vec3(0, 90, 0), 0.0f, initScale));
    models.push_back(Model("birds", "models/birds.obj", glm::vec3(8.95982, 11.4901, -8.48013), glm::vec3(0, 0, 0), 0.0f, 1.5f));
    models.push_back(Model("arbol", "models/arbol.obj", glm::vec3(-9.6, 0.0, -2), glm::vec3(0, 0, 0), 0.0f, 0.5f));
    models.push_back(Model("semaforo", "models/semaforo.obj", glm::vec3(5.62004, -0.17, 24.3605), glm::vec3(0, 0, 0), 0.0f, 50));
    models.push_back(Model("box", "models/box.obj", glm::vec3(35.2304, 0.85, -9.30019), glm::vec3(0, 180, 0), 0.0f, 1));
    models.push_back(Model("fire", "models/fire.obj", glm::vec3(36.45, 0.8, -2.1701), glm::vec3(0, 180, 0), 0.0f, 1));
    models.push_back(Model("tv", "models/tv.obj", glm::vec3(35.4, 0.71, -6.60005), glm::vec3(0, 180, 0), 0.0f, 2.0f));
    models.push_back(Model("house2", "models/house2.obj", glm::vec3(2.72, 0.0, -2), glm::vec3(0, 0, 0), 0.0f, 1));
    models.push_back(Model("box", "models/box.obj", glm::vec3(35.2304, 0.85, -9.30019), glm::vec3(0, 180, 0), 0.0f, 1));
    models.push_back(Model("nubes", "models/nubes.obj", glm::vec3(5.62004, 17, 24.3605), glm::vec3(0, 180, 0), 0.0f, 1));
    //models.push_back(Model("nubes2", "models/nubes.obj", glm::vec3(54.1135, 17, -2.31996), glm::vec3(0, 180, 0), 0.0f, 1));
    //models.push_back(Model("nubes3", "models/nubes.obj", glm::vec3(34.9202, 19.14, 43.430038), glm::vec3(0, 180, 0), 0.0f, 1));
    //models.push_back(Model("nubes4", "models/nubes.obj", glm::vec3(78.13545, 19.14, 25.430038), glm::vec3(0, 180, 0), 0.0f, 1));
    
   
    //CREAMOS TODAS  LAS CAJAS DE COLISION INDIVIDUALES
    CollisionBox collbox;
    glm::vec4 colorCollbox(0.41f, 0.2f, 0.737f, 0.06f);
    collbox = CollisionBox(glm::vec3(36.8693, 3.09, -0.860156), glm::vec3(0.3, 4.75, 10.1101), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(0, pair<string, CollisionBox>("pared_atras", collbox)));
    collbox = CollisionBox(glm::vec3(14.96, 4.15, -3.32), glm::vec3(0.5, 8.13003, 17.7901), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(1, pair<string, CollisionBox>("pared_frente_izq", collbox)));
    collbox = CollisionBox(glm::vec3(25.0, 3.95, -13.27), glm::vec3(14.97, 8.11, 0.5), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(2, pair<string, CollisionBox>("pared_frente_der", collbox)));
    collbox = CollisionBox(glm::vec3(19.36, 5.12002, 6.94001), glm::vec3(6.08002, 9.71013, 0.5), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(3, pair<string, CollisionBox>("pared_frente_arriba", collbox)));
    collbox = CollisionBox(glm::vec3(28.5604, 10, -0.8501), glm::vec3(9.3101, 0.49, 5), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(4, pair<string, CollisionBox>("techo", collbox)));
    collbox = CollisionBox(glm::vec3(18.7502, 0.899866, -4.66011), glm::vec3(0.49, 0.5, 0.5), colorCollbox);

    //CREAMOS LOS LIGHTCUBES QUE ENREALIDAD SON COLLISION BOXES QUE NOS AYUDARAN A IDENTIFICAR LA POSICIÓN DE DONDE ESTAN
    glm::vec3 lScale(0.5);
    colorCollbox = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    collbox = CollisionBox(pointLightPositions[0], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(0, pair<string, CollisionBox>("LUZ1", collbox)));
    collbox = CollisionBox(pointLightPositions[1], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(1, pair<string, CollisionBox>("LUZ2", collbox)));
    collbox = CollisionBox(pointLightPositions[2], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(2, pair<string, CollisionBox>("LUZ3", collbox)));
    collbox = CollisionBox(pointLightPositions[3], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(3, pair<string, CollisionBox>("LUZ4", collbox)));

    //===============ADD WATER===============//
    plane = Plane("textures/agua3.jpg", 2048.0, 2048.0, 0.0, 0.0);  //=========ADD PATH OF THE WATER ON THE TEXT===========//

    plane.setPosition(glm::vec3(20.5, -0.5, -10.5));                  //========POSITION OF THE WATER========//
    plane.setAngles(glm::vec3(90.0, 0.0, 0.0));                 //========ANGLE OF WATER==============//
    plane.setScale(glm::vec3(150.0));                            //========SCALE OF WATER IMAGE=======//
     
    //==========BILLBOARD==========//
    
    fuego = Billboard("textures/fuego.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 400.0f, 420.0f);//907.0f, 1536.0f
    fuego.setPosition(glm::vec3(36.45, 0.7, -2.1701));//1024, 516
    fuego.setScale(1.5f);

    pasto = Billboard("textures/pasto.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 512.0f, 258.0f);//907.0f, 1536.0f
    pasto.setPosition(glm::vec3(0, -0.5, 0));//1024, 516
    pasto.setScale(1.5f);

    pasto2 = Billboard("textures/pasto.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 512.0f, 258.0f);//907.0f, 1536.0f
    pasto2.setPosition(glm::vec3(-15, -0.5, 0));//1024, 516
    pasto2.setScale(1.5f);

    pasto3 = Billboard("textures/pasto.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 512.0f, 258.0f);//907.0f, 1536.0f
    pasto3.setPosition(glm::vec3(-15, -0.5,-5));//1024, 516
    pasto3.setScale(1.5f);

    pasto4 = Billboard("textures/pasto.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 512.0f, 258.0f);//907.0f, 1536.0f
    pasto4.setPosition(glm::vec3(-15, -0.5, -10));//1024, 516
    pasto4.setScale(1.5f);

    arbol = Billboard("textures/arbol.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 325.0f, 405.0f);//907.0f, 1536.0f
    arbol.setPosition(glm::vec3(-15, -0.5, 20));//1024, 516
    arbol.setScale(1.5f);

    humo = Billboard("textures/humo.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 515.0f, 515.0f);//907.0f, 1536.0f
    humo.setPosition(glm::vec3(-20, -0.5, 10));//1024, 516
    humo.setScale(1.5f);

}
//:::: CONFIGURACIONES :::://

void loadEnviroment(Terrain *terrain, SkyBox *sky, glm::mat4 view, glm::mat4 projection)
{
    glm::mat4 model = mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, -2.5f, 0.0f));   // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(100.5f, 100.0f, 100.5f)); // it's a bit too big for our scene, so scale it down

    terrain->draw(model, view, projection);
    terrain->getShader()->setFloat("shininess", 100.0f);
    setMultipleLight(terrain->getShader(), pointLightPositions);

    glm::mat4 skyModel = mat4(1.0f);
    skyModel = glm::translate(skyModel, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    skyModel = glm::scale(skyModel, glm::vec3(40.0f, 40.0f, 40.0f));  // it's a bit too big for our scene, so scale it down
    sky->draw(skyModel, view, projection, skyPos);
    sky->getShader()->setFloat("shininess", 10.0f);
    setMultipleLight(sky->getShader(), pointLightPositions);

    //RENDER DE LIGHT CUBES
    if (renderLightingCubes)
        for (pair<int, pair<string, CollisionBox>> lights : lightcubes)
            lights.second.second.draw(view, projection);

    
    //Cambio de skybox (amanecer)
    contadorSky = (float)glfwGetTime();
    int totalSec;
    if (contadorSky > 60) {// Sacar el residuo
        totalSec = (int)contadorSky % 60;
    }
    else {
        totalSec = (int)contadorSky;
    }
    if (/*minuteCount % 2 == 0*/  totalSec >= 0 && totalSec <= 30) {//cada 30 segundos

        if (cambioSky > 0.2) {
            mainLight = vec3(cambioSky);
            cambioSky -= 0.001;
        }
        else {
            if (changeSkyBoxTexture == 0)
            {
                isNight = true;
                isDay2 = false;
                sky->reloadTexture("5");
                changeSkyBoxTexture++;
            }
        }
    }
    else {
        if (cambioSky < 0.5) {
            mainLight = vec3(cambioSky);
            cambioSky += 0.001;
        }
        else {
            if (changeSkyBoxTexture == 1)
            {
                isDay2 = true;
                isNight = false;
                sky->reloadTexture("1");
                changeSkyBoxTexture--;
            }
        }
    }
    
    //========WATER ANIMATION========//
    if (animWaterY <= 4.0f && animWaterX <= 4.0f && !is_water_out) {
        animWaterX += 0.001f;
        animWaterY += 0.001f;
    }
    else {
        is_water_out = true;
        if (animWaterY > 0.0f && animWaterX > 0.0f && is_water_out) {
            animWaterX -= 0.001f;
            animWaterY -= 0.001f;
        }
        else {
            is_water_out = false;
        }
    }
    //BILLBOARDS ANIMATION//
    if (fireAnimY <= 1.0f) {
        if (fireAnimY <= 3.0f) {
            fireAnimX += 0.3f;
        }
        else {
            fireAnimX = 0.0f;
            fireAnimY += 1.0f;
        }
    }
    else {
        fireAnimX = 0.0f;
        fireAnimY = 0.0f;
    }
    fuego.Draw(camera, round(fireAnimX), round(fireAnimY));
    if (humoY <= 1.0f) {
        if (humoY <= 3.0f) {
            humoX += 0.3f;
        }
        else {
            humoX = 0.0f;
            humoY += 1.0f;
        }
    }
    else {
        humoX = 0.0f;
        humoY = 0.0f;
    }
    humo.Draw(camera, round(humoX), round(humoY));
    pasto.Draw(camera, round(pastoX), round(pastoY));
    pasto2.Draw(camera, round(pastoX2), round(pastoY2));
    pasto3.Draw(camera, round(pastoX3), round(pastoY3));
    pasto4.Draw(camera, round(pastoX4), round(pastoY4));
    arbol.Draw(camera, round(arbolX), round(arbolY));
    
}
void drawModels(Shader *shader, glm::mat4 view, glm::mat4 projection)
{
    //DEFINIMOS EL BRILLO  DEL MATERIAL
    shader->setFloat("material.shininess", 60.0f);
    setMultipleLight(shader, pointLightPositions);   
    for (int i = 0; i < models.size(); i++)
    {
        //SI SE RECOGIO EL OBJETO
        shader->use();
        models[i].Draw(*shader);
        detectColls(&models[i].collbox, models[i].name, &camera, renderCollBox, collidedObject_callback);
    }
}

void setSimpleLight(Shader *shader)
{
    shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    shader->setInt("lightType", (int)lightType);
    shader->setVec3("light.position", lightPos);
    shader->setVec3("light.direction", lightDir);
    shader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
    shader->setVec3("viewPos", camera.Position);
    shader->setFloat("light.constant", 0.2f);
    shader->setFloat("light.linear", 0.02f);
    shader->setFloat("light.quadratic", 0.032f);
    shader->setFloat("material.shininess", 60.0f);
}
void setMultipleLight(Shader *shader, vector<glm::vec3> pointLightPositions)
{
    shader->setVec3("viewPos", camera.Position);

    shader->setVec3("dirLights[0].direction", pointLightPositions[0]);
    shader->setVec3("dirLights[0].ambient", mainLight.x, mainLight.y, mainLight.z);
    shader->setVec3("dirLights[0].diffuse", mainLight.x, mainLight.y, mainLight.z);
    shader->setVec3("dirLights[0].specular", mainLight.x, mainLight.y, mainLight.z);

    shader->setVec3("dirLights[1].direction", pointLightPositions[1]);
    shader->setVec3("dirLights[1].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("dirLights[1].diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLights[1].specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("dirLights[2].direction", pointLightPositions[2]);
    shader->setVec3("dirLights[2].ambient", 0.3f, 0.5f, 0.5f);
    shader->setVec3("dirLights[2].diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLights[2].specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("dirLights[3].direction", pointLightPositions[3]);
    shader->setVec3("dirLights[3].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("dirLights[3].diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLights[3].specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("pointLights[0].position", pointLightPositions[0]);
    shader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[0].constant", 1.0f);
    shader->setFloat("pointLights[0].linear", 0.09);
    shader->setFloat("pointLights[0].quadratic", 0.032);

    shader->setVec3("pointLights[1].position", pointLightPositions[1]);
    shader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[1].constant", 1.0f);
    shader->setFloat("pointLights[1].linear", 0.09);
    shader->setFloat("pointLights[1].quadratic", 0.032);

    shader->setVec3("pointLights[2].position", pointLightPositions[2]);
    shader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[2].constant", 1.0f);
    shader->setFloat("pointLights[2].linear", 0.09);
    shader->setFloat("pointLights[2].quadratic", 0.032);

    shader->setVec3("pointLights[3].position", pointLightPositions[3]);
    shader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[3].constant", 1.0f);
    shader->setFloat("pointLights[3].linear", 0.09);
    shader->setFloat("pointLights[3].quadratic", 0.032);

    shader->setVec3("spotLights[0].position", pointLightPositions[0]);
    shader->setVec3("spotLights[0].direction", glm::vec3(0.2, 0.8, 0.2));
    shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[0].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[0].constant", 1.0f);
    shader->setFloat("spotLights[0].linear", 0.09);
    shader->setFloat("spotLights[0].quadratic", 0.032);
    shader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));

    // spotLight
    shader->setVec3("spotLights[1].position", pointLightPositions[1]);
    shader->setVec3("spotLights[1].direction", camera.Front);
    shader->setVec3("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[1].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[1].constant", 1.0f);
    shader->setFloat("spotLights[1].linear", 0.09);
    shader->setFloat("spotLights[1].quadratic", 0.032);
    shader->setFloat("spotLights[1].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setVec3("spotLights[2].position", pointLightPositions[2]);
    shader->setVec3("spotLights[2].direction", camera.Front);
    shader->setVec3("spotLights[2].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[2].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[2].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[2].constant", 1.0f);
    shader->setFloat("spotLights[2].linear", 0.09);
    shader->setFloat("spotLights[2].quadratic", 0.032);
    shader->setFloat("spotLights[2].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[2].outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setVec3("spotLights[3].position", pointLightPositions[3]);
    shader->setVec3("spotLights[3].direction", camera.Front);
    shader->setVec3("spotLights[3].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[3].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[3].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[3].constant", 1.0f);
    shader->setFloat("spotLights[3].linear", 0.09);
    shader->setFloat("spotLights[3].quadratic", 0.032);
    shader->setFloat("spotLights[3].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[3].outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setInt("lightType", (int)lightType);
    shader->setInt("maxRenderLights", 1);
}


void collisions()
{
    //TODO LO DE LAS COLISIONES VA AQUÍ
    detectColls(collboxes, &camera, renderCollBox, collidedObject_callback);
}
