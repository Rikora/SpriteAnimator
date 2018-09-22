////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <editor/Application.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>

namespace px
{
	bool Application::m_showSpriteSheet = true;

	Application::Application() :
	m_window(
	sf::VideoMode(1440U, 900U), 
	"Sprite Animator", 
	sf::Style::Close, 
	sf::ContextSettings(0U, 0U, 8U)),
	m_animator(m_animations),
	m_selectedTile(0.f, 0.f)
	{
		m_window.setPosition(sf::Vector2i(225, 90));
		m_window.setVerticalSyncEnabled(true);
		ImGui::SFML::Init(m_window);

		// Supply actions to the action map
		thor::Action eventClosed(sf::Event::Closed);
		thor::Action close(sf::Keyboard::Escape, thor::Action::PressOnce);

		m_actions["close"] = eventClosed || close;

		// Load textures (should not be here later...)
		m_spriteTexture.loadFromFile("src/res/sprites/orc.png");
		m_sprite.setTexture(m_spriteTexture);

		// Fill the vector
		const auto rows = m_spriteTexture.getSize().x / 64;
		const auto cols = m_spriteTexture.getSize().y / 64;
		unsigned index = 0;

		// Allocate size
		m_tiles.resize(cols * rows);

		for (unsigned y = 0; y < cols; ++y)
		{
			for (unsigned x = 0; x < rows; ++x)
			{
				// Change this to index [] later on...
				m_tiles[index] = { "image" + std::to_string(index), 
									sf::FloatRect(static_cast<float>(x * 64), static_cast<float>(y * 64), 64.f, 64.f) };
				index++;
			}
		}

		// Test animations
		addAnimation("walk", 11, 9);
		playAnimation("walk", true);
	}

	Application::~Application()
	{
		ImGui::SFML::Shutdown();
	}

	void Application::pollEvents()
	{
		m_actions.clearEvents();

		sf::Event event;
		while (m_window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			m_actions.pushEvent(event);
		}

		// React to different action types
		if (m_actions.isActive("close"))
			m_window.close();
	}

	void Application::update(sf::Time dt)
	{
		//m_sprite.setPosition(sf::Vector2f(400.f, 400.f));

		ImGui::SFML::Update(m_window, dt);
		//updateAnimation(dt);
	}

	void Application::updateGUI()
	{	
		if (m_showSpriteSheet)
		{
			ImGui::Begin("Sprite sheet", &m_showSpriteSheet, ImVec2(0, 0), 1.f, ImGuiWindowFlags_NoMove);
			drawGrid();
			ImGui::End();
		}
	}

	void Application::render()
	{
		m_window.clear();
		//m_window.draw(m_sprite);
		ImGui::SFML::Render(m_window);
		m_window.display();
	}

	void Application::drawGrid()
	{
		static unsigned index = 0;
		static std::vector<char> spriteName(50);
		const sf::Vector2f tilesetImagePos = ImGui::GetCursorScreenPos();
		ImGui::Image(m_spriteTexture);

		// Draw grid
		auto draw_list = ImGui::GetWindowDrawList();
		const auto tileSize = 64.f;
		const auto xTiles = m_spriteTexture.getSize().x / (unsigned)tileSize;
		const auto yTiles = m_spriteTexture.getSize().y / (unsigned)tileSize;

		// Draw horizontal lines
		for (unsigned x = 0; x < xTiles + 1; ++x) 
		{
			draw_list->AddLine(ImVec2(tilesetImagePos.x + x * tileSize, tilesetImagePos.y),
							   ImVec2(tilesetImagePos.x + x * tileSize, tilesetImagePos.y + yTiles * tileSize),
							   ImColor(255, 255, 255));
		}

		// Draw vertical lines
		for (unsigned y = 0; y < yTiles + 1; ++y) 
		{
			draw_list->AddLine(ImVec2(tilesetImagePos.x, tilesetImagePos.y + y * tileSize),
							   ImVec2(tilesetImagePos.x + xTiles * tileSize, tilesetImagePos.y + y * tileSize),
							   ImColor(255, 255, 255));
		}

		// Check input
		if (ImGui::IsItemHovered()) 
		{
			if (ImGui::IsMouseClicked(0)) 
			{
				auto relMousePos = sf::Vector2f(ImGui::GetMousePos()) - tilesetImagePos;
				m_selectedTile = sf::Vector2f(std::floor(relMousePos.x / tileSize) * tileSize, std::floor(relMousePos.y / tileSize) * tileSize);
				index = static_cast<unsigned>(std::floor(relMousePos.x / tileSize) + std::floor(relMousePos.y / tileSize) * xTiles);

				// Copy name to vector as the default buffer size is too small
				spriteName.clear();
				spriteName.resize(50);
				for (unsigned i = 0; i < m_tiles[index].name.size(); ++i)
					spriteName[i] = m_tiles[index].name[i];
			}
		}

		// Show info about selected tile
		ImGui::Begin("Sprite", NULL, ImVec2(0, 0), 1.f, ImGuiWindowFlags_NoMove);
		ImGui::Text("Name:");
		ImGui::SameLine();

		if (ImGui::InputText("", spriteName.data(), spriteName.size(), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_tiles[index].name = spriteName.data();
		}

		ImGui::Text("Tile: %d, %d", (int)m_tiles[index].tile.left, (int)m_tiles[index].tile.top);
		ImGui::End();
	
		// Highlight selected tile on spritesheet
		sf::Vector2f selectedTileTL = sf::Vector2f(m_selectedTile.x, m_selectedTile.y);
		sf::Vector2f selectedTileBR = sf::Vector2f(m_selectedTile.x + tileSize + 1.f, m_selectedTile.y + tileSize + 1.f);
		selectedTileTL += tilesetImagePos;
		selectedTileBR += tilesetImagePos;

		draw_list->AddRect(selectedTileTL, selectedTileBR, ImColor(255, 0, 0));
	}

	void Application::run()
	{
		sf::Clock clock;

		while (m_window.isOpen())
		{
			pollEvents();
			update(clock.restart());
			updateGUI();
			render();
		}
	}

	thor::FrameAnimation Application::addFrames(thor::FrameAnimation & anim, int row, int frames, int pxSize, float duration)
	{
		// Row -> y
		// Frames -> x

		// Remove loop for editor purpose?
		if (frames > 1)
			for (int i = 0; i < frames * pxSize; i += pxSize)
				anim.addFrame(duration, sf::IntRect(i, row * pxSize, pxSize, pxSize));
		else
			anim.addFrame(duration, sf::IntRect(frames * pxSize, row * pxSize, pxSize, pxSize));

		return anim;
	}

	void Application::addAnimation(const std::string & id, int row, int frames, sf::Time duration)
	{
		thor::FrameAnimation frameAnim;
		m_animations.addAnimation(id, addFrames(frameAnim, row, frames), duration);
	}

	void Application::playAnimation(const std::string & id, bool repeat)
	{
		repeat ? m_animator.play() << id << thor::Playback::loop(id) : m_animator.play();
	}

	void Application::updateAnimation(sf::Time dt)
	{
		m_animator.update(dt);
		m_animator.animate(m_sprite);
	}
}