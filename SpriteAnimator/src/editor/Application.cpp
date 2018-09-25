////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <editor/Application.hpp>
#include <utils/Utility.hpp>
#include <utils/imguiSTL.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>

namespace px
{
	bool Application::m_showSpriteSheet = false;

	Application::Application() :
	m_window(
	sf::VideoMode(1440U, 900U), 
	"Sprite Animator", 
	sf::Style::Close, 
	sf::ContextSettings(0U, 0U, 8U)),
	m_animator(m_spriteAnimations),
	m_selectedTile(0.f, 0.f),
	m_tileSize(0, 0)
	{
		m_window.setPosition(sf::Vector2i(225, 90));
		m_window.setVerticalSyncEnabled(true);
		ImGui::SFML::Init(m_window);

		// Supply actions to the action map
		thor::Action eventClosed(sf::Event::Closed);
		thor::Action close(sf::Keyboard::Escape, thor::Action::PressOnce);

		m_actions["close"] = eventClosed || close;

		// Load textures (should not be here later...)
		m_spritesheet.loadFromFile("src/res/sprites/orc.png");
		m_sprite.setTexture(m_spritesheet);

		// Fill the vector
		// Tile size should be specified by the user!
		const auto rows = m_spritesheet.getSize().x / 64;
		const auto cols = m_spritesheet.getSize().y / 64;
		unsigned index = 0;

		// Allocate size
		m_tiles.resize(cols * rows);

		for (unsigned y = 0; y < cols; ++y)
		{
			for (unsigned x = 0; x < rows; ++x)
			{
				m_tiles[index] = { "image" + std::to_string(index), 
									sf::FloatRect(static_cast<float>(x * 64), static_cast<float>(y * 64), 64.f, 64.f) };
				index++;
			}
		}

		// Test animations
		/*addAnimation("walk", 11, 9);
		playAnimation("walk", true);*/
	}

	Application::~Application()
	{
		ImGui::SFML::Shutdown();
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
		static std::vector<char> animationName(50);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}

			ImGui::Begin("Animator", NULL, ImVec2(0, 0), 1.f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
															  ImGuiWindowFlags_NoBringToFrontOnFocus);
			addAnimationsToGUI();

			ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Animations"))
			{
				ImGui::Spacing();
				ImGui::Text("Name:");
				ImGui::SameLine();

				// Enter animation name
				ImGui::InputText("##1", animationName.data(), animationName.size(), ImGuiInputTextFlags_CharsNoBlank);
				ImGui::SameLine(ImGui::GetWindowWidth() - 40);

				if (ImGui::Button("NEW"))
				{
					if(animationName[0] != '\0')
						m_animations.insert(std::make_pair(animationName.data(), AnimationInfo()));

					// Clear vector and resize
					animationName.clear();
					animationName.resize(50);
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Text("Placeholder");
				ImGui::SameLine(ImGui::GetWindowWidth() - 35);
				ImGui::Button("X", ImVec2(20, 20)); // Remove animation
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}

			// Properties for sprite sheet
			//ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Sprite Sheet"))
			{
				ImGui::Spacing();
				ImGui::InputInt2("Tile size", &m_tileSize.x);
				ImGui::Spacing();
				utils::constrainNegativesVec(m_tileSize);

				ImGui::Spacing();
				ImGui::Image(m_spritesheet, sf::Vector2f(300.f, 300.f), sf::Color::White, sf::Color::White);
				ImGui::Spacing();
				
				if (ImGui::Button("Open texture.."))
				{
					// TODO: Add functionality to open a texture
				}

				ImGui::SameLine();

				if (ImGui::Button("Edit sprite sheet"))
				{
					// The texture must have been loaded first
					if (m_spritesheet.getSize().x > 0 && m_spritesheet.getSize().y > 0)
					{
						m_showSpriteSheet = true;
					}
				}
				ImGui::Spacing();
			}

			ImGui::Spacing();
			ImGui::End();

			// Sprite sheet
			if (m_showSpriteSheet)
			{
				ImGui::Begin("Sprite sheet", &m_showSpriteSheet, ImVec2(0, 0), 1.f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
																			        ImGuiWindowFlags_HorizontalScrollbar);
				drawGrid();
				ImGui::End();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void Application::render()
	{
		m_window.clear();
		//m_window.draw(m_sprite);
		ImGui::SFML::Render(m_window);
		m_window.display();
	}

	void Application::addAnimationsToGUI()
	{
		// Note: This can't be done before one has chosen a sprite sheet...
		unsigned int i = 1;
		for (auto& animation : m_animations)
		{
			ImGui::Spacing();
			ImGui::Text("Animation: %s", animation.first.c_str());
			ImGui::PushID(i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 35);

			// Remove animation
			if (ImGui::Button("X", ImVec2(20, 20)))
			{
				m_animations.erase(animation.first);
				ImGui::PopID();
				return;
			}

			ImGui::Spacing();
			ImGui::InputFloat("Duration", &animation.second.duration, 0.1f);
			utils::constrainNegativesFloat(animation.second.duration);

			unsigned int p = 1;
			for (auto& frame : animation.second.framesDetail)
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PushID(p);

				// Update sprite index for frame animation
				ImGui::Combo("Sprite", &frame.spriteIndex, m_tiles);
				ImGui::SameLine(ImGui::GetWindowWidth() - 35);

				// Remove frame
				if (ImGui::Button("X", ImVec2(20, 20)))
				{
					animation.second.framesDetail.erase(animation.second.framesDetail.begin() + (p - 1));
					ImGui::PopID();
					break;
				}

				ImGui::Spacing();
				ImGui::InputFloat("Duration", &frame.duration, 0.1f); // Note: This is a relative duration compared to the animation duration
				utils::constrainNegativesFloat(frame.duration);
				ImGui::PopID();
				p++;
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Text("\t\t\t\tAdd frame animation..");
			ImGui::SameLine(ImGui::GetWindowWidth() - 35);

			// Add frame animation
			if (ImGui::Button("+", ImVec2(20, 20)))
			{
				animation.second.framesDetail.push_back({});
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SameLine(ImGui::GetWindowWidth() - 230);

			if (ImGui::Button("SUBMIT"))
			{
				if (!animation.second.framesDetail.empty())
				{

				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::PopID();
			i++;
		}
	}

	void Application::drawGrid()
	{
		static unsigned index = 0;
		static std::vector<char> spriteName(50);
		const sf::Vector2f tilesetImagePos = ImGui::GetCursorScreenPos();
		ImGui::Image(m_spritesheet);

		// Draw grid
		auto draw_list = ImGui::GetWindowDrawList();
		const auto tileSize = 64.f;
		const auto xTiles = m_spritesheet.getSize().x / static_cast<unsigned>(tileSize);
		const auto yTiles = m_spritesheet.getSize().y / static_cast<unsigned>(tileSize);

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
		ImGui::Begin("Sprite", NULL, ImVec2(0, 0), 1.f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Name:");
		ImGui::SameLine();

		if (ImGui::InputText("", spriteName.data(), spriteName.size(), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_tiles[index].name = spriteName.data();
		}

		ImGui::Text("Tile: %d, %d", static_cast<int>(m_tiles[index].tile.left), static_cast<int>(m_tiles[index].tile.top));
		ImGui::End();
	
		// Highlight selected tile on spritesheet
		sf::Vector2f selectedTileTL = sf::Vector2f(m_selectedTile.x, m_selectedTile.y);
		sf::Vector2f selectedTileBR = sf::Vector2f(m_selectedTile.x + tileSize + 1.f, m_selectedTile.y + tileSize + 1.f);
		selectedTileTL += tilesetImagePos;
		selectedTileBR += tilesetImagePos;

		draw_list->AddRect(selectedTileTL, selectedTileBR, ImColor(255, 0, 0));
	}

	//void Application::addAnimation(const std::string& id, sf::Time duration)
	//{
	//	thor::FrameAnimation frameAnim;
	//	frameAnim.ad
	//	//frameAnim.addFrame(duration, static_cast<sf::IntRect>(rect));
	//	//m_spriteAnimations.addAnimation(id, addFrames(frameAnim, row, frames), duration);
	//}

	void Application::addFrameAnimation(thor::FrameAnimation& anim, const sf::FloatRect& rect, float duration)
	{
		anim.addFrame(duration, static_cast<sf::IntRect>(rect));
	}

	void Application::playAnimation(const std::string& id, bool repeat)
	{
		repeat ? m_animator.play() << id << thor::Playback::loop(id) : m_animator.play();
	}

	void Application::updateAnimation(sf::Time dt)
	{
		m_animator.update(dt);
		m_animator.animate(m_sprite);
	}
}