#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/App.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/JobSystem.hpp"

Renderer* g_theRenderer = nullptr;			
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
DevConsole* g_theDevConsole = nullptr;
BitmapFont* g_theFont = nullptr;
JobSystem* g_theJobSystem = nullptr;

constexpr float MAX_FRAME_SEC = 1.f / 10.f;


bool QuitAppCallbackFunction(EventArgs& args)
{
	UNUSED(args);
	m_isQuitting = true;
	return true;
}


App::App()
{
	BGM = 0;
	m_theGame = nullptr;

	m_screenCamera = new Camera();
}

App::~App()
{
	delete g_theDevConsole;
	delete m_theGame;
	delete g_theAudio;
	delete g_theRenderer;
	delete g_theWindow;
	delete g_theInput;
	delete m_screenCamera;

	g_theDevConsole = nullptr;
	m_theGame = nullptr;
	g_theAudio = nullptr;
	g_theWindow = nullptr;
	g_theRenderer = nullptr;
	g_theInput = nullptr;
	m_screenCamera = nullptr;
}

void App::Startup()
{
	LoadingGameConfig();
	

	m_screenCamera->SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Simple Miner!";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_camera = m_screenCamera;
	devConsoleConfig.m_renderer = g_theRenderer;

	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	JobSystemConfig jobConfig;
	jobConfig.m_numWorkerThreads = 15;
	g_theJobSystem = new JobSystem(jobConfig);
	
	m_theGame = new Game(this);

	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theAudio->Startup();
	g_theJobSystem->Startup();
	m_theGame->Startup();
	SubscribeEventCallbackFunction("quit", QuitAppCallbackFunction);

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Keys");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "WASD    - Moving Horizontally");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Up      - Camera Up");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Down    - Camera Down");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Left    - Camera Left");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Right   - Camera Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Q/E     - Moving Vertically");
	

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	debugRenderConfig.m_startHidden = false;
	DebugRenderSystemStartup(debugRenderConfig);
	FireEvent("debugrenderclear");
	g_theFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
}


void App::Run()
{
	while (!m_isQuitting)			
	{
		Sleep(0); 
        Runframe();
	}
}

void App::Shutdown()
{	
	DebugRenderSystemShutdown();
	g_theInput->Shutdown();
	g_theWindow->Shutdown();
	g_theRenderer->Shutdown();
	g_theAudio->Shutdown();
	g_theJobSystem->ShutDown();
	m_theGame->Shutdown();
}

void App::Runframe()
{	
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	BeginFrame();
	Update(deltaSeconds);
	Render();
	EndFrame();
}

bool App::HandleQuitRequested()
{
	return false;
}

bool App::OpenSlowMo()
{
	g_theAudio->SetSoundPlaybackSpeed(BGM, 0.1f);

	return false;
}

bool App::CloseSlowMo()
{
	//if (!m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
	}

	return false;
}

bool App::SwitchPaused()
{
	//if (m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
	}
	//else
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 0.f);
	}


	return false;
}

void App::MoveOneStepThenPaused()
{
	//if (m_isPaused) 
	{
		m_theGame->Update(0.f);
		m_theGame->Render();

	}
	//else
	{
		//if (m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
		}
		//else
		{
			g_theAudio->SetSoundPlaybackSpeed(BGM, 0.f);
		}
	}
}

bool const App::IsKeyDown(unsigned char keyCode)
{
	return g_theInput->IsKeyDown(keyCode);
}

bool const App::WasKeyJustPressed(unsigned char keyCode)
{
	return g_theInput->WasKeyJustPressed(keyCode);
}

void App::ShowAttractMode()
{
	m_theGame->ShowAttractMode();
}

bool App::GetIsAttractMo()
{
	return m_theGame->m_isAttractMode;
}


SoundPlaybackID App::GetBGMPlaybackID() const
{
	return BGM;
}


void App::BeginFrame()
{
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theDevConsole->BeginFrame();
 	DebugRenderBeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();

	if (g_theInput->WasKeyJustPressed(ESC_KEY))
	{
		g_theAudio->StopSound(BGM);

		if (m_theGame->m_isAttractMode)
		{
			m_isQuitting = true;
		}
		else
		{
			delete m_theGame;
			m_theGame = nullptr;

			m_theGame = new Game(this);
			m_theGame->Startup();
		}
	}

	if (g_theInput->IsKeyDown('T'))
	{
		this->OpenSlowMo();
	}
	else
	{
		this->CloseSlowMo();
	}


	if (g_theInput->WasKeyJustPressed('O'))
	{
		this->MoveOneStepThenPaused();
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		this->SwitchPaused();
	}

	if (g_theInput->WasKeyJustPressed(F8_KEY))
	{
		m_theGame->RandomizeDate();
	}

	if (g_theInput->WasKeyJustPressed(F1_KEY)) 
	{
		m_theGame->g_DebugMo = !m_theGame->g_DebugMo;
	}

	if (g_theInput->WasKeyJustPressed(187)) //+
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->StartSound(testSound);
		
	}

	XboxController const& controller = g_theInput->GetController(0);
	if (m_theGame->m_isAttractMode && (g_theInput->IsKeyDown(SPACE_KEY) || g_theInput->IsKeyDown('N') || controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START)))
	{
		m_theGame->m_isAttractMode = false;
	}

	if (m_theGame->m_isAttractMode && (g_theInput->IsKeyDown(KEYCODE_ENTER)))
	{
		m_theGame->m_isAttractMode = false;
		m_theGame->m_isTestMode = true;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleOpen();
	}
}

void App::Update(float deltaSeconds)
{
	Clock::TickSystemClock();
	if (m_theGame->m_isAttractMode || g_theDevConsole->IsOpen() || !g_theWindow->IsFocusingWindow())
	{
		g_theInput->SetCursorMode(true, false);
	}
	else
	{
		g_theInput->SetCursorMode(false, true);
	}

	m_theGame->Update(deltaSeconds);
	
}

void App::Render() const
{
	m_theGame->Render();
}

void App::EndFrame()
{
	CopyIsDownToWasDown();
	m_theGame->EndFrame();
	g_theAudio->EndFrame();
	DebugRenderEndFrame();
	g_theRenderer->EndFrame();

}

void App::UpdateShip(float deltaSeconds)
{
	deltaSeconds;
}



void App::CopyIsDownToWasDown()
{
	g_theInput->EndFrame();
}

void App::LoadingGameConfig()
{
	XmlDocument document;
	document.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = document.RootElement();
	m_setting.autoCreateChunks = ParseXMLAttribute(*rootElement, "autoCreateChunks", m_setting.autoCreateChunks);
	m_setting.chunkActivationDistance = ParseXMLAttribute(*rootElement, "chunkActivationDistance", m_setting.chunkActivationDistance);
	m_setting.saveModifiedChunks = ParseXMLAttribute(*rootElement, "saveModifiedChunks", m_setting.saveModifiedChunks);
	m_setting.hiddenSurfaceRemoval = ParseXMLAttribute(*rootElement, "hiddenSurfaceRemoval", m_setting.hiddenSurfaceRemoval);
	m_setting.version = ParseXMLAttribute(*rootElement, "version", m_setting.version);
	m_setting.globalIndoorLightColor = ParseXMLAttribute(*rootElement, "globalIndoorLightColor", m_setting.globalIndoorLightColor);
	m_setting.globalOutdoorLightColor = ParseXMLAttribute(*rootElement, "globalOutdoorLightColor", m_setting.globalOutdoorLightColor);
	m_setting.skyColor = ParseXMLAttribute(*rootElement, "skyColor", m_setting.skyColor);
	m_setting.fogFarDistance = m_setting.chunkActivationDistance - 16.f;
	m_setting.fogNearDistance = m_setting.chunkActivationDistance * 0.5f;
	m_setting.raycastMaxDist = ParseXMLAttribute(*rootElement, "raycastMaxDist", m_setting.raycastMaxDist);
	int worldSeed_int = 0;
	worldSeed_int = ParseXMLAttribute(*rootElement, "WorldSeed", worldSeed_int);
	m_setting.WorldSeed = static_cast<unsigned int>(worldSeed_int);
}

