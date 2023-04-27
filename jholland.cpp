#include <fstream>
#include <SFML/Graphics.hpp>

class Termite {
public:
  explicit Termite(sf::RenderWindow & new_window, std::string description) : window(new_window)
  {
    std::ranges::transform(description, description.begin(), [](char c){return isdigit(c) ? c : ' ';});

    std::array<int, 12> spec;
    size_t index = 0;
    size_t processed = 0;
    for (auto & e : spec)
    {
      e = std::stoi(&description[index], &processed);
      index += processed;
    }

    populate_rules_table(false, false, spec[0], spec[1],  spec[2]);
    populate_rules_table(true,  false, spec[3], spec[4],  spec[5]);
    populate_rules_table(false, true,  spec[6], spec[7],  spec[8]);
    populate_rules_table(true,  true,  spec[9], spec[10], spec[11]);

    populate_change_course_map(Heading::north, Turn::no_turn,    Heading::north, &Termite::decrement_y);
    populate_change_course_map(Heading::north, Turn::right_turn, Heading::east,  &Termite::increment_x);
    populate_change_course_map(Heading::north, Turn::left_turn,  Heading::west,  &Termite::decrement_x);
    populate_change_course_map(Heading::north, Turn::u_turn,     Heading::south, &Termite::increment_y);
    populate_change_course_map(Heading::south, Turn::no_turn,    Heading::south, &Termite::increment_y);
    populate_change_course_map(Heading::south, Turn::right_turn, Heading::west,  &Termite::decrement_x);
    populate_change_course_map(Heading::south, Turn::left_turn,  Heading::east,  &Termite::increment_x);
    populate_change_course_map(Heading::south, Turn::u_turn,     Heading::north, &Termite::decrement_y);
    populate_change_course_map(Heading::east,  Turn::no_turn,    Heading::east,  &Termite::increment_x);
    populate_change_course_map(Heading::east,  Turn::right_turn, Heading::south, &Termite::increment_y);
    populate_change_course_map(Heading::east,  Turn::left_turn,  Heading::north, &Termite::decrement_y);
    populate_change_course_map(Heading::east,  Turn::u_turn,     Heading::west,  &Termite::decrement_x);
    populate_change_course_map(Heading::west,  Turn::no_turn,    Heading::west,  &Termite::decrement_x);
    populate_change_course_map(Heading::west,  Turn::right_turn, Heading::north, &Termite::decrement_y);
    populate_change_course_map(Heading::west,  Turn::left_turn,  Heading::south, &Termite::increment_y);
    populate_change_course_map(Heading::west,  Turn::u_turn,     Heading::east,  &Termite::increment_x);
  }
  void move() {
    Current_state current_state{pixel_array[termite_x][termite_y], state};
    const auto action = rules_table.find(current_state);
    state = action->second.state;
    illuminate_pixel(action->second.colour);
    Required_change change{heading, action->second.direction};
    const auto new_heading = change_course.find(change);
    heading = new_heading->second.heading;
    (this->*new_heading->second.change_position)();
  }
  const static auto grid_size_x = 200;
  const static auto grid_size_y = 160;
  const static auto dot_size = 6;
private:
  enum class Turn {no_turn = 1, right_turn = 2, u_turn = 4, left_turn = 8};
  enum class Heading {north, south, east, west};

  void illuminate_pixel(const bool illumination) {
    sf::RectangleShape dot;
    dot.setSize({dot_size, dot_size});
    dot.setFillColor(sf::Color::White);

    for (int y = 0; y < grid_size_y; ++y) {
      for (int x = 0; x < grid_size_x; ++x) {
        if (pixel_array[x][y]) {
          dot.setPosition(static_cast<float>(dot_size * x),
                 static_cast<float>(dot_size * y));
          window.draw(dot);
        }
      }
    }
    pixel_array[termite_x][termite_y] = illumination;
    dot.setPosition(static_cast<float>(dot_size * termite_x),
           static_cast<float>(dot_size * termite_y));
    dot.setFillColor(sf::Color::Red);
    window.draw(dot);
  }

  struct Current_state   {
    bool state;
    bool colour;

    bool operator<(const Current_state & other) const {
      const auto value = (state ? 1 : 0) + 2 * (colour ? 1 : 0);
      const auto other_value = 
              (other.state ? 1 : 0) + 2 * (other.colour ? 1 : 0);
      return value < other_value;
    }
  };

  struct Action {
    bool colour;
    Turn direction;
    bool state;
  };

  sf::RenderWindow & window;
  bool state = false;
  Heading heading = Heading::north;

  size_t termite_x = grid_size_x / 2;
  size_t termite_y = grid_size_y / 2;
  std::array<std::array<bool, grid_size_y>, grid_size_x> 
                       pixel_array{{{false}}};
  std::map<Current_state, Action> rules_table;

  void populate_rules_table(const bool present_state, const bool colour,
        const int new_colour, const int direction, const int new_state) {
    const Current_state current_state {present_state, colour};
    const Action action{new_colour == 1, static_cast<Turn>(direction), 
                               new_state == 1};
    rules_table.insert({current_state, action});
  }

  void populate_change_course_map(const Heading present_heading, 
                     const Turn turn, const Heading new_heading, 
                     void (Termite::* change_position)())  {
    const Required_change change{present_heading, turn};
    const Course course{new_heading, change_position};
    change_course.try_emplace(change, course);
  }

  struct Required_change  {
    Heading heading;
    Turn required_turn;
    bool operator<(const Required_change & other) const  {
      const auto value = 16 * static_cast<int>(heading) 
                      + static_cast<int>(required_turn);
      const auto other_value = 16 * static_cast<int>(other.heading) 
                      + static_cast<int>(other.required_turn);
      return value < other_value;
    }
  };

  void increment_x()  {
    ++termite_x;
    if (termite_x >= grid_size_x) termite_x = 0;
  }
  void decrement_x()  {
    --termite_x;
    if (termite_x >= grid_size_x) termite_x = grid_size_x - 1;
  }
  void increment_y() {
    ++termite_y;
    if (termite_y >= grid_size_y) termite_y = 0;
  }
  void decrement_y() {
    --termite_y;
    if (termite_y >= grid_size_y) termite_y = grid_size_y - 1;
  }
  struct Course {
    Heading heading;
    void (Termite::* change_position)();
  };

  std::map<Required_change, Course> change_course;
};

std::string read_specification_from_file(){
  std::string line;
  if (std::ifstream input_file("termites.txt"); input_file.is_open()){
    while (std::getline(input_file, line) && line.starts_with("//"));
  }
  const auto position = line.find("//");
  return line.substr(0, position);
}

int main(){
  sf::RenderWindow window(sf::VideoMode(Termite::grid_size_x * Termite::dot_size, Termite::grid_size_y * Termite::dot_size), "Termite");
  window.setFramerateLimit(60);
  const std::string s = read_specification_from_file();
  Termite termite(window, s);
  auto iteration = 0;
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)){
      if (event.type == sf::Event::Closed)
        window.close();
    }
    if (iteration < 27'300) {
      ++iteration;
      window.clear();
      termite.move();
    }
    window.display();
  }
}
