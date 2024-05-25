#include <imgui.h>

#include <imgui-SFML.h>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <cmath>
#include <random>

namespace {

const float kLightSpeed = 3 * std::pow(10, 8);
const float kPlankContant = 6.626 * std::pow(10, -34);
const float kElectronCharge = 1.6 * std::pow(10, -19);
const float kPhotonScale = std::pow(10, -6);
const float kElectronScale = std::pow(10, -4);

} // namespace

float RandomInteger(float min, float max) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> uni(min, max);
    return uni(rng);
}

class Photon {
  public:
    Photon(const sf::Color& color, double kinetic_energy, double radius)
        : kinetic_energy_(kinetic_energy) {
        circle_.setRadius(radius);
        circle_.setOutlineColor(sf::Color::Black);
        circle_.setOutlineThickness(3);
        circle_.setFillColor(color);
    }

    const sf::Vector2f& GetSpeed() const {
        return speed_;
    }

    void SetSpeed(sf::Vector2f speed) {
        speed_ = std::move(speed);
    }

    const sf::Vector2f& GetPosition() const {
        return circle_.getPosition();
    }

    void SetPosition(const sf::Vector2f& position) {
        circle_.setPosition(position);
    }

    void SetPosition(float x, float y) {
        circle_.setPosition(x, y);
    }

    void Move() {
        SetPosition(circle_.getPosition() + speed_);
    }

    void SetColor(const sf::Color& color) {
        circle_.setFillColor(color);
    }

    void Draw(sf::RenderTarget& renderer) {
        renderer.draw(circle_);
    }

    template <typename T>
    bool Intersects(const sf::Rect<T>& rect) const {
        return circle_.getGlobalBounds().intersects(rect);
    }

    void Update(const sf::Time& dt) {
        SetPosition(circle_.getPosition() + speed_ * dt.asSeconds());
    }

    double GetKinteticEnegery() const {
        return kinetic_energy_;
    }

  private:
    sf::Color color_;
    double kinetic_energy_;
    sf::Vector2f speed_;
    sf::CircleShape circle_;
};

class Electron : public Photon {
  public:
    inline static const double kMass = 9.1 * std::pow(10, -31);

    explicit Electron(double kinetic_energy, double radius)
        : Photon(sf::Color::Cyan, kinetic_energy, radius) {
    }
};

class Metal {
  public:
    explicit Metal(std::string name, float work_func, sf::Color color)
        : name_(std::move(name)),
          work_func_(work_func),
          color_(std::move(color)) {
    }

    const std::string& GetName() const {
        return name_;
    }

    float GetWorkFunc() const {
        return work_func_;
    }

    const sf::Color& GetColor() const {
        return color_;
    }

  private:
    std::string name_;
    float work_func_;
    sf::Color color_;
};

static const std::vector<double> kWaveLengths = {0,   380, 450, 485, 500,
                                                 565, 590, 625, 750, 860};

std::pair<double, double> get_wave_length_bound(double wave_length) {
    for (size_t i = 0; i < kWaveLengths.size() - 1; ++i) {
        const double lhs = kWaveLengths[i];
        const double rhs = kWaveLengths[i + 1];

        if (wave_length <= rhs) {
            return {lhs, rhs};
        }
    }

    return {
        kWaveLengths[kWaveLengths.size() - 2],
        kWaveLengths[kWaveLengths.size() - 1]};
}

sf::Color GetWaveColor(double wave_length) {
    const auto [min_wave, max_wave] = get_wave_length_bound(wave_length);

    const uint8_t var_color =
        (wave_length - min_wave) * 255 / (max_wave - min_wave);

    sf::Color result = {0, 0, 0};

    if (min_wave < 380) {
        result.r = 220;
        result.b = 255;
    } else if (min_wave == 380) {
        result.r = 220 - var_color * 220 / 255;
        result.b = 255;
    } else if (min_wave == 450) {
        result.g = var_color * 180 / 255;
        result.b = 255;
    } else if (min_wave == 485) {
        result.g = 180 + var_color * 60 / 255;
        result.b = 255 - var_color * 75 / 255;
    } else if (min_wave == 500) {
        result.r = var_color * 180 / 255;
        result.g = 240 + var_color * 15 / 255;
        result.b = 185 - var_color * 75 / 255;
    } else if (min_wave == 565) {
        result.g = 255 - var_color * 25 / 255;
        result.r = 200 + var_color * 55 / 255;
        result.b = 75 - var_color * 75 / 255;
    } else if (min_wave == 590) {
        result.r = 245 - var_color * 20 / 255;
        result.g = 210 - var_color * 100 / 255;
    } else if (min_wave == 625) {
        result.r = 225 + var_color * 30 / 255;
        result.g = 110 - var_color * 100 / 255;
    } else if (min_wave > 625) {
        result.r = 255;
    }

    return result;
}

int main() {
    const auto& sizes = sf::VideoMode::getDesktopMode();

    sf::RenderWindow window(
        sf::VideoMode(sizes.width, sizes.height), L"Моделирование фотоэффекта"
    );
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(2);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 2.0;
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "/System/Library/Fonts/Helvetica.ttc", 18, NULL,
        io.Fonts->GetGlyphRangesCyrillic()
    );
    io.Fonts->Build();
    ImGui::SFML::UpdateFontTexture();

    std::vector<Metal> metals = {
        Metal("Цинк", 6.89 * std::pow(10, -19), sf::Color(186, 196, 200)),
        Metal("Медь", 7.53 * std::pow(10, -19), sf::Color(184, 115, 51)),
        Metal("Магний", 5.90 * std::pow(10, -19), sf::Color(193, 194, 195)),
    };

    Metal current_metal = metals.front();

    std::vector<std::string> metal_names;
    for (const auto& metal : metals) {
        metal_names.push_back(metal.GetName());
    }

    std::vector<Photon> photons;
    std::vector<Electron> electrons;

    float wave_length = 200;
    float intensity = 0.5;
    float stopping_voltage = 0.0;

    const sf::Color bg_color = sf::Color::White;

    sf::CircleShape light_source(300, 3);
    light_source.setPosition(sizes.width * 0.90, -100);
    light_source.rotate(52);
    light_source.setFillColor(sf::Color::Black);

    sf::RectangleShape cathode(sf::Vector2f(100, 700));
    cathode.setPosition(sizes.width * 0.05, sizes.height * 0.50);
    cathode.setFillColor(current_metal.GetColor());

    sf::RectangleShape anode(sf::Vector2f(100, 700));
    anode.setPosition(sizes.width * 0.92, sizes.height * 0.50);
    anode.setFillColor(sf::Color::Black);

    sf::Clock create_clock;

    sf::Clock delta_clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        sf::Time elapsed = delta_clock.restart();

        ImGui::SFML::Update(window, elapsed);

        ImGui::PushFont(font);

        ImGui::Begin("Настройки");

        ImGui::InputFloat("Длина волны", &wave_length, 1.0f, 10.0f, "%.0f");
        wave_length = std::clamp(wave_length, 200.0f, 860.0f);

        ImGui::SliderFloat("Интенсивность", &intensity, 0.0f, 1.0f, "%.2f");

        ImGui::SliderFloat(
            "Задерживающее напряжение", &stopping_voltage, -3.0f, 3.0f, "%.1f"
        );

        if (ImGui::BeginCombo("Металл", current_metal.GetName().c_str())) {
            for (size_t i = 0; i < metal_names.size(); i++) {
                bool is_selected = (current_metal.GetName() == metal_names[i]);

                if (ImGui::Selectable(metal_names[i].c_str(), is_selected)) {
                    current_metal = metals[i];
                    cathode.setFillColor(current_metal.GetColor());
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::End();

        ImGui::PopFont();

        window.clear(bg_color);

        window.draw(light_source);
        window.draw(cathode);
        window.draw(anode);

        if (create_clock.getElapsedTime().asSeconds() > 0.1 / intensity) {
            create_clock.restart();

            const float speed = kLightSpeed * kPhotonScale;
            const float offset = RandomInteger(-200, 200);
            const float frequency =
                kLightSpeed / (wave_length * std::pow(10, -9));
            float kinetic_energy = kPlankContant * frequency;

            Photon photon(GetWaveColor(wave_length), kinetic_energy, 10);
            photon.SetSpeed({-speed * 2.5f, speed});
            photon.SetPosition(
                sizes.width * 0.83 + offset, sizes.height * 0.25 + offset
            );

            photons.push_back(std::move(photon));
        }

        for (auto& p : photons) {
            if (p.Intersects(cathode.getGlobalBounds())) {
                float kinetic_energy = p.GetKinteticEnegery() -
                                       stopping_voltage * kElectronCharge -
                                       current_metal.GetWorkFunc();

                if (kinetic_energy > 0) {
                    const float speed =
                        std::sqrt(kinetic_energy * 2 / Electron::kMass) *
                        kElectronScale;

                    Electron electron(kinetic_energy, 10);
                    electron.SetPosition(p.GetPosition());
                    electron.SetSpeed({speed, 0});

                    electrons.push_back(std::move(electron));
                }
            }
        }

        std::erase_if(photons, [&](const auto& photon) {
            return photon.Intersects(cathode.getGlobalBounds());
        });

        for (auto& p : photons) {
            p.Update(elapsed);
            p.Draw(window);
        }

        std::erase_if(electrons, [&](const auto& electron) {
            return electron.Intersects(anode.getGlobalBounds()) ||
                   electron.GetSpeed() == sf::Vector2f{0, 0};
        });

        for (auto& e : electrons) {
            e.Update(elapsed);
            e.Draw(window);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
