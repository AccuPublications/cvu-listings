#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <numeric>
#include <ranges>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace sr = std::ranges;
using Prime_number_t = std::array<char, 5>;

bool is_colour_format(const auto & response)
{
  const std::regex r("^[bgy]{5}$");
  return std::regex_match(response, r);
}

int array_to_int(const auto a)
{
  const std::string number{a[0], a[1], a[2], a[3], a[4], a[5]};
  return std::stoi(number);
}

Prime_number_t int_to_char_array(const int i)
{
  const auto s{std::to_string(i)};
  return {s[0], s[1], s[2], s[3], s[4]};
}

bool is_prime(const int n)
{
  if (n == 2 || n == 3)
  {
    return true;
  }
  if (n <= 1 || n % 2 == 0 || n % 3 == 0)
  {
    return false;
  }
  for (int i{5}; i * i <= n; i += 6)
  {
    if (n % i == 0 || n % (i + 2) == 0)
    {
      return false;
    }
  }
  return true;
}

std::vector<Prime_number_t> create_primes()
{
  std::vector<Prime_number_t>  primes;
  for (int i{10'000}; i <= 99'999; ++i)
  {
    if (is_prime(i))
    {
      primes.emplace_back(int_to_char_array(i));
    }
  }
  return primes;
}

std::tuple<std::vector<std::pair<char, int>>,
           std::vector<std::pair<char, int>>,
           std::vector<std::pair<char, int>>>
get_number_categories(const auto & response, const auto & input)
{
  std::vector<std::pair<char, int>> black_numbers;
  std::vector<std::pair<char, int>> yellow_numbers;
  std::vector<std::pair<char, int>> green_numbers;

  for (int i{0}; i < std::ssize(response); ++i)
  {
    const auto character{response[i]};
    if (character == 'g')
    {
      green_numbers.emplace_back(input[i], i);
    }
    else if (character == 'y')
    {
      yellow_numbers.emplace_back(input[i], i);
    }
    else black_numbers.emplace_back(input[i], i);
  }
  return {black_numbers, yellow_numbers, green_numbers};
}

auto black_digits(auto & primes, const auto end, const auto & invalid_digits, const auto & green_digits, const auto & yellow_digits)
{
  auto criterion = [&](auto element)
  {
    // Collate all the black digits of the guess.
    std::set<char> black_digits_set;
    for (const auto & digit : invalid_digits)
    {
      black_digits_set.insert(digit.first);
    }

    // Collate all the yellow digits of the guess.
    std::set<char> yellow_digits_set;
    for (const auto & digit : yellow_digits)
    {
      yellow_digits_set.insert(digit.first);
    }

    // Collate all the green digits of the guess.
    std::set<char> green_digits_set;
    for (const auto & digit : green_digits)
    {
      green_digits_set.insert(digit.first);
    }

    std::set<char> combined_set;
    std::ranges::set_union(yellow_digits_set, green_digits_set, std::inserter(combined_set, combined_set.begin()));
    std::set<char> remover_set;
    std::ranges::set_difference(black_digits_set, combined_set, std::inserter(remover_set, remover_set.begin()));

    // Collate the digits of the prime number.
    std::set<char> prime_digits_set;
    for (const auto digit : element)
    {
      prime_digits_set.insert(digit);
    }

    // If any digits of the remover set are in the prime digits set, return true, else false.
    for (const auto a : prime_digits_set)
    {
      for (const auto b : remover_set)
      {
        if (a == b)
        {
          return true;
        }
      }
    }
    return false;
  };

  const auto [new_end, x] = sr::remove_if(primes.begin(), end, criterion);
  return new_end;
}

auto yellow_digits(auto & primes, const auto end, const auto & yellow_digits)
{
  // The yellow numbers cannot be in the yellow positions.
  const auto [new_end_0, last_0] = sr::remove_if(primes.begin(), end, [&](auto element)
  {
    for (int i{0}; i < std::ssize(yellow_digits); ++i)
    {
      if (yellow_digits[i].first == element[yellow_digits[i].second])
      {
        return true;
      }
    }
    return false;
  });

  // // The yellow numbers must be in a position other that the stated positions.
  const auto [new_end_1, x] = sr::remove_if(primes.begin(), new_end_0, [&](auto element)
    {
       std::multiset<char> yellow;
       for (int i{0}; i < std::ssize(yellow_digits); ++i)
        {
         yellow.insert(yellow_digits[i].first);
       }

       std::multiset<char> digits;
       for (int i{0}; i < std::ssize(element); ++i)
       {
         digits.insert(element[i]);
       }

       return !sr::includes(digits, yellow);
    });

  return new_end_1;
}

auto green_digits(auto & primes, const auto end, const auto & green_digits)
{
  // If any of the green digits are not in the number, return true, else false.
  const auto [new_end, x] = sr::remove_if(primes.begin(), end, [&](auto element)
  {
    for (int i{0}; i < std::ssize(green_digits); ++i)
    {
      if (element[green_digits[i].second] != green_digits[i].first)
      {
        return true;
      }
    }
    return false;
  });
  return new_end;
}

bool is_number(const auto & line)
{
  const std::regex r("^[1-9][[:digit:]]{4}$");
  return std::regex_match(line, r);
}

auto get_guess()
{
  Prime_number_t guess;
  bool correct_response{false};
  while (!correct_response)
  {
    std::string line;
    while (!correct_response)
    {
      std::cout << "Enter your prime number: ";
      std::getline(std::cin, line);
      correct_response = is_number(line);
    }

    for (int i{0}; i < 5; ++i)
    {
      guess[i] = line[i];
    }

    correct_response = is_prime(array_to_int(guess));
  }
  return guess;
}

auto get_colours()
{
  std::string colour_response;
  bool correct_response{false};
  while (!correct_response)
  {
    std::cout << "Enter Primel's colour response, example bbbgyb: ";
    std::getline(std::cin, colour_response);
    correct_response = is_colour_format(colour_response);
  }
  return colour_response;
}

void primel()
{
  std::cout << "Welcome to Primel assistant!\nPlease enter a 5-digit prime number.\n";
  int suggestion_number{};
  auto primes{create_primes()};
  auto end{primes.end()};

  bool game_over{};
  while (!game_over)
  {
    const std::array<std::pair<std::string, int>, 2> starting_suggestions({{"first", 20431}, {"second", 56897}});
    if (suggestion_number == 1)
    {
      if (std::ranges::find(primes.begin(), end, int_to_char_array(starting_suggestions[1].second)) == primes.end())
      {
         suggestion_number = 2;
      }
    }
    if (suggestion_number < 2)
    {
      std::cout << std::format("I recommend {} as your {} guess.\n", starting_suggestions[suggestion_number].second, starting_suggestions[suggestion_number].first);
      ++suggestion_number;
    }

    const auto guess{get_guess()};

    if (const auto colour_response{get_colours()}; colour_response == "ggggg")
    {
      std::cout << "You win!\n";
      game_over = true;
    }
    else
    {
      auto [black, yellow, green]{get_number_categories(colour_response, guess)};

      // Remove incompatible primes.
      end = yellow_digits(primes, end, yellow);
      end = black_digits(primes, end, black, green, yellow);
      end = green_digits(primes, end, green);

      // Remove the guess from the primes.
      auto [new_end, x] = std::ranges::remove(primes.begin(), end, guess);
      end = new_end;

      if (std::distance(primes.begin(), end) > 0)
      {
        if (suggestion_number > 1)
        {
          std::cout << "I suggest your next guess is ";
          sr::for_each(primes[0], [](const auto d){std::cout << d;});
          std::cout << '\n';
        }
      }
      else
      {
        std::cout << "You loose!\n";
        game_over = true;
      }
    }
  }
  std::cout << "Game over!\n";
}

bool is_coloured_digit_format(const std::string & digits)
{
  // The input format is
  // pvcpvcpvc or pvcpvcpvcpvc
  // Where p is the position in the range 0 to 4,
  // v is the value in the range 0 to 9 and
  // c is the colour of the digit, either y (yellow) or g (green).
  // For example, 02y13y25y, as  per the Challenge.
  const std::regex r("^([0-4][0-9]['g'|'y']){3,4}$");
  if (!std::regex_match(digits, r))
  {
    return false;
  }

  // Detect attempts to assign a digit a colour more than once.
  if (digits[0] == digits[3] || digits[3] == digits[6])
  {
    return false;
  }
  if (digits.size() == 12 && digits[6] == digits[9])
  {
    return false;
  }
  return true; // The user's input is correctly formatted.
}

std::vector<std::tuple<int, char, char>> get_coloured_digits()
{
  std::string coloured_digits;
  bool correct_response{false};
  while (!correct_response)
  {
    std::cout << "Enter the coloured digits, example 02y13y25y: ";
    std::getline(std::cin, coloured_digits);
    correct_response = is_coloured_digit_format(coloured_digits);
  }

  std::vector<std::tuple<int, char, char>> digits;
  for (int i{0}; i < std::ssize(coloured_digits); i += 3)
  {
    auto d{std::stoi({std::string{coloured_digits[i]}})};
    digits.emplace_back(d, coloured_digits[i + 1], coloured_digits[i + 2]);
  }
  return digits;
}

void challenge()
{
  std::vector<Prime_number_t> numbers;
  for (auto i{10'000}; i <= 99'999; ++i)
  {
    numbers.push_back(int_to_char_array(i));
  }

  const auto criterion = [](auto n)
    {
      const auto sum{std::accumulate(n.begin(), n.end(), 0, [](const char a, const char b)
      {
        return std::stoi(std::string{b}) + a;
      })};

      // The numbers start at 10,000 so there is no need to check for a leading zero.
      return (n[4] != '1' && n[4] != '3' && n[4] != '7' && n[4] != '9') ||
              sum % 3 == 0;
    };

  auto [end, x] = sr::remove_if(numbers, criterion);

  const auto coloured_digits{get_coloured_digits()};

  // Obtain the black numbers.
  std::vector<std::pair<char, int>> black{{'0', 0}, {'1', 0}, {'2', 0}, {'3', 0}, {'4', 0}, {'5', 0}, {'6', 0}, {'7', 0}, {'8', 0}, {'9', 0}};
  auto [black_end, xx] = sr::remove_if(black, [&](const auto c)
  {
    for (int i{0}; i < std::ssize(coloured_digits); ++i)
    {
      if (std::get<1>(coloured_digits[i]) == c.first)
      {
        return true;
      }
    }
    return false;
  });

  black.erase(black_end, black.end());

  // Obtain the yellow numbers.
  std::vector<std::pair<char, int>> yellow;
  for (int i{0}; i < std::ssize(coloured_digits); ++i)
  {
    if (get<2>(coloured_digits[i]) == 'y')
    {
      yellow.emplace_back(get<1>(coloured_digits[i]), std::get<0>(coloured_digits[i]));
    }
  }

  // Obtain the green numbers.
  std::vector<std::pair<char, int>> green;
  for (int i{0}; i < std::ssize(coloured_digits); ++i)
  {
    if (get<2>(coloured_digits[i]) == 'g')
    {
      green.emplace_back(get<1>(coloured_digits[i]), std::get<0>(coloured_digits[i]));
    }
  }

  end = black_digits(numbers, end, black, green, yellow);
  end = yellow_digits(numbers, end, yellow);
  end = green_digits(numbers, end, green);

  std::for_each(numbers.begin(), end, [](auto n){std::cout << std::format("{}{}{}{}{}\n", n[0], n[1], n[2], n[3], n[4]);});

  sr::for_each(numbers.begin(), end, [](auto & d){sr::sort(d);});
  sr::sort(numbers.begin(), end);
  end = std::unique(numbers.begin(), end);

  for (auto i{numbers.begin()}; i != end; ++i)
  {
    std::cout << std::format("[{},{},{},{},{}]\n", (*i)[0], (*i)[1], (*i)[2], (*i)[3], (*i)[4]);
  }
}
int main()
{
  challenge();
  // primel();
}
