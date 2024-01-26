#include "Player.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/Chunk.hpp"
#include "BlockDef.hpp"

constexpr float MOUSE_DELTA_SPEED = 20.f;

Player::Player(Game* owner)
	:Entity(owner)
{
	m_camera = new Camera();
	m_camera->SetOrthographicView(Vec2(-1.f, -1.f), Vec2(1.f, 1.f), 0.f, 1.f);
	m_angularVelocity.m_pitchDegrees = m_turnRatePerSec;
	m_angularVelocity.m_yawDegrees = m_turnRatePerSec;
	m_angularVelocity.m_rollDegrees = m_turnRatePerSec;
}

Player::~Player()
{
	delete m_camera;
	m_camera = nullptr;
}

void Player::Update(float deltaSeconds)
{
	XboxController const& controller = g_theInput->GetController(0);
	float movement = deltaSeconds * m_movementSpeed;
	Mat44 mat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	mat.Orthonormalize_XFwd_YLeft_ZUp();
	Vec3 iBasis = mat.GetIBasis3D();
	Vec3 jBasis = mat.GetJBasis3D();
	Vec3 kBasis = mat.GetKBasis3D();
	Vec3 moveIntention;
	Vec3 fixedI = Vec3(iBasis.x, iBasis.y, 0.f).GetNormalized();
	Vec3 fixedJ = Vec3(jBasis.x, jBasis.y, 0.f).GetNormalized();
	float speedFactor = 1.f;

	if (g_theInput->IsKeyDown(KEYCODE_LEFTSHIFT) || controller.IsButtonDown(XBOX_BUTTON_A) || g_theInput->IsKeyDown(SPACE_KEY))
	{
		speedFactor = 10.f;
	}
	if (g_theInput->IsKeyDown('W'))
	{
	
		moveIntention += fixedI * movement * speedFactor;

	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveIntention -= fixedI * movement * speedFactor;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveIntention += fixedJ * movement * speedFactor;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveIntention -= fixedJ * movement * speedFactor;
	}
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_orientation.m_pitchDegrees -= m_angularVelocity.m_pitchDegrees * deltaSeconds * speedFactor;
	}
	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds * speedFactor;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds * speedFactor;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_orientation.m_yawDegrees -= m_angularVelocity.m_yawDegrees * deltaSeconds * speedFactor;
	}

	if (g_theInput->IsKeyDown('Q') || controller.IsButtonDown(XBOX_BUTTON_L))
	{
		m_position.z += movement * speedFactor;
	}
	if (g_theInput->IsKeyDown('E') || controller.IsButtonDown(XBOX_BUTTON_R))
	{
		m_position.z -= movement * speedFactor;
	}
	if (g_theInput->WasKeyJustPressed('H') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		m_position = Vec3(0.f, 0.f, 0.f);
		m_orientation.m_yawDegrees = 0.f;
		m_orientation.m_pitchDegrees = 0.f;
		m_orientation.m_rollDegrees = 0.f;
	}
	float leftStickMag = controller.GetLeftStick().GetMagnitude();
	float rightStickMag = controller.GetRightStick().GetMagnitude();
	if (leftStickMag > 0.3f)
	{
		float speedRate = RangeMap(leftStickMag, 0.3f, 1.f, 0.f, 1.f);
		Vec2  leftStickPos = Vec2::MakeFromPolarDegrees(controller.GetLeftStick().GetOrientationDegrees());
		moveIntention += iBasis * movement * speedFactor * leftStickPos.y * speedRate;
		moveIntention -= jBasis * movement * speedFactor * leftStickPos.x * speedRate;
	}

	if (rightStickMag > 0.3f)
	{
		float speedRate = RangeMap(leftStickMag, 0.3f, 1.f, 0.f, 1.f);
		Vec2  RightStickPos = Vec2::MakeFromPolarDegrees(controller.GetRightStick().GetOrientationDegrees());
		m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds * speedFactor * RightStickPos.y * speedRate;
		m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds * speedFactor * RightStickPos.x * speedRate;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
	{
		m_game->m_currentWorld->DigRaycastResult();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RMB))
	{
		m_game->m_currentWorld->PutRaycastResult(m_game->m_currentWorld->m_currentType);
	}
	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_game->m_currentWorld->m_currentType = 1;
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_game->m_currentWorld->m_currentType = 2;
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		m_game->m_currentWorld->m_currentType = 3;
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		m_game->m_currentWorld->m_currentType = 4;
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		m_game->m_currentWorld->m_currentType = 11;
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		m_game->m_currentWorld->m_currentType = 10;
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		m_game->m_currentWorld->m_currentType = 9;
	}
	if (g_theInput->WasKeyJustPressed('8'))
	{
		m_game->m_currentWorld->m_currentType = 5;
	}
	if (g_theInput->WasKeyJustPressed('9'))
	{
		m_game->m_currentWorld->m_currentType = 6;
	}

	// ----------------------------------------Adding Text on Screen Camera---------------------------------------------------------------------------
	std::string topYellowLine;
	std::string topLightBlueLine;
	std::string holdingType;
	topYellowLine = Stringf("WASD=horizontal, EQ=vertical (space=fast), F8=regenerate, F1=debug");
	uint8_t typeIndex = m_game->m_currentWorld->m_currentType;
	std::string typeStr;

	typeStr = BlockDef::GetDefByIndex(typeIndex)->m_name;

	holdingType.append("You are holding Type: ");
	holdingType.append(typeStr);
	DebugAddScreenText(holdingType, Vec2(0.f, 0.f), 15.f, Vec2(0.01f, 0.93f), 0.f, Rgba8::GREEN);
	DebugAddScreenText(topYellowLine, Vec2(0.f, 0.f), 15.f, Vec2(0.01f, 0.99f), 0.f, Rgba8::YELLOW);
	int chunkNum = (int)m_game->m_currentWorld->m_activeChunks.size();
	int VertsNum = 0;
	for (auto const& pair : m_game->m_currentWorld->m_activeChunks)
	{
		if (pair.second != nullptr)
		{
			VertsNum += (int)(pair.second)->GetSizeOfCPUMesh();
		}
	}

	float FPS = 1.f / Clock::GetSystemClock().GetDeltaSeconds();
	topLightBlueLine = Stringf("Chunks=%.1d, Verts=%.1d; xyz=(%.1f,%.1f,%.1f), ypr=(%.1f,%.1f,%.1f); frameMS=%.1f ( %.1f FPS)",
		chunkNum, VertsNum, m_position.x, m_position.y,m_position.z,m_orientation.m_yawDegrees,m_orientation.m_pitchDegrees,m_orientation.m_rollDegrees
		, deltaSeconds * 1000.f, FPS);
	DebugAddScreenText(topLightBlueLine, Vec2(0.f, 0.f), 15.f, Vec2(0.01f, 0.96f), 0.f, Rgba8::LIGHTBLUE);
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	m_orientation.m_yawDegrees -= g_theInput->GetCursorClientDelta().x * MOUSE_DELTA_SPEED * deltaSeconds * speedFactor;
	m_orientation.m_pitchDegrees += g_theInput->GetCursorClientDelta().y * MOUSE_DELTA_SPEED * deltaSeconds * speedFactor;

// 	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
// 	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

	m_position += moveIntention;
	m_camera->SetTransform(m_position, m_orientation);
}

void Player::Render() const
{

}

