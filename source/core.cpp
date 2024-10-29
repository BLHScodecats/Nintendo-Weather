#include "core.hpp"

Core::Core()
{
    this->topLeft = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    this->topRight = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
    this->bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Get main thread priority
    s32 threadPriority;
    svcGetThreadPriority(&threadPriority, CUR_THREAD_HANDLE);

    this->input = Input();
    this->httpServer = HTTPServer(&this->weatherData, &this->weatherDataLock, threadPriority);

    WeatherDataPoint::readFromFile("weatherdata.txt", &this->weatherData);

    // Initialize server
    this->httpServer.StartServer();

    // Set initial scene
    this->currentScene = this->baseScenePage1;
}

Core::~Core() {}

void Core::Update()
{
    this->programRunTime = svcGetSystemTick() / CPU_TICKS_PER_MSEC;

    this->input.GatherInputs();

    this->currentScene.Update();

    for (unsigned int i = 0; i < this->currentScene.topScreenObjects.size(); i++)
    {
        this->currentScene.topScreenObjects[i]->Update(this->input);
    }

    for (unsigned int i = 0; i < this->currentScene.bottomScreenObjects.size(); i++)
    {
        this->currentScene.bottomScreenObjects[i]->Update(this->input);
    }
}

void Core::Render()
{
    float depthMult = osGet3DSliderState();

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    // Top screen
    C2D_TargetClear(topLeft, this->currentScene.topBackgroundColor);
    C2D_SceneBegin(topLeft);
    C2D_Prepare();
    for (unsigned int i = 0; i < this->currentScene.topScreenObjects.size(); i++)
    {
        this->currentScene.topScreenObjects[i]->Render(-depthMult);
    }
    C2D_Flush();
    if (depthMult != 0.0f)
    {
        C2D_TargetClear(topRight, this->currentScene.topBackgroundColor);
        C2D_SceneBegin(topRight);
        C2D_Prepare();
        for (unsigned int i = 0; i < this->currentScene.topScreenObjects.size(); i++)
        {
            this->currentScene.topScreenObjects[i]->Render(depthMult);
        }
        C2D_Flush();
    }

    // Bottom screen
    C2D_TargetClear(bottom, this->currentScene.bottomBackgroundColor);
    C2D_SceneBegin(bottom);
    C2D_Prepare();
    for (unsigned int i = 0; i < this->currentScene.bottomScreenObjects.size(); i++)
    {
        this->currentScene.bottomScreenObjects[i]->Render(0);
    }
    C2D_Flush();

    C3D_FrameEnd(0);
}