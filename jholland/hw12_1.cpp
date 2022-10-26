#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>

enum class instruction_code
{
  in, out, print, line,
  load, load_direct, load_indirect, store, add, add_direct, add_indirect,
  subtract, subtract_direct, subtract_indirect,
  multiply, multiply_direct, multiply_indirect, divide, divide_direct, divide_indirect, jump, jineg, jizero, halt
};

std::vector<std::pair<instruction_code, int>> program;
std::map<std::string, std::pair<int, std::vector<int>>> labels;
std::vector<std::pair<std::string, int32_t>> named_storage;
std::vector<std::string> literal_texts;
int32_t accumulator = 0;
//////////////////////////////////////////////////////////////////////////////////////////////
bool is_identifier(const std::string &string)
{
  std::regex pattern("[_[:alpha:]][_[:alnum:]]*");
  return std::regex_match(string, pattern);
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool is_integer(const std::string &string)
{
  std::regex pattern("[+-]?[_[:digit:]]*");
  return std::regex_match(string, pattern);
}

//////////////////////////////////////////////////////////////////////////////////////////////
std::pair<char, std::filesystem::path> parse(const std::string &text)
{
  // Remove redundant spaces from the text.
  std::string command = std::regex_replace(text, std::regex("^ +| +$|( ) +"), "$1");
  if (command.empty())
  {
    return {'b', ""}; // blank line.
  }
  if (command.length() == 1 && command[0] != 'l')
  {
    return {command[0], ""};
  }
  if (command.length() == 1)
  {
    std::cout << "Missing filename.\n";
    return {'b', ""};
  }
  else if (command[1] != ' ')
  {
    return {'?', ""};
  }
  const auto position = command.find_first_not_of(' ', 1);
  std::filesystem::path path{command.substr(position).append(".cesil")};
  if (!is_regular_file(path) || !exists(path))
  {
    std::cout << "No such file.\n";
    return {'b', ""};
  }
  return {'l', path};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int get_store_index(const std::string &store_name)
{
  // Assume storage_name is a valid number or a valid identifier.
  size_t index = 0;
  while (index < named_storage.size() && named_storage[index].first != store_name)
  {
    ++index;
  }

  if (index == named_storage.size())
  {
    // The storage location does not exist, so create one.
    named_storage.push_back({store_name, 0});
  }
  return static_cast<int>(index);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void
assemble(const std::string &data, const instruction_code direct, const instruction_code indirect, const bool identifier)
{
  if (identifier)
  {
    const auto value = get_store_index(data);
    program.push_back({indirect, {value}});
  }
  else
  {
    program.push_back({direct, {std::stoi(data)}});
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////
bool
check_and_assemble(const std::string &instruction_data, const instruction_code direct, const instruction_code indirect)
{
  if (const auto identifier = is_identifier(instruction_data); identifier || is_integer(instruction_data))
  {
    assemble(instruction_data, direct, indirect, identifier);
    return true;
  }
  return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////
bool analyse_file(const std::filesystem::path &filename)
{
  const std::map<std::string, instruction_code> instructions = {{"IN",       instruction_code::in},
                                                                {"OUT",      instruction_code::out},
                                                                {"PRINT",    instruction_code::print},
                                                                {"LINE",     instruction_code::line},
                                                                {"LOAD",     instruction_code::load},
                                                                {"STORE",    instruction_code::store},
                                                                {"ADD",      instruction_code::add},
                                                                {"SUBTRACT", instruction_code::subtract},
                                                                {"MULTIPLY", instruction_code::multiply},
                                                                {"DIVIDE",   instruction_code::divide},
                                                                {"JUMP",     instruction_code::jump},
                                                                {"JINEG",    instruction_code::jineg},
                                                                {"JIZERO",   instruction_code::jizero},
                                                                {"HALT",     instruction_code::halt}};
  program.clear();
  named_storage.clear();
  labels.clear();
  literal_texts.clear();
  auto error = false;
  int line_number = 0;
  int instruction_number = -1;
  std::ifstream file_stream(filename.string());
  std::string line;
  while (std::getline(file_stream, line))
  {
    ++line_number;
    if (!line.empty())
    {
      std::istringstream input_stream(line);
      std::string label;
      std::string instruction;
      std::string instruction_data;
      if (line[0] == ' ' || line[0] == '\t')
      {
        input_stream >> instruction;
      }
      else
      {
        input_stream >> label >> instruction;
      }

      // Ignore lines that contain only white space.
      if (instruction.empty())
      {
        continue;
      }

      ++instruction_number;
      input_stream >> std::quoted(instruction_data);

      if (!label.empty())
      {
        if (!is_identifier(label))
        {
          std::cout << "The label '" << label << "' on line " << line_number << " is not of the correct format.\n";
          error = true;
          continue;
        }
        else
        {
          // The instruction is labelled. Discover whether it is in the list of labels. If it is in the table
          // it is a duplicate and so is an error. If it is not in the table, add the name of the label to the
          // table along with the program line number.
          const auto label_location = labels.find(label);
          if (label_location != labels.end() && label_location->second.first != -1)
          {
            error = true;
            std::cout << "Duplicate Label " << label << " found at line " << line_number
                      << " first encountered at line "
                      << label_location->second.first << ".\n";
          }
          else if (label_location == labels.end())
          {
            labels.insert({label, {instruction_number, {}}});
          }
          else
          {
            label_location->second.first = instruction_number;
          }
        }
      }
      const auto instruction_location = instructions.find(instruction);
      if (instruction_location == instructions.end())
      {
        error = true;
        std::cout << "Instruction '" << instruction << "' at line " << line_number << " not recognised.\n";
      }
      else
      {
        switch (instruction_location->second)
        {
          case instruction_code::in:
          case instruction_code::out:
          case instruction_code::line:
          case instruction_code::halt:
            program.push_back({instruction_location->second, {}});
            break;
          case instruction_code::print:
            literal_texts.push_back(instruction_data);
            program.push_back({instruction_location->second, literal_texts.size() - 1});
            break;
          case instruction_code::load:
            if (const auto identifier = is_identifier(instruction_data); identifier || is_integer(instruction_data))
            {
              assemble(instruction_data, instruction_code::load_direct, instruction_code::load_indirect, identifier);
            }
            else
            {
              error = true;
              std::cout << "The named store or integer '" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
            break;
          case instruction_code::store:
          {
            if (is_identifier(instruction_data))
            {
              const auto store_index = get_store_index(instruction_data);
              program.push_back({instruction_location->second, {store_index}});
            }
            else
            {
              error = true;
              std::cout << "The named store '" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
          }
            break;
          case instruction_code::add:
            if (!check_and_assemble(instruction_data, instruction_code::add_direct, instruction_code::add_indirect))
            {
              error = true;
              std::cout << "The named store or integer '" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
            break;
          case instruction_code::subtract:
            if (!check_and_assemble(instruction_data, instruction_code::subtract_direct,
                                    instruction_code::subtract_indirect))
            {
              error = true;
              std::cout << "The named store or integer '" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
            break;
          case instruction_code::multiply:
            if (!check_and_assemble(instruction_data, instruction_code::multiply_direct,
                                    instruction_code::multiply_indirect))
            {
              error = true;
              std::cout << "The named store or integer '" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
            break;
          case instruction_code::divide:
            if (!check_and_assemble(instruction_data, instruction_code::divide_direct,
                                    instruction_code::divide_indirect))
            {
              error = true;
              std::cout << "The named store or integer'" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
            break;
          case instruction_code::jump:
          case instruction_code::jineg:
          case instruction_code::jizero:
          {
            if (is_identifier(instruction_data))
            {
              if (const auto label_pointer = labels.find(instruction_data); label_pointer == labels.end())
              {
                labels.insert({instruction_data, {-1, {instruction_number}}});
              }
              else
              {
                label_pointer->second.second.push_back(instruction_number);
              }
              program.push_back({instruction_location->second, {}});
            }
            else
            {
              std::cout << "The label '" << instruction_data << "' at line " << line_number
                        << " is not of the correct format.\n";
            }
          }
            break;
        }
      }
    }
  }
  // Link labels to program.
  for (const auto &[name, locations]: labels)
  {
    if (locations.first == -1)
    {
      error = true;
      std::cout << "Could not resolve label " << name << ".\n";
    }
    else
    {
      for (const auto &usage: locations.second)
      {
        program[usage].second = locations.first;
      }
    }
  }

  // Check that the last instruction is a JUMP or a HALT.
  auto no_jump_or_halt = false;
  if (program.empty())
  {
    no_jump_or_halt = true;
  }
  else
  {
    const auto &last_instruction = program[program.size() - 1].first;
    if (last_instruction != instruction_code::jump && last_instruction != instruction_code::halt)
    {
      no_jump_or_halt = true;
    }
  }
  if (no_jump_or_halt)
  {
    error = true;
    std::cout << "The program must end with a JUMP or a HALT instruction.\n";
  }

  // The program must have at least one HALT instruction.
  if (std::find_if(program.begin(), program.end(), [](const auto & instruction){return instruction.first == instruction_code::halt;}) == program.end())
  {
    error = true;
    std::cout << "The program must contain at least one halt statement.\n";
  }

  return error;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void run()
{
  if (program.empty())
  {
    std::cout << "Nothing to run.\n";
    return;
  }
  accumulator = 0;
  size_t instruction_pointer = 0;
  while (true)
  {
    const auto &[instruction_type, instruction_data] = program[instruction_pointer];
    ++instruction_pointer;
    switch (instruction_type)
    {
      case instruction_code::in:
        std::cout << "Please enter an integer ";
        std::cin >> accumulator;
        break;
      case instruction_code::out:
        std::cout << accumulator;
        break;
      case instruction_code::print:
        std::cout << literal_texts[instruction_data];
        break;
      case instruction_code::line:
        std::cout << '\n';
        break;
      case instruction_code::load_direct:
        accumulator = instruction_data;
        break;
      case instruction_code::load_indirect:
        accumulator = named_storage[instruction_data].second;
        break;
      case instruction_code::store:
        named_storage[instruction_data].second = accumulator;
        break;
      case instruction_code::add_direct:
        accumulator += instruction_data;
        break;
      case instruction_code::add_indirect:
        accumulator += named_storage[instruction_data].second;
        break;
      case instruction_code::subtract_direct:
        accumulator -= instruction_data;
        break;
      case instruction_code::subtract_indirect:
        accumulator -= named_storage[instruction_data].second;
        break;
      case instruction_code::multiply_direct:
        accumulator *= instruction_data;
        break;
      case instruction_code::multiply_indirect:
        accumulator *= named_storage[instruction_data].second;
        break;
      case instruction_code::divide_direct:
      case instruction_code::divide_indirect:
        if (instruction_data == 0)
        {
          std::cout << "Run-time error - divide by zero\n";
        }
        else
        {
          accumulator /= instruction_type == instruction_code::divide_direct ? instruction_data
                                                                             : named_storage[instruction_data].second;
        }
        break;
      case instruction_code::jump:
        instruction_pointer = instruction_data;
        break;
      case instruction_code::jineg:
        if (accumulator < 0)
        {
          instruction_pointer = instruction_data;
        }
        break;
      case instruction_code::jizero:
        if (accumulator == 0)
        {
          instruction_pointer = instruction_data;
        }
        break;
      case instruction_code::halt:
        std::cout << "Program halted.\n";
        std::cin.ignore();
        return;
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
void display_storage()
{
  std::cout << "Accumulator:-\n";
  std::cout << accumulator << '\n';
  std::cout << "Named storage:-\n";
  for (const auto &[name, value]: named_storage)
  {
    std::cout << name << " = " << value << '\n';
  }
  std::cout << "Literal texts:-\n";
  for (const auto &text: literal_texts)
  {
    std::cout << text << '\n';
  }

}
//////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
  std::cout << "Welcome to the CESIL run-time environment.\n\n";
  std::cout << "Usage:\n";
  std::cout << "l(oad) <filename> - checks a source file for errors.\n";
  std::cout << "r(un) - executes the code.\n";
  std::cout << "m(emory) - displays named storage.\n";
  std::cout << "q(uit) - quits CESIL.\n\n";

  auto error = false;
  std::pair<char, std::filesystem::path> token{'b', ""};
  while (token.first != 'q')
  {
    std::string input;
    switch (token.first)
    {
      case 'l':
        error = analyse_file(token.second);
        break;
      case 'r':
        if (!error)
        {
          run();
        }
        else
        {
          std::cout << "Cannot run program due to syntax errors.\n";
        }
        break;
      case 'm':
        display_storage();
        break;
      case 'b':
        break;
      default:
        std::cout << "I do not understand.\n";
        break;
    }
    std::cout << "CESIL ";
    getline(std::cin, input);
    token = parse(input);
  }
  std::cout << "Bye\n";
}

