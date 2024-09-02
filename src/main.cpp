
#include <SFML/Graphics.hpp>
#include <optional>
#include <random>
#include <vector>

#define USE_VERTEX_ARRAY true

// const sf::Vector2u WINDOW_SIZE = {2560, 1440};
const sf::Vector2u WINDOW_SIZE = {1920, 1080};
const sf::Vector2f WINDOW_SIZE_F = static_cast<sf::Vector2f>(WINDOW_SIZE);
const uint32_t MAX_FRAME_RATE = 144;
const float DELTA_TIME = 1.0f / static_cast<float>(MAX_FRAME_RATE);

const uint32_t STAR_COUNT = 10000;
const float STAR_RADIUS = 20.0f;
const float NEAR_DISTANCE = 0.1f;
const float FAR_DISTANCE = 10.0f;
const float STAR_SPEED = 5.0f;

struct Star {
  sf::Vector2f position;
  float z = 1.0f;
};

static std::vector<Star> CreateStars(uint32_t count, float scale) {
  std::vector<Star> stars;
  stars.reserve(count);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  for (auto i = 0; i < count; ++i) {
    float x = (dist(gen) - 0.5f) * WINDOW_SIZE_F.x * scale;
    float y = (dist(gen) - 0.5f) * WINDOW_SIZE_F.y * scale;
    float z = dist(gen) * (FAR_DISTANCE - NEAR_DISTANCE) + NEAR_DISTANCE;
    stars.push_back(Star{{x, y}, z});
  }

  std::sort(stars.begin(), stars.end(),
            [](const Star& s1, const Star& s2) { return s1.z > s2.z; });

  return stars;
}

int main() {
  auto window =
      sf::RenderWindow{{WINDOW_SIZE.x, WINDOW_SIZE.y},
                       "CMake SFML Project" /*, sf::Style::Fullscreen */};
  window.setFramerateLimit(MAX_FRAME_RATE);

  auto stars = CreateStars(STAR_COUNT, FAR_DISTANCE);

  bool paused = false;

  uint32_t first = 0;

#if USE_VERTEX_ARRAY
  sf::VertexArray vertices{sf::PrimitiveType::Quads, 4 * STAR_COUNT};
#endif

  while (window.isOpen()) {
    for (auto event = sf::Event{}; window.pollEvent(event);) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space) {
          paused = true;
        }
      }
      if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Space) {
          paused = false;
        }
      }
    }

    std::optional<uint32_t> first_moved = std::nullopt;
    for (uint32_t i = 0; i < stars.size() && !paused; ++i) {
      auto& s = stars[i];
      s.z -= STAR_SPEED * DELTA_TIME;
      if (s.z <= NEAR_DISTANCE) {
        s.z = FAR_DISTANCE - (NEAR_DISTANCE - s.z);
        if (!first_moved.has_value()) {
          first_moved = i;
        }
      }
    }
    first = first_moved.value_or(first);

    window.clear();

    for (uint32_t i = 0; i < stars.size(); i++) {
      uint32_t index = (static_cast<size_t>(first) + i) % stars.size();
      const auto& s = stars[index];

      {
        float scale = 1.0f / s.z;
        float depth_ratio =
            (s.z - NEAR_DISTANCE) / (FAR_DISTANCE - NEAR_DISTANCE);
        float color_ratio = 1.0f - depth_ratio;
        auto color = static_cast<uint8_t>(color_ratio * 255.0f);

        auto p = s.position * scale;
        float r = STAR_RADIUS * scale;
        uint32_t j = i * 4;
        sf::Color c {color, color, color};

#if USE_VERTEX_ARRAY
        vertices[j + 0].position = {p.x - r, p.y - r};
        vertices[j + 1].position = {p.x + r, p.y - r};
        vertices[j + 2].position = {p.x + r, p.y + r};
        vertices[j + 3].position = {p.x - r, p.y + r};

        vertices[j + 0].color = c;
        vertices[j + 1].color = c;
        vertices[j + 2].color = c;
        vertices[j + 3].color = c;
#else
        sf::CircleShape circle{r};
        circle.setPosition(p + WINDOW_SIZE_F * 0.5f);
        circle.setFillColor(c);
        window.draw(circle);
#endif
      }
    }

#if USE_VERTEX_ARRAY
    sf::Transform translate;
    translate.translate(WINDOW_SIZE_F * 0.5f);
    window.draw(vertices, translate);
#endif

    window.display();
  }
}
