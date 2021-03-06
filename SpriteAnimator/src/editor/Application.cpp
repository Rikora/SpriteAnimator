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
#include <fstream>
#include <iomanip>
#include <nfd.h>
#include <json.hpp>

using namespace nlohmann;

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
	m_tileSize(0, 0),
	m_playingAnimation("None")
	{
		m_window.setPosition(sf::Vector2i(225, 90));
		m_window.setVerticalSyncEnabled(true);
		ImGui::SFML::Init(m_window);

		// Supply actions to the action map
		thor::Action eventClosed(sf::Event::Closed);
		thor::Action close(sf::Keyboard::Escape, thor::Action::PressOnce);
		thor::Action ctrl(sf::Keyboard::LControl, thor::Action::Hold);
		thor::Action save(sf::Keyboard::S, thor::Action::PressOnce);

		m_actions["close"] = eventClosed || close;
		m_actions["saveFiles"] = ctrl && save;

		// Load button textures
		m_playButtonTexture.loadFromFile("src/res/icons/play_button.png");
		m_pauseButtonTexture.loadFromFile("src/res/icons/pause_button.png");

		// Set textures
		m_playButton.setTexture(m_playButtonTexture);
		m_pauseButton.setTexture(m_pauseButtonTexture);
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
		if (m_actions.isActive("saveFiles"))
			saveAnimations();
	}

	void Application::update(sf::Time dt)
	{
		// Various updates
		m_sprite.setPosition(sf::Vector2f(400.f, 400.f));
		ImGui::SFML::Update(m_window, dt);
		updateGUI();

		// Render
		m_window.clear();

		if (m_spriteAnimations.getSize() != 0)
		{
			updateAnimation(dt);
			m_window.draw(m_sprite);
		}

		ImGui::SFML::Render(m_window);
		m_window.display();
	}

	void Application::updateGUI()
	{
		static std::vector<char> animationName(50);

		// Display current playing animation
		ImGui::SetNextWindowPos(ImVec2(700, 22));
		ImGui::Begin("Overlay", NULL, ImVec2(340, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
															ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Spacing();
		ImGui::Text("Playing Animation: %s", m_playingAnimation.c_str());
		ImGui::Spacing();
		ImGui::End();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save all..", "CTRL+S"))
				{
					saveAnimations();
				}
				ImGui::EndMenu();
			}

			ImGui::Begin("Animator", NULL, ImVec2(0, 0), 1.f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
															  ImGuiWindowFlags_NoBringToFrontOnFocus);
			addAnimationsToGUI();

			if (ImGui::CollapsingHeader("Animations", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto submitAnimation = [this](std::size_t size)
				{
					if (animationName[0] != '\0' && hasLoadedTexture())
						m_animations.insert(std::make_pair(animationName.data(), AnimationInfo()));

					// Clear vector and resize
					animationName.clear();
					animationName.resize(size);
				};

				ImGui::PushItemWidth(140);
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Name:");
				ImGui::SameLine();

				// Enter animation name
				if (ImGui::InputText("##1", animationName.data(), animationName.size(), ImGuiInputTextFlags_CharsNoBlank | 
																						ImGuiInputTextFlags_EnterReturnsTrue))
				{
					submitAnimation(50);
				}
				ImGui::PopItemWidth();

				ImGui::SameLine(ImGui::GetWindowWidth() - 60);
				if (ImGui::Button("NEW"))
				{
					submitAnimation(50);
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Spacing();

				unsigned int i = 1;
				for (auto animation : m_animations)
				{
					ImGui::Text(animation.first.c_str());
					ImGui::PushID(i);

					// Play animation
					ImGui::SameLine(ImGui::GetWindowWidth() - 50);
					if (ImGui::ImageButton(m_playButton, sf::Vector2f(17.f, 17.f), 1, sf::Color::Black))
					{
						if (animation.second.submitted)
						{
							playAnimation(animation.first, true);
							m_playingAnimation = animation.first;
						}
					}

					// Pause animation
					ImGui::SameLine(ImGui::GetWindowWidth() - 75);
					if (ImGui::ImageButton(m_pauseButton, sf::Vector2f(17.f, 17.f), 1, sf::Color::Black))
					{
						if (animation.second.submitted && m_playingAnimation == animation.first)
							m_animator.stop();
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::PopID();
					i++;
				}
			}

			// Properties for sprite sheet
			if (ImGui::CollapsingHeader("Sprite Sheet", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// Tile size cannot be manipulated after the texture have been loaded with the tile size
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::InputInt2("Tile size", &m_tileSize.x, hasLoadedTexture() ? ImGuiInputTextFlags_ReadOnly : 0);
				ImGui::Spacing();
				utils::constrainNegativesVec(m_tileSize);

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Image(m_spritesheet, sf::Vector2f(300.f, 300.f), sf::Color::White, sf::Color::White);
				ImGui::Spacing();
				
				if (ImGui::Button("Open texture.."))
				{
					// Tile size must be chosen first and texture can't be selected again when everything is generated
					if (!hasLoadedTexture() && hasSelectedTileSize())
						openTextureFile();
				}

				ImGui::SameLine();

				if (ImGui::Button("Edit sprite sheet"))
				{
					if (hasLoadedTexture() && hasSelectedTileSize())
						m_showSpriteSheet = true;
				}
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Separator();
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

	void Application::addAnimationsToGUI()
	{
		unsigned int i = 1;
		for (auto& animation : m_animations)
		{
			if (ImGui::CollapsingHeader(animation.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Spacing();
				ImGui::PushID(i);

				ImGui::Spacing();
				ImGui::PushItemWidth(150);
				if (ImGui::InputFloat("Duration", &animation.second.duration, 0.1f))
				{
					if (animation.second.submitted && !animation.second.framesDetail.empty())
					{
						utils::constrainNegativesFloat(animation.second.duration, 0.01f);
						m_animator.stop();
						m_spriteAnimations.setDuration(animation.first, animation.second.duration);
						playAnimation(animation.first, true);
					}
				}
				ImGui::PopItemWidth();
				utils::constrainNegativesFloat(animation.second.duration, 0.01f);

				// Remove animation
				ImGui::SameLine(ImGui::GetWindowWidth() - 57);
				if (ImGui::Button("X", ImVec2(20, 20)))
				{
					if (animation.second.submitted)
					{
						m_animator.stop();
						animation.second.framesDetail.clear();
						m_spriteAnimations.removeAnimation(animation.first);
						m_playingAnimation = "None";
					}

					m_animations.erase(animation.first);
					ImGui::PopID();
					return;
				}

				unsigned int p = 1;
				for (auto& frame : animation.second.framesDetail)
				{
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::PushID(p);

					// Update sprite index for frame animation
					ImGui::PushItemWidth(150);
					ImGui::Combo("Sprite", &frame.spriteIndex, m_tiles);
					ImGui::SameLine(ImGui::GetWindowWidth() - 57);

					// Remove frame
					if (ImGui::Button("X", ImVec2(20, 20)))
					{
						animation.second.framesDetail.erase(animation.second.framesDetail.begin() + (p - 1));

						if (animation.second.framesDetail.empty() && animation.second.submitted)
						{
							m_animator.stop();
							m_spriteAnimations.removeAnimation(animation.first);
							animation.second.submitted = false; // Animation can now be added again
						}

						if (animation.second.submitted && !animation.second.framesDetail.empty())
						{
							animation.second.frameAnimation = thor::FrameAnimation();

							// Supply info to the frame animations vector
							for (unsigned i = 0; i < animation.second.framesDetail.size(); ++i)
							{
								addFrameAnimation(animation.second.frameAnimation,
									m_tiles[animation.second.framesDetail[i].spriteIndex].tile,
									animation.second.framesDetail[i].duration);
							}
						}
					}

					// Note: This is a relative duration compared to the animation duration
					ImGui::Spacing();
					ImGui::InputFloat("Duration", &frame.duration, 0.1f);
					ImGui::PopItemWidth();
					utils::constrainNegativesFloat(frame.duration);
					ImGui::PopID();
					p++;
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Spacing();

				// Add frame animation
				ImGui::SameLine(ImGui::GetWindowWidth() - 57);
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
						// Clear the frame animation object when submitting again for new changes to take place
						if (animation.second.submitted)
							animation.second.frameAnimation = thor::FrameAnimation();

						// Supply info to the frame animations vector
						for (unsigned i = 0; i < animation.second.framesDetail.size(); ++i)
						{
							addFrameAnimation(animation.second.frameAnimation,
								m_tiles[animation.second.framesDetail[i].spriteIndex].tile,
								animation.second.framesDetail[i].duration);
						}

						// Add the final animation to the container
						if (!animation.second.submitted)
						{
							addAnimation(animation.first, animation.second.frameAnimation, animation.second.duration);
							playAnimation(animation.first, true);
							animation.second.submitted = true;
							m_playingAnimation = animation.first;
						}
					}
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PopID();
				i++;
			}		
		}
	}

	void Application::drawGrid()
	{
		static unsigned index = 0;
		static bool firstTime = true;
		static std::vector<char> spriteName(50);
		const sf::Vector2f tilesetImagePos = ImGui::GetCursorScreenPos();

		// Set the selected tile to image0 when browsing for the first time
		if (firstTime)
		{
			for (unsigned i = 0; i < std::string("image0").size(); ++i)
				spriteName[i] = m_tiles[0].name[i];
			firstTime = false;
		}

		ImGui::Image(m_spritesheet);

		// Draw grid
		auto draw_list = ImGui::GetWindowDrawList();
		const auto xTiles = m_spritesheet.getSize().x / static_cast<unsigned>(m_tileSize.x);
		const auto yTiles = m_spritesheet.getSize().y / static_cast<unsigned>(m_tileSize.y);
		const auto tileSize = sf::Vector2f(static_cast<float>(m_tileSize.x), static_cast<float>(m_tileSize.y));

		// Draw horizontal lines
		for (unsigned x = 0; x < xTiles + 1; ++x) 
		{
			draw_list->AddLine(ImVec2(tilesetImagePos.x + x * tileSize.x, tilesetImagePos.y),
							   ImVec2(tilesetImagePos.x + x * tileSize.x, tilesetImagePos.y + yTiles * tileSize.y),
							   ImColor(255, 255, 255));
		}

		// Draw vertical lines
		for (unsigned y = 0; y < yTiles + 1; ++y) 
		{
			draw_list->AddLine(ImVec2(tilesetImagePos.x, tilesetImagePos.y + y * tileSize.y),
							   ImVec2(tilesetImagePos.x + xTiles * tileSize.x, tilesetImagePos.y + y * tileSize.y),
							   ImColor(255, 255, 255));
		}

		// Check input
		if (ImGui::IsItemHovered()) 
		{
			if (ImGui::IsMouseClicked(0)) 
			{
				auto relMousePos = sf::Vector2f(ImGui::GetMousePos()) - tilesetImagePos;
				m_selectedTile = sf::Vector2f(std::floor(relMousePos.x / tileSize.x) * tileSize.x, std::floor(relMousePos.y / tileSize.y) * tileSize.y);
				index = static_cast<unsigned>(std::floor(relMousePos.x / tileSize.x) + std::floor(relMousePos.y / tileSize.y) * xTiles);

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

		if (ImGui::InputText("", spriteName.data(), spriteName.size(), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_tiles[index].name = spriteName.data();
		}

		ImGui::Text("Tile: %d, %d", static_cast<int>(m_tiles[index].tile.left), static_cast<int>(m_tiles[index].tile.top));
		ImGui::End();
	
		// Highlight selected tile on spritesheet
		sf::Vector2f selectedTileTL = sf::Vector2f(m_selectedTile.x, m_selectedTile.y);
		sf::Vector2f selectedTileBR = sf::Vector2f(m_selectedTile.x + tileSize.x + 1.f, m_selectedTile.y + tileSize.y + 1.f);
		selectedTileTL += tilesetImagePos;
		selectedTileBR += tilesetImagePos;

		draw_list->AddRect(selectedTileTL, selectedTileBR, ImColor(255, 0, 0));
	}

	// File browser for selecting a sprite sheet
	void Application::openTextureFile()
	{
		nfdchar_t* outPath = NULL;
		nfdresult_t result = NFD_OpenDialog("png,jpg", NULL, &outPath);

		if (result == NFD_OKAY)
		{
			std::string m_texturePath = outPath;
			std::replace(m_texturePath.begin(), m_texturePath.end(), '\\', '/');
			auto found = m_texturePath.find_last_of("/");
			free(outPath);

			if (m_spritesheet.loadFromFile(m_texturePath))
			{
				m_sprite.setTexture(m_spritesheet);

				const auto rows = m_spritesheet.getSize().x / m_tileSize.x;
				const auto cols = m_spritesheet.getSize().y / m_tileSize.y;
				unsigned int index = 0;

				// Allocate size
				m_tiles.resize(cols * rows);

				for (unsigned y = 0; y < cols; ++y)
				{
					for (unsigned x = 0; x < rows; ++x)
					{
						m_tiles[index] = { "image" + std::to_string(index),
											sf::FloatRect(static_cast<float>(x * m_tileSize.x), static_cast<float>(y * m_tileSize.y),
														  static_cast<float>(m_tileSize.x), static_cast<float>(m_tileSize.y)) };
						index++;
					}
				}
			}
			else
				std::cout << "Failed to load texture: " << m_texturePath << std::endl;
		}
		else if (result == NFD_CANCEL)
			printf("User pressed cancel.\n");
		else
			printf("Error: %s\n", NFD_GetError());
	}

	// File browser for selecting a save folder
	void Application::saveAnimations()
	{
		nfdchar_t *outPath = NULL;
		nfdresult_t result = NFD_PickFolder(NULL, &outPath);

		if (result == NFD_OKAY)
		{
			std::string folderPath = outPath;
			std::replace(folderPath.begin(), folderPath.end(), '\\', '/');
			outputAnimationData(folderPath);
			free(outPath);
		}
		else if (result == NFD_CANCEL)
			printf("User pressed cancel.\n");
		else
			printf("Error: %s\n", NFD_GetError());
	}

	// Write animation data to animation files
	void Application::outputAnimationData(const std::string& folderPath)
	{
		if (m_spriteAnimations.getSize() != 0)
		{
			for (auto animation : m_animations)
			{
				json data;
				data["id"] = animation.first;
				data["duration"] = animation.second.duration;
				data["frames"] = animation.second.framesDetail.size();
					
				for (unsigned i = 0; i < animation.second.framesDetail.size(); ++i)
				{
					data["frame"][i]["floatRect"] = { 
					  m_tiles[animation.second.framesDetail[i].spriteIndex].tile.left,
					  m_tiles[animation.second.framesDetail[i].spriteIndex].tile.top,
					  m_tiles[animation.second.framesDetail[i].spriteIndex].tile.width,
					  m_tiles[animation.second.framesDetail[i].spriteIndex].tile.height };
					data["frame"][i]["duration"] = animation.second.framesDetail[i].duration;
				}

				const auto fullPath = folderPath + "/" + animation.first + ".anim";
				std::ofstream o(fullPath);
				o << std::setw(4) << data << std::endl;
				std::cout << "Wrote to file " << fullPath << std::endl;
			}
		}
	}

	const bool Application::hasLoadedTexture() const
	{
		return m_spritesheet.getSize().x > 0 && m_spritesheet.getSize().y > 0 ? true : false;
	}

	const bool Application::hasSelectedTileSize() const
	{
		return m_tileSize.x > 0 && m_tileSize.y > 0 ? true : false;
	}

	void Application::addAnimation(const std::string& id, const thor::FrameAnimation& anim, float duration)
	{
		m_spriteAnimations.addAnimation(id, thor::refAnimation(anim), sf::seconds(duration));
	}

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