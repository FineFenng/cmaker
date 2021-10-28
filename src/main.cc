#include <unistd.h>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

#include "subprocess/ProcessBuilder.hpp"
#include "subprocess/basic_types.hpp"

std::vector<std::string> GetGitProjectPaths() {
  auto process = subprocess::run({ "find", ".", ".git", "-type", "d", "-prune"},
                                 subprocess::RunBuilder()
                                 .cerr(subprocess::PipeOption::pipe)
                                 .cout(subprocess::PipeOption::pipe)
                                 .cin(subprocess::PipeOption::pipe));

  std::string out = std::move(process.cout);
  std::vector<std::string> paths;
  const auto* begin = out.c_str();
  const auto* end = begin;
  while ((end = strchr(begin, '\n'))) {
    paths.emplace_back(begin, end);
    begin = ++end;
  }
  return paths;
}

std::string GetGitStatus(const std::string& wd) {
  auto process = subprocess::run({ "cd", wd, "&&", "git", "status", },
                                 subprocess::RunBuilder()
                                 .cerr(subprocess::PipeOption::pipe)
                                 .cout(subprocess::PipeOption::pipe)
                                 .cin(subprocess::PipeOption::close));
  std::string out = std::move(process.cout);
  return out;
}

int main(int argc, const char *argv[]) {
  using namespace ftxui;
  (void) argc;
  (void) argv;

  auto screen = ScreenInteractive::Fullscreen();

  ButtonOption button_option;
  button_option.border = false;
  auto quit_button = Button("[Q]uit", screen.ExitLoopClosure(), button_option);
  auto options = Container::Horizontal({ quit_button });

  auto header_render = Renderer(options, [&] {
    return hbox({
                    text("project-cmake"),
                    filler(),
                    quit_button->Render(),
           }) | bgcolor(Color::White) | color(Color::Black);
  });

  std::vector<std::string> project_paths = GetGitProjectPaths();


  auto project_names = [&project_paths]() -> std::vector<std::string> {
    std::vector<std::string> names;
    for(const auto& path: project_paths) {
      const auto* slash = strrchr(path.c_str(), '/');
      names.emplace_back(++slash, path.c_str() + path.size());
    }
    return names;
  }();

  int menu_idx = 0;
  MenuOption menu_option;
  std::string project_status;
  menu_option.on_change = [&] {
    auto project_path = project_paths[menu_idx];
    project_status = GetGitStatus(project_path);
  };
  Menu(&project_names, &menu_idx, menu_option);



  screen.Loop(header_render);

  return EXIT_SUCCESS;
}