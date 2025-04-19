#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <SFML/Graphics.hpp>

#include <array>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>

constexpr auto & afs_colour {sf::Color::Blue};
constexpr auto & afd_colour {sf::Color::Cyan};
constexpr auto & ass_colour {sf::Color::Green};
constexpr auto & asd_colour {sf::Color::Magenta};
constexpr auto & pfs_colour {sf::Color::Red};
constexpr auto & pfd_colour {sf::Color::Yellow};
constexpr auto & pss_colour {sf::Color::Black};
constexpr auto & psd_colour {sf::Color::White};

constexpr auto max_value {200};
constexpr auto number_of_lines {11};
constexpr auto each_division {max_value / (number_of_lines - 1)};

struct Measurement
{
  float systolic;
  float diastolic;
};

struct Data_sets
{
  // Two measurements are made, five minutes apart.
  Measurement first;
  Measurement second;
};

struct Daily_data
{
  Data_sets am;
  Data_sets pm;
};

const auto width {sf::VideoMode::getDesktopMode().width};
const auto height {sf::VideoMode::getDesktopMode().height};

constexpr auto top_margin {50.0f};
constexpr auto bottom_margin {250.0f};
constexpr auto left_margin {140.0f};
constexpr auto right_margin {100.0f};
constexpr auto max_scale_y_value {200.0f};

constexpr auto graph_x {left_margin};
constexpr auto graph_y {top_margin};
const auto graph_width {width - right_margin - left_margin};
const auto graph_height {height - bottom_margin - top_margin};

void draw_vertical_lines(sf::RenderWindow & window)
{
  std::array<sf::Vertex, 60> vertical_line;
  for (int i {0}; i < 60; i += 2)
  {
    vertical_line[0 + i] = sf::Vertex{sf::Vector2f(i/2.0f * graph_width / 29.0f + left_margin, height - bottom_margin)};
    vertical_line[1 + i] = sf::Vertex{sf::Vector2f(i/2.0f * graph_width / 29.0f + left_margin, top_margin)};
  }
  window.draw(vertical_line.data(), vertical_line.size(), sf::PrimitiveType::Lines);
}
void draw_horizontal_lines(sf::RenderWindow & window)
{
  std::array<sf::Vertex, 2 * number_of_lines> horizontal_line;
  for (int i {0}; i < 2 * number_of_lines; i += 2)
  {
    const auto y {i * graph_height / (2 * (number_of_lines - 1)) + top_margin};
    horizontal_line[0 + i] = sf::Vertex{sf::Vector2f(left_margin, y)};
    horizontal_line[1 + i] = sf::Vertex{sf::Vector2f(left_margin + graph_width, y)};
  }
  window.draw(horizontal_line.data(), horizontal_line.size(), sf::PrimitiveType::Lines);
}
void draw_vertical_scale(sf::RenderWindow & window, const sf::Font & font)
{
  for (int i {0}; i < number_of_lines; ++i)
  {
    sf::Text text(std::to_string((number_of_lines - i - 1) * each_division), font);
    const auto text_bounds {text.getGlobalBounds()};
    text.setOrigin(text_bounds.width + 10, text_bounds.height);
    text.setPosition(left_margin, i * graph_height / (number_of_lines - 1) + top_margin);
    window.draw(text);
  }

  sf::Text text("Blood Pressure", font);
  text.setRotation(-90);
  auto text_bounds {text.getGlobalBounds()};
  text.setOrigin(text_bounds.height / 2, text_bounds.width / 2);
  text.setPosition(15, graph_height / 2 + top_margin);
  window.draw(text);

  text.setString("(mm Hg)");
  text.setRotation(-90);
  text_bounds = text.getGlobalBounds();
  text.setOrigin(text_bounds.height / 2, text_bounds.width / 2);
  text.setPosition(50, graph_height / 2 + top_margin);
  window.draw(text);
}
void draw_horizontal_scale(sf::RenderWindow & window, const sf::Font & font)
{
  for (int i {0}; i < 30; ++i)
  {
    sf::Text text(std::to_string(i), font);
    const auto text_bounds {text.getGlobalBounds()};
    text.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
    text.setPosition((30 - i - 1) * graph_width / 29 + left_margin, graph_height + top_margin + 10);
    window.draw(text);
  }

  sf::Text text("Days ago", font);
  const auto text_bounds {text.getGlobalBounds()};
  text.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
  text.setPosition(graph_width / 8 + left_margin, graph_height + top_margin + 60);
  window.draw(text);
}
void draw_graticule(sf::RenderWindow & window, const sf::Font & font)
{
  draw_vertical_lines(window);
  draw_horizontal_lines(window);
  draw_vertical_scale(window, font);
  draw_horizontal_scale(window, font);
}
void draw_key_symbol(sf::RenderWindow & window, sf::CircleShape shape, const float x, const float y, const sf::Color colour)
{
  shape.setPosition(x, y);
  shape.setFillColor(colour);
  shape.setOrigin(shape.getRadius(), shape.getRadius());
  window.draw(shape);
}
void draw_key_text(sf::RenderWindow & window, const float x, const float y, sf::Text text)
{
  const auto text_bounds {text.getGlobalBounds()};
  text.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
  text.setPosition(x, y);
  window.draw(text);
}
void draw_key_legend(sf::RenderWindow & window, const sf::Font & font, const float x, const float y, const sf::String & text_1, const sf::String & text_2, const sf::String & text_3)
{
  constexpr auto spacing {30.0f};
  sf::Text text(text_1, font);
  draw_key_text(window, x, y + 0 * spacing, text);

  text.setString(text_2);
  draw_key_text(window, x, y + 1 * spacing, text);

  text.setString(text_3);
  draw_key_text(window, x, y + 2 * spacing, text);
}
void draw_key(sf::RenderWindow & window, const sf::Font & font)
{
  constexpr auto gap {150.0f};
  constexpr auto x {700.0f};
  const auto y {graph_height + bottom_margin / 2.0f - 20.0f};
  const sf::CircleShape shape(9.0f);

  draw_key_symbol(window, shape, x + 0 * gap, y, afs_colour);
  draw_key_symbol(window, shape, x + 1 * gap, y, afd_colour);
  draw_key_symbol(window, shape, x + 2 * gap, y, ass_colour);
  draw_key_symbol(window, shape, x + 3 * gap, y, asd_colour);
  draw_key_symbol(window, shape, x + 4 * gap, y, pfs_colour);
  draw_key_symbol(window, shape, x + 5 * gap, y, pfd_colour);
  draw_key_symbol(window, shape, x + 6 * gap, y, pss_colour);
  draw_key_symbol(window, shape, x + 7 * gap, y, psd_colour);

  constexpr auto morning {"Morning"};
  constexpr auto evening {"Evening"};
  constexpr auto first {"first"};
  constexpr auto second {"second"};
  constexpr auto systolic {"systolic"};
  constexpr auto diastolic {"diastolic"};

  draw_key_legend(window, font, x + 0 * gap, y + 30, morning, first, systolic);
  draw_key_legend(window, font, x + 1 * gap, y + 30, morning, first, diastolic);
  draw_key_legend(window, font, x + 2 * gap, y + 30, morning, second, systolic);
  draw_key_legend(window, font, x + 3 * gap, y + 30, morning, second, diastolic);
  draw_key_legend(window, font, x + 4 * gap, y + 30, evening, first, systolic);
  draw_key_legend(window, font, x + 5 * gap, y + 30, evening, first, diastolic);
  draw_key_legend(window, font, x + 6 * gap, y + 30, evening, second, systolic);
  draw_key_legend(window, font, x + 7 * gap, y + 30, evening, second, diastolic);
}
void draw_graph(sf::RenderWindow & window, const sf::Font & font)
{
  sf::RectangleShape rectangle({graph_width, graph_height});
  rectangle.setPosition({left_margin, top_margin});
  rectangle.setFillColor({75, 75, 75});
  window.draw(rectangle);
  draw_graticule(window, font);
  draw_key(window, font);
}
std::pair<float, float> value_to_screen(const size_t day, const float y)
{
  const auto screen_x {left_margin - 1 + graph_width - day * graph_width / 29.0f};
  const auto screen_y {top_margin + (max_scale_y_value - y) * graph_height / max_scale_y_value};
  return {screen_x, screen_y};
}
void display_data_point(sf::RenderWindow & window, sf::CircleShape dot, const size_t day, const float value, const sf::Color colour)
{
  dot.setFillColor(colour);
  const auto [afsx, afsy] {value_to_screen(day, value)};
  dot.setPosition(afsx , afsy);
  window.draw(dot);
}
void calculate_mean(std::vector<Daily_data> data, size_t mean_start, size_t mean_end, auto how, std::vector<sf::Vertex> & average_line, sf::Color colour)
{
  const auto mean {std::accumulate(data.begin() + mean_start, data.begin() + mean_end + 1, 0.0f, how) / (mean_end - mean_start + 1)};
  const auto point_1 {value_to_screen(data.size() - mean_end - 1, mean)};
  average_line.emplace_back(sf::Vector2f(point_1.first, point_1.second), colour);

  if (average_line.size() > 1)
  {
    average_line.push_back(average_line[average_line.size() - 1]);
  }
}
void display_mean_lines(sf::RenderWindow & window, const std::vector<Daily_data> & data)
{
  std::vector<sf::Vertex> average_afs_line;
  std::vector<sf::Vertex> average_afd_line;
  std::vector<sf::Vertex> average_ass_line;
  std::vector<sf::Vertex> average_asd_line;
  std::vector<sf::Vertex> average_pfs_line;
  std::vector<sf::Vertex> average_pfd_line;
  std::vector<sf::Vertex> average_pss_line;
  std::vector<sf::Vertex> average_psd_line;

  auto mean_end {data.size() > 30 ? data.size() - 30 : 0};
  auto mean_start {data.size() >= 30 + 7 - 1 ? mean_end - 7 + 1 : mean_end};
  while (mean_end < data.size())
  {
    const auto afs_how {[](const auto sum, const auto curr){return sum + curr.am.first.systolic;}};
    calculate_mean(data, mean_start, mean_end, afs_how, average_afs_line, afs_colour);

    const auto afd_how {[](const auto sum, const auto curr){return sum + curr.am.first.diastolic;}};
    calculate_mean(data, mean_start, mean_end, afd_how, average_afd_line, afd_colour);

    const auto ass_how {[](const auto sum, const auto curr){return sum + curr.am.second.systolic;}};
    calculate_mean(data, mean_start, mean_end, ass_how, average_ass_line, ass_colour);

    const auto asd_how {[](const auto sum, const auto curr){return sum + curr.am.second.diastolic;}};
    calculate_mean(data, mean_start, mean_end, asd_how, average_asd_line, asd_colour);

    const auto pfs_how {[](const auto sum, const auto curr){return sum + curr.pm.first.systolic;}};
    calculate_mean(data, mean_start, mean_end, pfs_how, average_pfs_line, pfs_colour);

    const auto pfd_how {[](const auto sum, const auto curr){return sum + curr.pm.first.diastolic;}};
    calculate_mean(data, mean_start, mean_end, pfd_how, average_pfd_line, pfd_colour);

    const auto pss_how {[](const auto sum, const auto curr){return sum + curr.pm.second.systolic;}};
    calculate_mean(data, mean_start, mean_end, pss_how, average_pss_line, pss_colour);

    const auto psd_how {[](const auto sum, const auto curr){return sum + curr.pm.second.diastolic;}};
    calculate_mean(data, mean_start, mean_end, psd_how, average_psd_line, psd_colour);

    if (mean_end - mean_start == 7 - 1)
    {
      ++mean_start;
    }
    ++mean_end;
  }
  window.draw(average_afs_line.data(), average_afs_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_afd_line.data(), average_afd_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_ass_line.data(), average_ass_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_asd_line.data(), average_asd_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_pfs_line.data(), average_pfs_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_pfd_line.data(), average_pfd_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_pss_line.data(), average_pss_line.size(), sf::PrimitiveType::Lines);
  window.draw(average_psd_line.data(), average_psd_line.size(), sf::PrimitiveType::Lines);
}
std::string read_from_file(std::vector<Daily_data> & data)
{
  std::string filename{"data.json"};
  std::ifstream file("../" + filename);
  if (!file.is_open())
  {
    return "File '" + filename + "' not found.";
  }

  try
  {
    boost::property_tree::ptree property_tree;
    read_json(file, property_tree);
    for (const auto & node : std::views::values(property_tree.get_child("measurements")))
    {
      const Measurement mf {node.get<float>("morning.first.systolic"), node.get<float>("morning.first.diastolic")};
      const Measurement ms {node.get<float>("morning.second.systolic"), node.get<float>("morning.second.diastolic")};
      const Data_sets morning {mf, ms};

      const Measurement af {node.get<float>("evening.first.systolic"), node.get<float>("evening.first.diastolic")};
      const Measurement as {node.get<float>("evening.second.systolic"), node.get<float>("evening.second.diastolic")};
      const Data_sets evening {af, as};

      const Daily_data daily_data {morning, evening};
      data.push_back(daily_data);
    }
  }
  catch (const boost::property_tree::ptree_error & e)
  {
    return e.what();
  }
  return {};
}
void draw_data(sf::RenderWindow & window, const std::vector<Daily_data> & data)
{
  constexpr auto radius {5.0f};
  sf::CircleShape shape(radius);
  shape.setOrigin(shape.getRadius(), shape.getRadius());
  const auto start {data.size() > 30 ? data.size() - 30 : 0};
  const auto start_day {data.size() > 30 ? 29 : data.size() - 1};
  for (auto index {start}; index < data.size(); ++index)
  {
    const auto day {start_day - index + start};
    display_data_point(window, shape, day, data[index].am.first.systolic, afs_colour);
    display_data_point(window, shape, day, data[index].am.first.diastolic, afd_colour);
    display_data_point(window, shape, day, data[index].am.second.systolic, ass_colour);
    display_data_point(window, shape, day, data[index].am.second.diastolic, asd_colour);
    display_data_point(window, shape, day, data[index].pm.first.systolic, pfs_colour);
    display_data_point(window, shape, day, data[index].pm.first.diastolic, pfd_colour);
    display_data_point(window, shape, day, data[index].pm.second.systolic, pss_colour);
    display_data_point(window, shape, day, data[index].pm.second.diastolic, psd_colour);
  }
  display_mean_lines(window, data);
}
void draw(sf::RenderWindow & window, const sf::Font & font, const std::vector<Daily_data> & data)
{
  draw_graph(window, font);
  draw_data(window, data);
}
int main()
{
  std::vector<Daily_data> blood_pressure_data_data;
  if (const auto result {read_from_file(blood_pressure_data_data)}; !result.empty())
  {
    std::cout << result << '\n';
    return -1;
  }
  sf::Font font;
  if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"))
  {
    std::cout << "Font file not found.\n";
    return -1;
  }

  sf::RenderWindow window(sf::VideoMode(width, height), "Blood Pressure");
  window.setFramerateLimit(2);

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }
    window.clear({50, 50, 75}); // Dark blue.
    draw(window, font, blood_pressure_data_data);
    window.display();
  }
}
